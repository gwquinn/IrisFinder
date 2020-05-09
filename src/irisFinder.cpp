/**
* This software was developed at the National Institute of Standards and Technology (NIST) by
* employees of the Federal Government in the course of their official duties. Pursuant to title
* 17 Section 105 of the United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
* parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
* other characteristic.
*/
#include "irisFinder.h"
#include <opencv2/imgcodecs.hpp>
#include <stdint.h>                            // for uint8_t type

#ifdef NDEBUG

#include <numeric>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;

#endif

using std::vector;
using cv::Size2f;

constexpr IrisBoundary::Type Pupil  = IrisBoundary::Type::Pupil;
constexpr IrisBoundary::Type Limbus = IrisBoundary::Type::Limbus;

#ifdef NDEBUG
// DEBUG globals.
Mat image,               // the unaltered raw image
    mask,                // masks off specular highlights
    contrast,            // contrast image
    houghMask,           // used for creating hough map
    houghLines,          // skeletonized version of hough mask
    hough;               // used for determining approximate pupil center
#endif

static inline Mat getKernel(const int kSize, const int shape = cv::MORPH_ELLIPSE)
{
   return getStructuringElement(shape, cv::Size2d(kSize, kSize));
}

// Applies morphological erosion operation to image.
static inline void erode(const Mat1b& src, Mat1b& dst, const int kSize)
{
   cv::erode(src, dst, getKernel(kSize));
}

static inline bool inside(unsigned int x, unsigned int y, const Mat& m)
{
   return x >= 1 && y >= 1 && x <= m.cols - 2 && y <= m.rows - 2;
}

void IrisFinder::setImage(const Mat& image)
{
   // If color image, utilize only the red channel.
   if (image.channels() > 1)
      extractChannel(image, _image, 2);
   else
      _image = image;

   // Convert image to single-channel 8-bit depth.
   _image.convertTo(_image, CV_8UC1);

#ifdef NDEBUG
   ::image = _image.clone();
#endif

   // Identify extremely bright pixels in the image.
   threshold(_image, _mask, MinLedIntensity, 255, cv::THRESH_BINARY_INV);

   // Erode bright pixel mask to connect neighbours.
   erode(_mask, _mask, LedDilation);

   // Dilate bright pixel mask to shrink total LED area.
   cv::dilate(_mask, _mask, getKernel(LedDilation));

   // Break the mask into connected components.
   Mat labels,
       stats,
       centroids;

   connectedComponentsWithStats(_mask == 0, labels, stats, centroids);

   for (int r = 0; r < _mask.rows; ++r)
      for (int c = 0; c < _mask.cols; ++c)
      {
         // Which component does this pixel belong to?
         const int label = labels.at<int>(r, c);

         if (label != 0)
         {
            // To be an LED pixel, component area must lie within a certain range.
            const int area = stats.at<int>(label, cv::CC_STAT_AREA);

            if (area < MinLedArea || area > MaxLedArea)
               _mask.at<uint8_t>(r, c) = 255;
         }
      }

#ifdef NDEBUG
   ::mask = _mask.clone();
#endif

   // Apply horizontal open operation, to help reduce noise introduced by eyelashes.
   const cv::Size kSize(EyelashThickness, 1);
   const Mat kernel = getStructuringElement(cv::MORPH_RECT, kSize);

   morphologyEx(_image, _image, cv::MORPH_CLOSE, kernel);

   // Blur the image, to smooth out gradient directions.
   GaussianBlur(_image, _image, Size2f(), GradientSigma);

   // Contrast stretch the image.
   double min,
          max;

   cv::minMaxLoc(_image, &min, &max, NULL, NULL, _mask);

   _image = (_image - min) * 255. / (max - min); 

   // Compute gradient information.
   Sobel(_image, _gradX, CV_32F, 1, 0, 7, 1 / 1280.);
   Sobel(_image, _gradY, CV_32F, 0, 1, 7, 1 / 1280.);

   magnitude(_gradX, _gradY, _gradMag);

#ifdef NDEBUG
   ::contrast = _image.clone();
   bitwise_and(::contrast, _mask, ::contrast);
#endif
}

// Localize the pupil and iris boundaries.
void IrisFinder::boundaries(IrisBoundary& pupil, IrisBoundary& limbus) const
{
   // Localize the pupil.
   pupilBoundary(pupil);
   
   // Localize the limbus.
   limbusBoundary(limbus, pupil);

#ifdef NDEBUG
   cout << pupil << endl << limbus << endl;

   // Save image with iris boundaries overlaid.
   auto save = [](const Mat& image, const std::string& description,
                  const IrisBoundary& pupil, const IrisBoundary& limbus)
   {
      // Convert from grayscale to color.
      Mat out;
      cvtColor(image, out, cv::COLOR_GRAY2RGB);

      // Draw pupil boundary.
      if (pupil.x != -1)
      {
         ellipse(out, pupil.center(), pupil.size(), 0, 0, 360, cv::Scalar(0, 0, 255));
         circle(out, pupil.center(), 2, cv::Scalar(0, 0, 255), cv::FILLED);
      }

      // Draw limbus boundary.
      if (limbus.x != -1)
      {
         ellipse(out, limbus.center(), limbus.size(), 0, 0, 360, cv::Scalar(0, 255, 0));
         circle(out, limbus.center(), 2, cv::Scalar(0, 255, 0), cv::FILLED);
      }

      // Save image to file.
      imwrite(description + ".png", out);
   };

   save(image,      "raw",        pupil, limbus);
   save(mask,       "mask",       pupil, limbus);
   save(contrast,   "contrast",   pupil, limbus);
   save(houghMask,  "houghMask",  pupil, limbus);
   save(houghLines, "houghLines", pupil, limbus);
   save(hough,      "hough",      pupil, limbus);
#endif
}

// Localize the pupil.
void IrisFinder::pupilBoundary(IrisBoundary& pupil) const
{
   pupil.type = Pupil;

   // Binarize by thresholding on the pixel intensity.
   Mat1b pupilMask;
   threshold(_image, pupilMask, MaxPupilIntensity, 255, cv::THRESH_BINARY_INV);

   // Binarize by thresholding on the gradient magnitude.
   Mat gradMask;
   threshold(_gradMag, gradMask, MinBoundaryGradient, 255, cv::THRESH_BINARY);
   gradMask.convertTo(gradMask, CV_8U);

   // Combine gradient and intensity masks.
   Mat1b houghMask;
   bitwise_and(pupilMask, gradMask, houghMask);

   // Expand LED mask.
   Mat1b noLedNearBy;
   erode(_mask, noLedNearBy, LedNeighbourhood);

   bitwise_and(houghMask, noLedNearBy, houghMask);

   // Skeletonize the mask.
   cv::ximgproc::thinning(houghMask, houghMask, cv::ximgproc::THINNING_ZHANGSUEN);

   vector<vector<cv::Point>> contours;
   vector<cv::Vec4i> hierarchy;

   cv::findContours(houghMask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

   const int numRadii = MaxPupilRadius - MinPupilRadius;
   Mat accum = Mat::zeros(_image.size(), CV_16SC(numRadii));

   int maxScore = -1,
         radius = 0;

   // Foreach contour.
   for (const auto& contour : contours)
      if (contour.size() >= MinPupilContourLength)
         // Foreach point along the contour.
         for (const auto& p : contour)
         {
            float cx = p.x,
                  cy = p.y;

            const float mag = _gradMag.at<float>(p),
                         dx = _gradX.at<float>(p) / -mag,
                         dy = _gradY.at<float>(p) / -mag;

            // Iterate over all possible radii.
            for (int cr = 0; cr < MaxPupilRadius; ++cr, cx += dx, cy += dy)
            {
               // Stop if point falls outside the image.
               if (!inside(cx, cy, _image))
                  break;

               // Stop if hit another prospective pupil boundary.
               if (cr > 1 && houghMask.at<uint8_t>(cy, cx) > 0 && noLedNearBy.at<uint8_t>(cy, cx) > 0)
                  break;

               if (cr >= MinPupilRadius)
               {
                  const int ri = cr - MinPupilRadius;

                  // Loop over immediate neighbourhood.
                  for (int x = cx - 1; x <= cx + 1; ++x)
                     for (int y = cy - 1; y <= cy + 1; ++y)
                     {
                        short* radii = (short*)accum.ptr(y, x);

                        for (int r = ri - 1; r <= ri + 1; ++r)
                        {
                           // Could apply any neighbourhood weighting function here.
                           radii[r] += 4 - fabs(x - cx) + fabs(y - cy) + abs(r - ri);

                           // See if new maximum found.
                           if (radii[r] > maxScore)
                           {
                              maxScore = radii[r];

                              pupil.x = x;
                              pupil.y = y;
                              pupil.a = pupil.b = r + MinPupilRadius + 1;
                           }
                        }
                     }
               }
            } // end foreach radii
         } // end foreach contour point

#ifdef NDEBUG
   ::houghMask = 0.3 * pupilMask + 0.6 * gradMask;
   bitwise_and(::houghMask, noLedNearBy, ::houghMask);

   ::houghLines = houghMask;

   ::hough = Mat::zeros(_image.size(), CV_32F);

   for (int y = 0; y < _image.rows; ++y)
      for (int x = 0; x < _image.cols; ++x)
      {
         const short* ptr = (short*)accum.ptr(y, x);
         ::hough.at<float>(y, x) = std::accumulate(ptr, ptr + numRadii, 0);
      }

   normalize(::hough, ::hough, 0, 255, cv::NORM_MINMAX);
   ::hough.convertTo(::hough, CV_8U);
#endif

   // Fine tune the pupil fit.
   if (maxScore > -1)
      optimizeFit(pupil);
}

// Localize the limbus boundary.
void IrisFinder::limbusBoundary(IrisBoundary& limbus, const IrisBoundary& pupil) const
{
   // If pupil not found, limbus can't be found either.
   if (pupil.x == -1 || pupil.y == -1)
   {
      limbus = pupil;
      limbus.type = IrisBoundary::Limbus;

      return;
   }

   // Start with concentric ellipses.
   limbus.x = pupil.x;
   limbus.y = pupil.y;

   limbus.type = Limbus;

   // Start with the smallest possible limbus based on the pupil size.
   limbus.a = limbus.b = fmax(MinLimbusRadius, pupil.a + MinAnnulusThickness);
   
   // Stop if pupil radius is too big to work with.
   if (limbus.a > MaxLimbusRadius)
      return;
   
   float maxLeft = -1,
         maxRight = -1;

   int aLeft  = limbus.a,
       aRight = limbus.a;

   IrisBoundary limbusLeft(limbus);
   limbusLeft.type = IrisBoundary::Type::LeftLimbus;

   IrisBoundary limbusRight(limbus);
   limbusRight.type = IrisBoundary::Type::RightLimbus;

   // Iterate over all possible radii, smallest to largest.
   do
   {
      // Left limbus boundary.
      float contrast = boundaryStrength(limbusLeft);

      if (contrast > maxLeft)
      {
         maxLeft = contrast;
         aLeft   = limbusLeft.a;
      }

      // Expand limbus boundary size.
      limbusLeft.expand();

      // Right limbus boundary.
      contrast = boundaryStrength(limbusRight);
      if (contrast > maxRight)
      {
         maxRight = contrast;
         aRight   = limbusRight.a;
      }

      limbusRight.expand();
   }
   while (limbusLeft.a <= MaxLimbusRadius);

   limbus.x += (aRight - aLeft) / 2;
   limbus.a = limbus.b = 0.5 * (aLeft + aRight);

   optimizeFit(limbus);
}

// A variation of Daugman's integro-differential equation.
float IrisFinder::boundaryStrength(const IrisBoundary& boundary) const
{
   const float radius = fmin(boundary.a, boundary.b);

   if (boundary.type == IrisBoundary::Pupil && radius < MinPupilRadius)
      return 0;

   if (boundary.type != IrisBoundary::Pupil &&radius < MinLimbusRadius)
      return 0;

   // Get equidistant points along the boundary.
   vector<Point2f> points;
   boundary.points(points);

   float sum = 0,
         num = 0;

   // Foreach boundary point.
   for (const auto& p : points)
   {
      // If pixel is inside the image and not near and LED.
      if (inside(p.x, p.y, _image) && _mask.at<uint8_t>(p))
      {
         // Angle perpendicular to the tangent at the given boundary point.
         const Point2f theta((p.x - boundary.x) / boundary.a,
                             (p.y - boundary.y) / boundary.b);

         // Cosine of the difference between angles is their dot product.
         const float mag = _gradMag.at<float>(p);

         // Gradient direction at the given location.
         const Point2f grad(_gradX.at<float>(p), _gradY.at<float>(p));

         // Angle disparity.
         const float cosDiff = grad.dot(theta) / mag;

         // If gradient direction is moving away from the boundary.
         if (cosDiff >= AngleTolerance)
         {
            sum += mag;
            ++num;
         }
      }
   }

   const float ratio        = boundary.a < boundary.b ? boundary.a / boundary.b :
                                                        boundary.b / boundary.a,
               eccentricity = pow(ratio, 0.7),
               length       = pow(num, 3.0);

   return sum * eccentricity * length;
}

void IrisFinder::optimizeFit(IrisBoundary& boundary) const
{
   // Helper class that extends DownhillSolver::Function.
   struct Wrapper : public cv::DownhillSolver::Function
   {
      Wrapper(const IrisFinder* _f, const IrisBoundary::Type _t) : f(_f), t(_t) {};
      int getDims() const { return 4; };
      double calc(const double* x) const
         { return -f->boundaryStrength(IrisBoundary(t, x[0], x[1], x[2], x[3])); };

      const IrisFinder* f;
      const IrisBoundary::Type t;
   };

   const Mat initStep = (cv::Mat_<double>(1, 4) << 10, 10, 10, 10),
             params   = (cv::Mat_<double>(1, 4) << boundary.x, boundary.y,
                                                   boundary.a, boundary.b);

   const Wrapper wrapper(this, boundary.type);

   cv::Ptr<cv::DownhillSolver> solver =
      cv::DownhillSolver::create(cv::makePtr<Wrapper>(wrapper), initStep);

   solver->minimize(params);

   // Convert results back to boundary parameters.
   const double* x = (double*)params.ptr();

   boundary.x = x[0];
   boundary.y = x[1];
   boundary.a = x[2];
   boundary.b = x[3];
}


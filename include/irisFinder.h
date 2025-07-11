/**
* This software was developed at the National Institute of Standards and Technology (NIST) by
* employees of the Federal Government in the course of their official duties. Pursuant to title
* 17 Section 105 of the United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
* parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
* other characteristic.
*/
#ifndef IRIS_FINDER_H_
#define IRIS_FINDER_H_

#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>
#include "irisBoundary.h"

using cv::Mat;
using cv::Mat1b;

class IrisFinder
{
   public:
      IrisFinder() = default;
      IrisFinder(const Mat& image) { setImage(image); };

      void setImage(const Mat& image);

      // Localize the pupil and iris boundaries.
      void boundaries(IrisBoundary& pupil, IrisBoundary& limbus) const;

      // Localizes the pupil boundary.
      void pupilBoundary(IrisBoundary& pupil) const;
      
      // Localizes the iris boundary using the pupil boundary.
      void limbusBoundary(IrisBoundary& limbus, const IrisBoundary& pupil) const;
      
      // Measures the strength of the given iris boundary.
      float boundaryStrength(const IrisBoundary& boundary) const;

      int MinLedArea            =   10, // minimum area of an LED specular highlight
          MaxLedArea            = 3000, // maximum area of an LED specular highlight
          MinLedIntensity       =  230, // minimum pixel intensity to constitute an LED point
          LedDilation           =   10, // amount to dilate the LED mask (connects neighbours)
          LedErode              =    3, // amount to erode the LED mask
          MinLedNeighbourhood   =   20, // min distance from an LED to ignore as possible boundary point
          EyelashThickness      =    8, // Apply morphological dilation (mitigates eyelash impact)
          MinPupilRadius        =   11, // minimum pupil radius in pixels
          MaxPupilRadius        =  100, // maximum pupil radius in pixels
          MaxPupilIntensity     =   35, // maximum pixel intensity to constitute a pupil pixel
          MinPupilContourLength =   13, // minimum length of a pupil boundary contour
          MinAnnulusThickness   =   36, // minimum pixel thickness of the annulus
          MinLimbusRadius       =   86, // minimum pixel radius of the limbus
          MaxLimbusRadius       =  200; // maximum pixel radius of the limbus

      float GradientSigma       = 2.4,  // blur to apply prior to gradient computation
            MinBoundaryGradient = 4.1,  // minimum gradient to constitute a boundary
            AngleTolerance      = cos(M_PI / 10); // angle tolerance of gradient at boundary point

   protected:

      // Apply optimization algorithm to fine tune the boundary fit.
      void optimizeFit(IrisBoundary& boundary) const;

      Mat _image,                       // original (contrast enhanced) image
          _gradX,                       // gradient in the horizontal direction
          _gradY,                       // gradient in the vertical direction
          _gradMag;                     // gradient magnitude

      Mat1b _mask;                      // LED specular highlight neighbouring region
};

#endif // IRIS_FINDER_H_

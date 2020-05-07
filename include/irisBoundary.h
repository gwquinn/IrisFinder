/**
* This software was developed at the National Institute of Standards and Technology (NIST) by
* employees of the Federal Government in the course of their official duties. Pursuant to title
* 17 Section 105 of the United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
* parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
* other characteristic.
*/
#ifndef IRIS_BOUNDARY_H_
#define IRIS_BOUNDARY_H_

#include <opencv2/opencv.hpp>

using std::vector;
using cv::Point2f;
using cv::Point;

class IrisBoundary
{
   public:
      // Type of iris boundary.
      enum Type { Pupil, Limbus, LeftLimbus, RightLimbus };
   
      IrisBoundary(const Type t = Type::Pupil,
                   const float x = -1, const float y = -1,
                   const float a = -1, const float b = -1);

      cv::Size2d size() const { return cv::Size2f(a, b); };
      Point2f center()  const { return Point2f(x, y); };

      void points(vector<Point2f>& points) const;

      void expand(const int size = 1);
      bool inside(const Point& p) const;
      bool inside(const int x, const int y) const;
      bool valid() const;
      float eccentricity() const;

      Type type;

      float x,                            // x-coordinate
            y,                            // y-coordinate
            a,                            // major axis
            b;                            // minor axis

      friend std::ostream& operator << (std::ostream& os, const IrisBoundary& b);  // DEBUG

   protected:
      // Precomputed trigonomic values.
      static const vector<Point2f> PupilPoints,
                                   LeftLimbusPoints,
                                   RightLimbusPoints;
};

#endif

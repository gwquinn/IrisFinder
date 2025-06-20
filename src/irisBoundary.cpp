/**
* This software was developed at the National Institute of Standards and Technology (NIST) by
* employees of the Federal Government in the course of their official duties. Pursuant to title
* 17 Section 105 of the United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
* parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
* other characteristic.
*/
#include "irisBoundary.h"

// Factory function.
static const vector<Point2f> trig(const double start, const double end)
{
   vector<Point2f> v;

   for (double angle = start; angle < end; angle += M_PI / 90)
      v.push_back(Point2f(cos(angle), sin(angle)));

   return v;
}

// Precompute triconomic values.
const vector<Point2f> IrisBoundary::PupilPoints       = trig(0, 2 * M_PI),
                      IrisBoundary::LeftLimbusPoints  = trig( 0.8 * M_PI, 1.3 * M_PI),
                      IrisBoundary::RightLimbusPoints = trig(-0.2 * M_PI, 0.3 * M_PI);

IrisBoundary::IrisBoundary(const Type ti, const float xi, const float yi,
                                          const float ai, const float bi) :
              type(ti), x(xi), y(yi), a(ai), b(bi)
{};

void IrisBoundary::points(vector<Point2f>& points) const
{
   switch (type)
   {
      case Type::Pupil:       points = PupilPoints;       break;
      case Type::LeftLimbus:  points = LeftLimbusPoints;  break;
      case Type::RightLimbus: points = RightLimbusPoints; break;
      case Type::Limbus:
         points = LeftLimbusPoints;
         points.insert(points.end(), RightLimbusPoints.begin(), RightLimbusPoints.end());
         break;
   }

   // Shift and scale boundary points.
   for (auto& p : points)
      p = Point(x + p.x * a + 0.5, y + p.y * b + 0.5);
};

void IrisBoundary::expand(const int size)
{
   a += size;
   b += size;
}

bool IrisBoundary::inside(const Point& p) const
{
   return pow((x - p.x) / a, 2) + pow((y - p.y) / b, 2) <= 1;
}

bool IrisBoundary::inside(const int x, const int y) const
{
   return inside(Point(x, y));
}

bool IrisBoundary::valid() const
{
   return !(x < 0 || y < 0 || a < 0 || b < 0);
}

float IrisBoundary::eccentricity() const // -GW function not used?
{
   const float ratio = a < b ? a / b : b / a;
   return sqrt(1 - pow(ratio, 2));
}

std::ostream& operator << (std::ostream& os, const IrisBoundary& b)
{
   const std::string type = (b.type == IrisBoundary::Pupil) ? "Pupil:" : "Limbus:";

   os << type << " [" << round(b.x) << " " << round(b.y) << "] ["
      << round(b.a) << " " << round(b.b) << "]";

   return os;
}


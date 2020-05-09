/**
* This software was developed at the National Institute of Standards and Technology (NIST) by
* employees of the Federal Government in the course of their official duties. Pursuant to title
* 17 Section 105 of the United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
* parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
* other characteristic.
*/
#include "irisFinder.h"
#include <opencv2/ximgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <sstream>

using namespace std;

int main(int argc, char* argv[])
{
   const string keys = "{@i   | <none> | path to image     }"
                       "{help |        | show this message }";

   // Parse arguments.
   cv::CommandLineParser parser(argc, argv, keys);

   // Print help message.
   if (parser.has("help")) {
      parser.printMessage();
      return EXIT_SUCCESS;
   }

   // Check command line parameters.
   if (!parser.check()) {
      parser.printErrors();
      return EXIT_FAILURE;
   }

   const string imgPath = parser.get<string>("@i");

   const cv::Mat img = cv::imread(imgPath);

   const IrisFinder irisFinder(img);

   IrisBoundary pupil,
                limbus;

   irisFinder.boundaries(pupil, limbus);

   cerr << pupil << " " << limbus << endl;

   // Draw pupil boundary.
   if (pupil.x != -1)
      ellipse(img, pupil.center(), pupil.size(), 0, 0, 360, cv::Scalar(0, 0, 255));

   // Draw limbus boundary.
   if (limbus.x != -1)
      ellipse(img, limbus.center(), limbus.size(), 0, 0, 360, cv::Scalar(0, 255, 0));

   // Save image to file.
   const string outPath = imgPath.substr(0, imgPath.find_last_of(".")) + "_out.png";

   cv::imwrite(outPath, img);

   return EXIT_SUCCESS;
}


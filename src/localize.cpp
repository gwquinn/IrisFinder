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
#include <opencv2/core.hpp>
#include <be_io_recordstore.h>
#include <be_memory_autoarrayutility.h>
#include <sstream>

namespace BE = BiometricEvaluation;
using namespace std;
using namespace BE::Memory::AutoArrayUtility;
using BE::IO::RecordStore;

int main(int argc, char* argv[])
{
   const string keys = "{@r   | <none> | path to record store }"
                       "{@k   | <none> | key to image         }"
                       "{help |        | show this message    }";

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

   // Retrieve command line parameters.
   const string recStorePath = parser.get<string>("@r"),
                         key = parser.get<string>("@k");

   std::shared_ptr<RecordStore> recordStore;

   // Open the record store.
   try {
      recordStore = RecordStore::openRecordStore(recStorePath);
   }
   catch (const BE::Error::StrategyError& e) {
      cerr << "Error: " << e.what() << endl;
      return EXIT_FAILURE;
   }

   // Load the image as raw data.
   BE::Memory::uint8Array data;

   try {
      data = recordStore->read(key);
   }
   catch (const BE::Error::Exception& e) {
      cerr << "Error: " << e.what() << endl;
      return EXIT_FAILURE;
   }

   // Convert raw image data into type Mat.
   const cv::Mat mat = cv::imdecode(cv::Mat(1, data.size(), CV_8U, cstr(data)),
                                    cv::IMREAD_UNCHANGED);

   const IrisFinder irisFinder(mat);

   IrisBoundary pupil,
                limbus;

   irisFinder.boundaries(pupil, limbus);

   return EXIT_SUCCESS;
}


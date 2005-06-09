#include <iostream>
#include <fstream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <classad_distribution.h>

/*#include "glite/wms/jdl/JobAdManipulation.h"
##include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"

namespace jdl = glite::wms::jdl;
*/
int
main(int argc, char* argv[])
{
  std::ifstream inf;
  std::string   outputfile;

  // we read the ClassAds file.
  if (argc == 3) {
    inf.open(argv[1]);
    outputfile = argv[2];
  } else {
    std::cout << "Usage: "
         << argv[0]
         << " CLASSAD_CE_INFO_FILE"
	 << " CLASSAD_OUTPUT_FILE"
         << std::endl;
    return 1;
  }

  std::string input_ad;
  while (!inf.eof()) {
    std::string line;
    inf >> line;
    input_ad += line;
  }

  inf.close(); 

  // we define a ClassAds.
  classad::ClassAdParser parser;
  classad::ClassAd* ad = parser.ParseClassAd(input_ad.c_str());

  std::cout << "start parser: " << input_ad << std::endl;

  if (ad == 0)
  {
    std::cout << "Bad input classad file" << std::endl;
    return 1;
  }

  std::ofstream outf(outputfile.c_str());

  for (int i = 0; i<=3; i++) {
    classad::ClassAd ad1;

    ad1.InsertAttr("id", "ranatan.cnaf.infn.it:2190");

    ad1.InsertAttr("update_time", i);

    ad1.InsertAttr("expiry_time", 120+i);

    ad1.Insert("info", ad->Copy());

    outf << ad1;
  }

  delete ad;
}

  



/***************************************************************************
 *  filename  : jobwrapperTest.cpp
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <string>

#include "classad_distribution.h"

#include "glite/wms/helper/exceptions.h"

#include "jobadapter/JobAdapter.h"
#include "jobadapter/exceptions.h"

#include "jobadapter/url.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/jdl/convert.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/logger/edglog.h"

using namespace std;
using namespace classad;

using namespace glite::wms::helper::jobadapter;

using namespace glite::wmsutils::jobid;

using namespace glite::wms::common::configuration;
using namespace glite::wms::common::logger;
using namespace glite::wms::common::logger::threadsafe;

namespace jdl = glite::jdl;
namespace helper = glite::wms::helper;

int
main(int argc, char* argv[])
{
  ifstream inf;	
  string   jaconf;
  string   outputfile;
  string   parsedofile;
  
  // we read the ClassAds file.
  if (argc == 5) {
    inf.open(argv[1]);
    outputfile = argv[2];
    parsedofile = argv[3];
    jaconf = argv[4];
  } else {
    cout << "Usage: " 
	 << argv[0] 
	 << " CLASSAD_INPUTFILE" 
	 << " CLASSAD_OUTPUTFILE"  
	 << " PARSED_OUTPUTFILE" 
	 << " JOBADAPTER_CONFFILE" 
	 << endl; 
    return 1;
  }

  string input_ad;
  while (!inf.eof()) {
    string line;
    inf >> line;
    input_ad += line;
  }

  inf.close();
  
  // we define a ClassAds.
  ClassAdParser parser;
  const ClassAd*      ad = parser.ParseClassAd(input_ad.c_str());
  
  if (ad == 0) 
  {
    cout << "Bad input classad file" << endl;
    return 1;
  }
  
  try {
    // print on the standard output the log file	  
    edglog.open(cout);

    // we define the submit path	  
    Configuration confi(jaconf, "WorkloadManager");

    const JCConfiguration *jwconfig = Configuration::instance()->jc();

    string jw_file_path(jwconfig->submit_file_dir());

    // we define a JobAdapter
    JobAdapter ja(ad); 

    const ClassAd*      ad_modified = ja.resolve();

    if (ad_modified == 0) 
    {
      cout << "Bad input classad file!" << endl;	    
      return 1;
    }
	    
    ofstream outf(outputfile.c_str());
    
    // we print out the ClassAds Modified
    PrettyPrint printer;
    string      printed_outputclassad;
    
    printer.Unparse(printed_outputclassad, ad_modified);
  
    outf << printed_outputclassad;
    
    ofstream convertedoutf(parsedofile.c_str());

    jdl::to_submit_stream(convertedoutf, *ad_modified); 
  }
  catch (CannotConfigure &ex) {
    cerr << ex << endl;
    return 1;
  }
  catch (helper::CannotSetAttribute& ex) {
    cerr << "Cannot Set Attribute " << ex.what() << endl;
    return 2;
  }
  catch (helper::CannotGetAttribute& ex) {
    cerr << "Cannot Get Attribute " <<  ex.what() << endl;
    return 3;
  }
  catch (CannotCreateJobWrapper& ex) {
    cerr << "Cannot Create Job Wrapper " <<  ex.what() << endl;
    return 4;
  }
  catch (helper::InvalidAttributeValue& ex) {
    cerr << "Invalid Attribute Value " <<  ex.what() << endl;
    return 5;
  }
  catch (InvalidURL& ex) {
    cerr << ex.what() << endl;
    return 6;
  }
  catch (std::exception& ex) {
    cerr << "Other " << ex.what() << endl;
    return 7;
  }
  catch (...) {
    cerr << "Other errors" << endl;
    return 8;
  }
  
  return 0;
}



/***************************************************************************
 *  filename  : jobwrapperTest.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <iostream>
#include <fstream>

#include "jobadapter/JobWrapper.h"
#include "glite/wms/common/configuration/Configuration.h"

using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

using namespace glite::wms::helper::jobadapter::url;
using namespace glite::wms::common::configuration;
using namespace glite::wms::helper::jobadapter;

int
main(int argc, char* argv[])
{
  // define input sandbox
  vector<string> in_files;
  in_files.push_back("job.sh");
  in_files.push_back("inputfile1");
  in_files.push_back("inputfile2");

  // define output sandbox
  vector<string> output_files;
  output_files.push_back("outputfile1");

  Configuration config("glite_wms.conf", "WorkloadManager");
  JobWrapper *jw;
  
  try {
    // initialize a job wrapper with job.sh as executable	  
    jw = new JobWrapper("job.sh");

    // set the input sandbox
    jw->input_sandbox(URL("gsiftp://joda.cnaf.infn.it:9000/home/joda/wp1/JobWrapper"), in_files);

    // set the output sandbox
    jw->output_sandbox(URL("gsiftp://joda.cnaf.infn.it:8000/tmp/gridtest"), output_files);
  } catch (ExInvalidURL& ex) {
    cerr << "Invalid URL" << ex.parameter() << endl;
    return -1;
  } catch (...) {
    cerr << "Caught uknown exception" << endl;
    return -2;
  }

  // set the stdin, stdout and stderr for job.sh
  jw->standard_input("job.in");
  jw->standard_output("job.out");
  jw->standard_error("job.err");
  
  // set arguments
  jw->arguments("-i \"\\\"Fabrizio Pacini\" *.txt");

  // set EDG_WL_RB_BROKERINFO to the default value
  jw->brokerinfo(".BrokerInfo");

  // set EDG_WL_JOBID to the default value
  jw->job_Id("https://edt003.cnaf.infn.it:9000/131.154.99.82/092250216745692?edt003.cnaf.infn.it:7771");

  // set EDG_WL_SEQUENCE_CODE to the default value
  //jw->sequence_code("UI=1:NS=1:WM=3:BH=3:JSS=2:LM=1:LRMS=1:APP=0");
 
  // run job.sh in a subdirectory
  jw->create_subdir();
  
  // output the job wrapper script to standard output
  cout << *jw;

  // output the job wrapper script to file
  ofstream of("./job_wrapper.sh");
  of << *jw;

  delete jw;

  return 0;
}



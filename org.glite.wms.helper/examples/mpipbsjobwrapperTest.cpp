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

#include "jobwrapper/JobWrapper.h"
#include "jobwrapper/MpiPbsJobWrapper.h"

using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

using namespace glite::wms::jobadapter::url;
using namespace glite::wms::jobadapter::jobwrapper;

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

  MpiPbsJobWrapper *mpjw;
  
  try {
    // initialize an mpi pbs job wrapper with job.sh as executable
    // and 2 as node number
    mpjw = new MpiPbsJobWrapper("job.sh");

    // set the input sandbox
    mpjw->input_sandbox(URL("gsiftp://joda.cnaf.infn.it:9000/home/joda/wp1/JobWrapper"), in_files);

    // set the output sandbox
    mpjw->output_sandbox(URL("gsiftp://joda.cnaf.infn.it:8000/tmp/gridtest"), output_files);
  } catch (ExInvalidURL& ex) {
    cerr << "Invalid URL" << ex.parameter() << endl;
    return -1;
  } catch (...) {
    cerr << "Caught uknown exception" << endl;
    return -2;
  }

  // set the stdin, stdout and stderr for job.sh
  mpjw->standard_input("job.in");
  mpjw->standard_output("job.out");
  mpjw->standard_error("job.err");

  // set arguments
  mpjw->arguments("-i \"\\\"Fabrizio Pacini\" *.txt");

  // set EDG_WL_RB_BROKERINFO to the default value
  mpjw->brokerinfo(".BrokerInfo");

  // set EDG_WL_JOBID to the default value
  mpjw->job_Id("https://edt003.cnaf.infn.it:9000/131.154.99.82/092250216745692?edt003.cnaf.infn.it:7771");

  // run job.sh in a subdirectory
  mpjw->create_subdir();

  // output the job wrapper script to file
  ofstream of("./mpi_pbs_job_wrapper.sh");
  of << *mpjw;

  delete mpjw;
  
  return 0;
}



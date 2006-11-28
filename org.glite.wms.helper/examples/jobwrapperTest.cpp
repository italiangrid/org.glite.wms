/***************************************************************************
 *  filename  : jobwrapperTest.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <netdb.h>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wms/common/configuration/Configuration.h"

#include "jobadapter/JobWrapper.h"

namespace fs = boost::filesystem;
namespace jobid = glite::wmsutils::jobid;

using std::ofstream;
using std::cout;
using std::cerr;
using std::vector;
using std::string;
using namespace glite::wms::common::configuration;
using namespace glite::wms::helper::jobadapter;

int
main(int argc, char* argv[])
{
  Configuration config("glite_wms.conf", "WorkloadManager");

  // Figure out the local host name (this should really come from
  // some common entity).
  char   hostname[1024];
  std::string local_hostname;

  if (gethostname(hostname, sizeof(hostname)) >= 0) {
    hostent resolved_host;
    hostent *resolver_result=NULL;
    int resolver_work_buffer_size = 2048;
    char* resolver_work_buffer = new char[resolver_work_buffer_size];
    int resolve_errno = ERANGE;
    while ((gethostbyname_r(hostname, &resolved_host,
                            resolver_work_buffer,
                            resolver_work_buffer_size,
                            &resolver_result,
                            &resolve_errno) ) == ERANGE ) {
      delete[] resolver_work_buffer;
      resolver_work_buffer_size *= 2;
      resolver_work_buffer = new char[resolver_work_buffer_size];
    }
    if (resolver_result != NULL) {
      local_hostname.assign(resolved_host.h_name);
    }
    delete[] resolver_work_buffer;
  }

  // define input sandbox
  vector<string> in_files;
  in_files.push_back("job.sh");
  in_files.push_back("inputfile1");
  in_files.push_back("inputfile2");

  // define output sandbox
  vector<string> output_files;
  output_files.push_back("outputfile1");

  boost::scoped_ptr<JobWrapper> jw;
  
  try {

    jw.reset(new JobWrapper("job.sh"));

    // set the input sandbox
    jw->input_sandbox(URL("gsiftp://joda.cnaf.infn.it:9000/home/joda/wp1/JobWrapper"), in_files);

    // set the output sandbox
    jw->output_sandbox(URL("gsiftp://joda.cnaf.infn.it:8000/tmp/gridtest"), output_files);
  } catch (InvalidURL& ex) {
    cerr << ex.what();
    return -1;
  } catch (...) {
    cerr << "Caught uknown exception\n";
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
  jobid::JobId j("https://edt003.cnaf.infn.it:9000/131.154.99.82/092250216745692?edt003.cnaf.infn.it:7771");
  jw->job_id(j.toString());

  // set EDG_WL_SEQUENCE_CODE to the default value
  //jw->sequence_code("UI=1:NS=1:WM=3:BH=3:JSS=2:LM=1:LRMS=1:APP=0");
 
  // run job.sh in a subdirectory
  jw->create_subdir();
  
  std::string maradonapr("gsiftp");
  std::string jobid_to_file(jobid::to_filename(j));
  try {
    fs::path maradona_path("SandBox", fs::native);
    maradona_path /= fs::path(jobid::get_reduced_part(j), fs::native);
    maradona_path /= fs::path(jobid_to_file, fs::native);
    maradona_path /= fs::path("Maradona.output", fs::native);

    jw->maradona(maradonapr + "://" + local_hostname, maradona_path.native_file_string());
  } catch(fs::filesystem_error const& ex) {
    cerr << ex.what();
    return -1;
  }

  try {
    URL url_("http://results_collector.cnaf.infn.it/gravitational_waves/mcecchi");
    jw->set_output_sandbox_base_dest_uri(url_);
    jw->set_osb_wildcards_support(true);
  } catch (InvalidURL& ex) {
    cerr << ex.what();
    return -1;
  }

  // output the job wrapper script to standard output
  cout << *jw;

  // output the job wrapper script to file
  ofstream of("./job_wrapper.sh");
  of << *jw;

  return 0;
}

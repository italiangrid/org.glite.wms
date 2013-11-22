/* LICENSE: 
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#include <list>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <getopt.h>
#include "iceDb/Transaction.h"
#include "iceDb/GetRegisteredStats.h"
#include "iceUtils/IceConfManager.h"
#include "iceUtils/CreamJob.h"

#include <boost/algorithm/string.hpp>

/* workaround for gsoap 2.7.13 */
#include "glite/ce/cream-client-api-c/cream_client_soapH.h"
SOAP_NMAC struct Namespace namespaces[] = {};

using namespace std;
using namespace glite::wms::ice;

int main( int argc, char* argv[] ) {
  util::IceConfManager::init( "glite_wms.conf" );
  try{
    util::IceConfManager::instance();
  }
  catch(util::ConfigurationManager_ex& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
  
  if(argc<3) return 1;
  vector<boost::tuple<time_t, string, string, string> > target;
  db::GetRegisteredStats stats( target, (time_t)atoi(argv[1]), (time_t)atoi(argv[2]), "queryRegisteredJobs" );
  db::Transaction tnx(true,false);
  tnx.execute( &stats );
  
  vector<boost::tuple<time_t, string, string, string> >::const_iterator it=target.begin();
  for( ; it!= target.end(); ++it) {
    cout << "GridJobID=" << it->get<2>() << " - Timestamp=" << it->get<0>() << " - CreamJobID=" << it->get<1>() << "/" << it->get<3>() << endl;
  }
}

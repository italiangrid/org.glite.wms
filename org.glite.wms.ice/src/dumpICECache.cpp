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
#include "ice-core.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "iceConfManager.h"
#include <string>
#include <map>
#include <iostream>
#include <unistd.h>
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

using namespace std;
namespace creamApi = glite::ce::cream_client_api;
namespace iceUtil = glite::wms::ice::util;

/*
 * Dumps the content of ICE jobCache in human-readable form
 */
int main(int argc, char*argv[]) {
  
  if ( argc!=2 ) {
      cout << "Usage: " << argv[0]
           << " <conf file name >" << endl;
      return 1;
  } 



  /*****************************************************************************
   * Initializes configuration manager (that in turn loads configurations)
   ****************************************************************************/
  iceUtil::iceConfManager::init(argv[1]);
  try{
    iceUtil::iceConfManager::getInstance();
  }
  catch(iceUtil::ConfigurationManager_ex& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
 
  /*****************************************************************************
   * Initializes job cache
   ****************************************************************************/ 
  /*string jcachefile = iceUtil::iceConfManager::getInstance()->getCachePersistFile();
  string jsnapfile  = iceUtil::iceConfManager::getInstance()->getCachePersistFile()+".snapshot";
  iceUtil::jobCache::setJournalFile(jcachefile);
  iceUtil::jobCache::setSnapshotFile(jsnapfile);*/
  string jcachedir = iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->persist_dir();
  iceUtil::jobCache::setPersistDirectory( jcachedir );
  iceUtil::jobCache::setRecoverableDb( false );
  // iceUtil::jobCache::setAutoPurgeLog( false );
  iceUtil::jobCache::setReadOnly( true );
  
  iceUtil::jobCache* _jCache = iceUtil::jobCache::getInstance();

  /*
   * Dumps the content of the job cache
   */   
  cout << endl
       << "Cream Job ID / Grid Job ID / Status" 
       << endl 
       << endl;
  iceUtil::jobCache::iterator it( _jCache->begin() );
  int count = 0;
  map<string, int> statusMap;
  for ( it=_jCache->begin(); it != _jCache->end(); ++it ) {
    iceUtil::CreamJob aJob( *it );
    cout << aJob.getCreamJobID() << "   "
	 << aJob.getGridJobID() << "   "
	 << creamApi::job_statuses::job_status_str[ aJob.getStatus() ] 
	 << endl;
    count++;
    statusMap[string(creamApi::job_statuses::job_status_str[ aJob.getStatus()] )]++;
  }
  
  cout << endl<< "Total number of job(s)=" << count << endl;
  for(map<string, int>::const_iterator it=statusMap.begin(); it!=statusMap.end(); ++it) {
    cout << "Status ["<< it->first << "] has "<< it->second<<" job(s)"<<endl;
  }
  return 0;
}

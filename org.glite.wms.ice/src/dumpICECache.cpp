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
  // iceUtil::jobCache::setReadOnly( true );
  
  iceUtil::jobCache* _jCache = iceUtil::jobCache::getInstance();

  /*
   * Dumps the content of the job cache
   */   
  cout << endl
       << "Cream Job ID / Grid Job ID / Status" 
       << endl 
       << endl;
  iceUtil::jobCache::iterator it;
  size_t totsize = 0;
  int count = 0;
  map<string, int> statusMap;
  for ( it=_jCache->begin(); it != _jCache->end(); it++ ) {
    iceUtil::CreamJob aJob( *it );
    cout << aJob.getCreamJobID() << "   "
	 << aJob.getGridJobID() << "   "
	 << creamApi::job_statuses::job_status_str[ aJob.getStatus() ] 
	 << " ~" << aJob.size() << " Bytes"
	 << endl;
    totsize += aJob.size();
    count++;
    statusMap[string(creamApi::job_statuses::job_status_str[ aJob.getStatus()] )]++;
  }
  
  cout << endl<< "Total number of job(s)=" << count << " - total bytes used in memory for jobCache: "<<totsize<<" bytes"<<endl;
  for(map<string, int>::const_iterator it=statusMap.begin(); it!=statusMap.end(); ++it) {
    cout << "Status ["<< it->first << "] has "<< it->second<<" job(s)"<<endl;
  }
  return 0;
}

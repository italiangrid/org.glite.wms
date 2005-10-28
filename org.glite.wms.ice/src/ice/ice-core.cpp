
#include "ice-core.h"
#include "jobCache.h"
#include "jobRequest.h"
#include "thread.h"

#include <exception>
#include <iostream>
#include <unistd.h>

using namespace glite::wms::ice;
using namespace glite::wms::common::utilities;
using namespace std;


//______________________________________________________________________________
ice::ice(const string& NS_FL, 
	 const string& WM_FL,
	 const string& jobcache_persist_file,
	 const int& listenPort,
	 const bool& start_listener,
	 const bool& start_poller,
	 const int&  poller_delay,
	 const std::string& CreamUrl,
	 const string& hostCert) throw(iceInit_ex&)
  : status_listener_started(false), 
    ns_filelist(NS_FL), 
    wm_filelist(WM_FL),
    fle(WM_FL.c_str())
{

  cout << "Initializing jobCache with journal file ["
       << jobcache_persist_file << "] and snapshot file ["
       << jobcache_persist_file+".snapshot" << "]..."<<endl;

//   try {
//     job_cache = 
//       new util::jobCache(jobcache_persist_file+".snapshot", 
// 			 jobcache_persist_file);
//   } catch(exception& ex) { throw iceInit_ex(ex.what()); }

  // the following line just create a eventStatusListener object and 
  // a CEConsumer that initialize the SOAP runtime

  glite::wms::ice::util::jobCache::setJournalFile(jobcache_persist_file);
  glite::wms::ice::util::jobCache::setSnapshotFile(jobcache_persist_file+".snapshot");
  
  if(start_listener) {
    cout << "Creating a CEMon listener object..."<<endl;
    listener = new util::eventStatusListener(listenPort);
    while(!listener->bind()) {
      cout << "error message=" << listener->getErrorMessage() << endl;
      cout << "error code   =" << listener->getErrorCode() << endl;
      cout << "Retrying in 5 seconds..." <<endl;
      sleep(5);
    }

//     try {
//       listener->setJobCache( jobCache::getInstance() );
//     } catch(std::exception& ex) {
//       cerr << ex.what() << endl;
//       exit(1);
//     }
    
    cout << "Creating thread object for CEMon listener..."<<endl;
    
    listenerThread = new util::thread(*listener);
    
    cout << "Starting CEMon listener thread..."<<endl;
    
    try {this->startJobStatusListener();}
    catch(util::thread_start_ex& ex) {
      throw iceInit_ex(ex.what()); 
    }  
    cout << "listener started succesfully!"<<endl;
  }    

  if(start_poller) {
    cout << "Creating a Cream status poller object..."<<endl;
    try {
      poller = new util::eventStatusPoller(hostCert, CreamUrl, poller_delay);
    } catch(glite::wms::ice::util::eventStatusPoller_ex& ex) {
      throw iceInit_ex(ex.what());
    }

//     try {
//       poller->setJobCache( jobCache::getInstance() );
//     } catch(std::exception& ex) {
//       cerr << ex.what() << endl;
//       exit(1);
//     }
    
    cout << "Creating thread object for Cream status poller..."<<endl;
    
    pollerThread = new util::thread(*poller);
    
    cout << "Starting Cream status poller thread..."<<endl;
    
    try {
      this->startJobStatusPoller();
    } catch(util::thread_start_ex& ex) {
      throw iceInit_ex(ex.what()); 
    }  
    cout << "poller started succesfully!"<<endl;
  }
  //sleep(1000);
  cout << "Initializing File Extractor object..."<<endl;

  try{
    flns.open(NS_FL.c_str());
  }
  catch(std::exception& ex) {
    throw iceInit_ex(ex.what());
  } catch(...) {
    cerr << "ice::ice - unknown exception caugth"<<endl;
    exit(1);
  }
}

//______________________________________________________________________________
ice::~ice() 
{ 
  if(listenerThread) {
    listenerThread->join();
    delete(listenerThread);
  }
  if(listener) delete(listener);
  //if(job_cache) delete(job_cache);
}

//______________________________________________________________________________
void ice::startJobStatusListener() throw(util::thread_start_ex&)
{
  listenerThread->start();
}

//______________________________________________________________________________
void ice::startJobStatusPoller() throw(util::thread_start_ex&)
{
  pollerThread->start();
}

//______________________________________________________________________________
void ice::stopJobStatusListener() {
  listenerThread->stop();
}

//______________________________________________________________________________
void ice::stopJobStatusPoller() {
  pollerThread->stop();
}

//______________________________________________________________________________
void ice::clearRequests() 
{
  requests.clear();
}

//______________________________________________________________________________
void ice::getNextRequests(vector<string>& ops) 
{
  requests = fle.get_all_available();
  for(unsigned int j=0; j<requests.size(); j++)  
    ops.push_back(*requests[j]);
}

//______________________________________________________________________________
void ice::removeRequest(const unsigned int& reqNum) 
{
  fle.erase(requests[reqNum]);
}

//______________________________________________________________________________
void ice::ungetRequest(const unsigned int& reqNum)
{
  FileListMutex mx(flns);
  FileListLock  lock(mx);

  string toResubmit = *requests[reqNum];
  unsigned int pos = toResubmit.find("jobsubmit");
  if(pos == string::npos) return;
  string tmp = toResubmit.substr(0,pos);
  tmp += "jobresubmit";
  tmp += toResubmit.substr(pos+9, string::npos);

  try {
    cout << "ice::ungetRequest: Putting ["<<tmp<<"] to NS filelist"<<endl;
    flns.push_back(tmp);
  } catch(std::exception& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
}

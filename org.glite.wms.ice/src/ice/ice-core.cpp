
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
	 const string& jobcache_persist_file
	 //	 const int& _listenPort,
	 //	 const bool& _start_listener,
	 //	 const bool& _start_poller,
	 //	 const int&  _poller_delay,
	 //	 const string& _hostCert
	 ) throw(iceInit_ex&)
  : status_listener_started(false), 
    status_poller_started(false),
    ns_filelist(NS_FL), 
    wm_filelist(WM_FL),
    fle(WM_FL.c_str())
    //     start_listener(_start_listener),
    //     start_poller(_start_poller),
    //    hostCert(_hostCert),
    //    listenPort(_listenPort),
    //    poller_delay(_poller_delay)
{
  cout << "Initializing jobCache with journal file ["
       << jobcache_persist_file << "] and snapshot file ["
       << jobcache_persist_file+".snapshot" << "]..."<<endl;

  glite::wms::ice::util::jobCache::setJournalFile(jobcache_persist_file);
  glite::wms::ice::util::jobCache::setSnapshotFile(jobcache_persist_file+".snapshot");
  
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
}

//______________________________________________________________________________
void ice::startListener(const int& listenPort)
{
  if(status_listener_started) return;
  cout << "Creating a CEMon listener object..."<<endl;
  listener = new util::eventStatusListener(listenPort);
  while(!listener->bind()) {
    cout << "error message=" << listener->getErrorMessage() << endl;
    cout << "error code   =" << listener->getErrorCode() << endl;
    cout << "Retrying in 5 seconds..." <<endl;
    sleep(5);
  }
  
  cout << "Creating thread object for CEMon listener..."<<endl;
  
  listenerThread = new util::thread(*listener);
  
  cout << "Starting CEMon listener thread..."<<endl;
  
  try {
    listenerThread->start();
  } catch(util::thread_start_ex& ex) {
    throw iceInit_ex(ex.what()); 
  }  
  cout << "listener started succesfully!"<<endl;
  status_listener_started = true;
}    

//______________________________________________________________________________
void ice::startPoller(const string& hostCert, const int& poller_delay)
{
  if(status_poller_started) return;
  cout << "Creating a Cream status poller object..."<<endl;
  try {
    poller = new util::eventStatusPoller(hostCert, poller_delay, this);
  } catch(glite::wms::ice::util::eventStatusPoller_ex& ex) {
    throw iceInit_ex(ex.what());
  }
  
  cout << "Creating thread object for Cream status poller..."<<endl;
  
  pollerThread = new util::thread(*poller);
  
  cout << "Starting Cream status poller thread..."<<endl;
  
  try {
    pollerThread->start();
  } catch(util::thread_start_ex& ex) {
    throw iceInit_ex(ex.what()); 
  }  
  cout << "poller started succesfully!"<<endl;
  status_poller_started = true;
}

//______________________________________________________________________________
void ice::stopListener() {
  listenerThread->stop();
}

//______________________________________________________________________________
void ice::stopPoller() {
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
  try{requests = fle.get_all_available();}
  catch(exception& ex) {
    cerr << ex.what()<<endl;
    exit(1);
  }
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

//______________________________________________________________________________
void ice::doOnJobFailure(const string& gid) {
  string resub_request = "[ version = \"1.0.0\"; command = \"jobresubmit\"; arguments = [ id = \"" + gid + "\" ] ]";
  FileListMutex mx(flns);
  FileListLock  lock(mx);
  try {
    cout << "Putting >"<<resub_request<<"< to NS filelist"<<endl;
    flns.push_back(resub_request);
  } catch(std::exception& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
}

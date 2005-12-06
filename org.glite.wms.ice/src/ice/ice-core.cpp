
#include "ice-core.h"
#include "jobCache.h"
#include "jobRequest.h"
#include <exception>
#include <iostream>
#include <unistd.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

using namespace glite::wms::ice;
using namespace glite::wms::common::utilities;
using namespace std;

typedef vector<string>::iterator vstrIt;

//______________________________________________________________________________
ice::ice(const string& NS_FL, 
	 const string& WM_FL
	 ) throw(iceInit_ex&)
  : status_listener_started(false), 
    status_poller_started(false),
    ns_filelist(NS_FL), 
    wm_filelist(WM_FL),
    fle(WM_FL.c_str())
{
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
  if(status_listener_started) {
    cout << "Waiting for listener termination..."<<endl;
    this->stopListener();
    listenerThread->join();
    cout << "Listener finished"<<endl;
    delete(listenerThread);
  }
  if(status_poller_started) {
    cout << "Waiting for poller termination..."<<endl;
    this->stopPoller();
    pollerThread->join();
    cout << "Poller finished"<<endl;
    delete(pollerThread);
  }
}

//______________________________________________________________________________
void ice::startListener(const int& listenPort)
{
  if(status_listener_started) return;
  cout << "Creating a CEMon listener object..."<<endl;
  listener = boost::shared_ptr<util::eventStatusListener>(new util::eventStatusListener(listenPort, "", ""));
  while(!listener->bind()) {
    cout << "error message=" << listener->getErrorMessage() << endl;
    cout << "error code   =" << listener->getErrorCode() << endl;
    cout << "Retrying in 5 seconds..." <<endl;
    sleep(5);
  }
  
  cout << "Creating thread object for CEMon listener..."<<endl;
   
  /**
   * The folliwing line requires that the copy ctor of CEConsumer
   * class be public(protected?)
   *
   */
  try {
    listenerThread = 
      new boost::thread(boost::bind(&util::eventStatusListener::operator(), 
				    listener)
			);
  } catch(boost::thread_resource_error& ex) {
    iceInit_ex( ex.what() );
  }
  //  cout << "Starting CEMon listener thread..."<<endl;
  
//   try {
//     listenerThread->start();
//   } catch(util::thread_start_ex& ex) {
//     throw iceInit_ex(ex.what()); 
//   }  
  cout << "listener started succesfully!"<<endl;
  status_listener_started = true;
}    

//______________________________________________________________________________
void ice::startPoller(const int& poller_delay)
{
  if(status_poller_started) return;
  cout << "Creating a Cream status poller object..."<<endl;
  //try {
    
  
  poller = boost::shared_ptr<util::eventStatusPoller>(new util::eventStatusPoller(this, poller_delay));

  cout << "Starting Cream status poller thread..."<<endl;

  try {
    pollerThread = 
      new boost::thread(boost::bind(&util::eventStatusPoller::operator(), 
				    poller)
			);
  } catch(boost::thread_resource_error& ex) {
    iceInit_ex( ex.what() );
  }

  cout << "poller started succesfully!"<<endl;
  status_poller_started = true;
}

//______________________________________________________________________________
void ice::stopListener() {
  listener->stop();
}

//______________________________________________________________________________
void ice::stopPoller() {
  poller->stop();
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
  for(unsigned j=0; j < requests.size(); j++)  
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

  boost::replace_first( toResubmit, "jobsubmit", "jobresubmit");

//   unsigned int pos = toResubmit.find("jobsubmit");
//   if(pos == string::npos) return;
//   string tmp = toResubmit.substr(0,pos);
//   tmp += "jobresubmit";
//   tmp += toResubmit.substr(pos+9, string::npos);

  try {
    cout << "ice::ungetRequest: Putting ["<<toResubmit<<"] to NS filelist"<<endl;
    flns.push_back(toResubmit);
  } catch(std::exception& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }
}

//______________________________________________________________________________
void ice::doOnJobFailure(const string& gid) {
  string resub_request = string("[ version = \"1.0.0\";")
    +" command = \"jobresubmit\"; arguments = [ id = \"" + gid + "\" ] ]";
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

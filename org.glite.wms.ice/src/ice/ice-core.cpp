
#include "ice-core.h"
#include "jobCache.h"
#include "jobRequest.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include <exception>
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
    fle(WM_FL.c_str()),
    log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger())
{
  log_dev->log(log4cpp::Priority::INFO,
	       "Initializing File Extractor object...");
  try{
    flns.open(NS_FL.c_str());
  }
  catch(std::exception& ex) {
    throw iceInit_ex(ex.what());
  } catch(...) {
    log_dev->log(log4cpp::Priority::ERROR,
		 "Catched unknown exception");
    exit(1);
  }
}

//______________________________________________________________________________
ice::~ice() 
{ 
  if(status_listener_started) {
    log_dev->log(log4cpp::Priority::INFO,
		 "Waiting for listener termination...");
    this->stopListener();
    listenerThread->join();
    log_dev->log(log4cpp::Priority::INFO,
		 "Listener finished");
    delete(listenerThread);
  }
  if(status_poller_started) {
    log_dev->log(log4cpp::Priority::INFO,
		 "Waiting for poller termination...");
    this->stopPoller();
    pollerThread->join();
    log_dev->log(log4cpp::Priority::INFO,
		 "Poller finished");
    delete(pollerThread);
  }
}

//______________________________________________________________________________
void ice::startListener(const int& listenPort)
{
  if(status_listener_started) return;
  log_dev->log(log4cpp::Priority::INFO,
	       "Creating a CEMon listener object...");
  listener = boost::shared_ptr<util::eventStatusListener>(new util::eventStatusListener(listenPort,util::iceConfManager::getInstance()->getHostProxyFile()));
  while(!listener->bind()) {
    log_dev->log(log4cpp::Priority::ERROR,
		 string("Bind error: ")+listener->getErrorMessage()
		 +" - error code="+listener->getErrorCode());
    log_dev->log(log4cpp::Priority::ERROR,
		 "Retrying in 5 seconds...");
    sleep(5);
  }

  log_dev->log(log4cpp::Priority::INFO,
	       "Creating a CEMon subscription updater...");
  subsUpdater = boost::shared_ptr<util::subscriptionUpdater>(new util::subscriptionUpdater(util::iceConfManager::getInstance()->getHostProxyFile()));

  log_dev->log(log4cpp::Priority::INFO,
	       "Creating thread object for CEMon listener...");
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
  log_dev->log(log4cpp::Priority::INFO,
	       "listener started succesfully !");
  status_listener_started = true;

  log_dev->log(log4cpp::Priority::INFO,
	       "Creating thread object for Subscription updater...");
  try {
    updaterThread =
      new boost::thread(boost::bind(&util::subscriptionUpdater::operator(),
			subsUpdater)
			);
  } catch(boost::thread_resource_error& ex) {
    iceInit_ex( ex.what() );
  }
  log_dev->log(log4cpp::Priority::INFO,
	       "Subscription updater started succesfully !");
}

//______________________________________________________________________________
void ice::startPoller(const int& poller_delay)
{
  if(status_poller_started) return;
  log_dev->log(log4cpp::Priority::INFO,
	       "Creating a Cream status poller object...");

  poller = boost::shared_ptr<util::eventStatusPoller>(new util::eventStatusPoller(this, poller_delay));

  log_dev->log(log4cpp::Priority::INFO,
	       "Starting Cream status poller thread...");

  try {
    pollerThread = 
      new boost::thread(boost::bind(&util::eventStatusPoller::operator(), 
				    poller)
			);
  } catch(boost::thread_resource_error& ex) {
    iceInit_ex( ex.what() );
  }

  log_dev->log(log4cpp::Priority::INFO,
	       "poller started succesfully !");
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
    log_dev->log(log4cpp::Priority::ERROR,
		 ex.what());
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

  try {
    log_dev->log(log4cpp::Priority::INFO,
		 string("Putting [")
		 +toResubmit+"] to WM's Input file");
    flns.push_back(toResubmit);
  } catch(std::exception& ex) {
    log_dev->log(log4cpp::Priority::ERROR,
		 ex.what());
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
    log_dev->log(log4cpp::Priority::INFO,
		 string("Putting [")
		 +resub_request+"] to WM's Input file");
    flns.push_back(resub_request);
  } catch(std::exception& ex) {
    log_dev->log(log4cpp::Priority::ERROR,
		 ex.what());
    exit(1);
  }
}


#include "ice-core.h"
#include "jobCache.h"
#include "jobRequest.h"
#include "subscriptionManager.h"
#include "subscriptionCache.h"
#include "iceEventLogger.h"

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
  : ns_filelist(NS_FL),
    wm_filelist(WM_FL),
    fle(WM_FL.c_str()),
    log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger())
{
  log_dev->log(log4cpp::Priority::INFO,
	       "ice::ice() - Initializing File Extractor object...");
  try{
    flns.open(NS_FL.c_str());
  }
  catch(std::exception& ex) {
    throw iceInit_ex(ex.what());
  } catch(...) {
    log_dev->log(log4cpp::Priority::ERROR,
		 "ice::ice() - Catched unknown exception");
    exit(1);
  }

  bool _tmp_start_listener;
  {
    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
    _tmp_start_listener = util::iceConfManager::getInstance()->getStartListener();
  }

  if( _tmp_start_listener ) {
  /**
   * The listener and the iceCommandSubmit need to subscribe to CEMon in order
   * to make ICE able to receive job status notifications.
   * So now as preliminary operation it's the case to check that the
   * subscriptionManager singleton can be created without problems.
   *
   * The subscriptionManager initialization also setup authentication.
   */
   {
    boost::recursive_mutex::scoped_lock M( util::subscriptionManager::mutex );
    util::subscriptionManager::getInstance();
    if( !util::subscriptionManager::getInstance()->isValid() ) {
      log_dev->errorStream() << "ice::CTOR() - "
                             << "Fatal error creating the subscriptionManager instance. Will not start listener."
	  		   << log4cpp::CategoryStream::ENDLINE;
      //_isOK = false;
      //exit(1); // FATAL, I think is right to exit
      //return;
      boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
      util::iceConfManager::getInstance()->setStartListener( false );
    }
   }
    /**
     * subscriptionCache is used to retrieve the list of cemon we're
     * subscribed. If it's creation failed, it is not the case (at 0-order)
     * to use the listener...
     *
     */
    {
      boost::recursive_mutex::scoped_lock M( util::subscriptionCache::mutex );
      if( util::subscriptionCache::getInstance() == NULL ) {
	  log_dev->errorStream() << "ice::CTOR() - "
                               << "Fatal error creating the subscriptionCache instance. Will not start listener."
	    		       << log4cpp::CategoryStream::ENDLINE;
	  boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
          util::iceConfManager::getInstance()->setStartListener( false );
      }
    }
  }
}

//______________________________________________________________________________
ice::~ice()
{
  if( listener && listener->isRunning() ) {
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Waiting for listener termination...");
    listener->stop();
    listenerThread->join();
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Listener finished");
    delete(listenerThread);
  }
  if ( subsUpdater && subsUpdater->isRunning() ) {
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Waiting for poller termination...");
    subsUpdater->stop();
    pollerThread->join();
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Poller finished");
    delete(pollerThread);
  }
  if ( lease_updater && lease_updater->isRunning() ) {
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Waiting for lease updater termination...");
    lease_updater->stop( );
    lease_updaterThread->join();
    log_dev->log(log4cpp::Priority::INFO,
		 "ice::~ice() - Lease updater finished");
    delete(lease_updaterThread);
  }

}

//______________________________________________________________________________
void ice::startListener(const int& listenPort)
{
  if ( listener && listener->isRunning() ) 
      return;



  log_dev->log(log4cpp::Priority::INFO,
	       "ice::startListener() - Creating a CEMon listener object...");
  {
    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
    listener = boost::shared_ptr<util::eventStatusListener>(new util::eventStatusListener(listenPort,util::iceConfManager::getInstance()->getHostProxyFile()));
  }
  if( !listener->isOK() ) {
      log_dev->log(log4cpp::Priority::ERROR, "CEMon listener creation went wrong. Won't start it.");
      // this must be set because other pieces of code
      // have a behaviour that depends on the listener is running or not
      boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
      util::iceConfManager::getInstance()->setStartListener( false );
      return;
  }
  while(!listener->bind()) {
      log_dev->log(log4cpp::Priority::ERROR,
                   string("ice::startListener() - Bind error: ")+listener->getErrorMessage()
                   +" - error code="+listener->getErrorCode());
      log_dev->log(log4cpp::Priority::ERROR,
                   "Retrying in 5 seconds...");
      sleep(5);
  }

  log_dev->log(log4cpp::Priority::INFO,
	       "ice::startListener() - Creating thread object for CEMon listener...");
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
	       "ice::startListener() - listener started succesfully !");
  
  //-----------------now is time to start subUpdater---------------------------
  bool _tmp_start_sub_updater;
  {
    boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
    _tmp_start_sub_updater = util::iceConfManager::getInstance()->getStartSubscriptionUpdater();
  }
  if( _tmp_start_sub_updater ) {
      log_dev->log(log4cpp::Priority::INFO,
                   "ice::startListener() - Creating a CEMon subscription updater...");
      {
        boost::recursive_mutex::scoped_lock M( util::iceConfManager::mutex );
        subsUpdater = boost::shared_ptr<util::subscriptionUpdater>(new util::subscriptionUpdater(util::iceConfManager::getInstance()->getHostProxyFile()));
      }
      log_dev->log(log4cpp::Priority::INFO,
                   "ice::startListener() - Creating thread object for Subscription updater...");
      try {
          updaterThread =
              new boost::thread(boost::bind(&util::subscriptionUpdater::operator(),
                                            subsUpdater)
                                );
      } catch(boost::thread_resource_error& ex) {
          iceInit_ex( ex.what() );
      }
      log_dev->log(log4cpp::Priority::INFO,
                   "ice::startListener() - Subscription updater started succesfully !");
  }
}

//______________________________________________________________________________
void ice::startPoller(const int& poller_delay)
{
    if ( poller && poller->isRunning() ) 
        return;

    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startPoller() - Creating a Cream status poller object...");
    
    poller = boost::shared_ptr<util::eventStatusPoller>(new util::eventStatusPoller(this, poller_delay));
    
    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startPoller() - Starting Cream status poller thread...");
    
    try {
        pollerThread = 
            new boost::thread(boost::bind(&util::eventStatusPoller::operator(), 
                                          poller)
                              );
    } catch(boost::thread_resource_error& ex) {
        iceInit_ex( ex.what() );
    }
    
    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startPoller() - Poller started succesfully !");
}

//------------------------------------------------------------------------------
void ice::startLeaseUpdater( void ) {
    if ( lease_updater && lease_updater->isRunning() )
        return ;

    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startLeaseUpdater() - Creating a Cream lease updater object...");
    
    lease_updater = boost::shared_ptr<util::leaseUpdater>(new util::leaseUpdater( ) );
    
    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startLeaseUpdater() - Starting Cream lease updater thread...");
    
    try {
        lease_updaterThread = 
            new boost::thread(boost::bind(&util::leaseUpdater::operator(), 
                                          lease_updater)
                              );
    } catch(boost::thread_resource_error& ex) {
        iceInit_ex( ex.what() );
    }
    
    log_dev->log(log4cpp::Priority::INFO,
                 "ice::startLeaseUpdater() - Lease updater succesfully !");
}


//______________________________________________________________________________
//void ice::stopListener() {
//  listener->stop();
//}

//______________________________________________________________________________
//void ice::stopPoller() {
//  poller->stop();
//}

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
		 string("ice::ungetRequest() - Putting [")
		 +toResubmit+"] to WM's Input file");
    flns.push_back(toResubmit);
  } catch(std::exception& ex) {
    log_dev->log(log4cpp::Priority::ERROR,
		 ex.what());
    exit(1);
  }
}

//______________________________________________________________________________
void ice::resubmit_job( util::CreamJob& j ) 
{
    util::iceEventLogger* _ev_logger= util::iceEventLogger::instance();

    string resub_request = string("[ version = \"1.0.0\";")
        +" command = \"jobresubmit\"; arguments = [ id = \"" + j.getGridJobID() + "\" ] ]";
    FileListMutex mx(flns);
    FileListLock  lock(mx);
    try {
        log_dev->log(log4cpp::Priority::INFO,
                     string("ice::doOnJobFailure() - Putting [")
                     +resub_request+"] to WM's Input file");
        _ev_logger->ns_enqueued_start_event( j, ns_filelist );
        flns.push_back(resub_request);
        _ev_logger->ns_enqueued_ok_event( j, ns_filelist );
    } catch(std::exception& ex) {
        log_dev->log(log4cpp::Priority::ERROR, ex.what());
        _ev_logger->ns_enqueued_fail_event( j, ns_filelist );
        exit(1); // FIXME: Should we keep going?
    }
}

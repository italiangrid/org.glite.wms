#ifndef __GLITE_WMS_ICE_UTIL_EVENTSTATUSLISTENER_H__
#define __GLITE_WMS_ICE_UTIL_EVENTSTATUSLISTENER_H__

#include "glite/ce/monitor-client-api-c/CEConsumer.h"

//#undef soapStub_H
#include "glite/ce/monitor-client-api-c/CEPing.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "boost/thread/recursive_mutex.hpp"
//#include "classad_distribution.h"
//#include "ClassadSyntax_ex.h"

// Forward declaration for the logger
namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

          class iceConfManager;
          class iceEventLogger;

	//! A class that receives notification from CEMon about job status changes
	/**!
	   \class eventStatusListener
	   This class is conceived to run as a boost::thread (this is the
	   motivation of the implementation of the operator()() ).
	   Its main purpose is to receive notifications from CEMon about all job status chages; the CEMon sending notifications runs on the same host of the CREAM service. In order to receive notifications, the listener must be subscribed to that CEMon service.

	*/
	class eventStatusListener : public CEConsumer  {
	  std::string grid_JOBID, cream_JOBID;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  bool endaccept;
 	  CESubscription subscriber;
 	  CESubscriptionMgr subManager;
	  CEPing *pinger;
	  Topic T;
	  Policy P;
	  std::vector<std::string> activeSubscriptions;
	  std::string proxyfile;
	  int tcpport;
	  std::string myname;
	  //std::map<std::string, bool> cemon_subscribed_to;
	  glite::wms::ice::util::iceConfManager* conf;
          log4cpp::Category *log_dev;
          iceEventLogger *_ev_logger;
          // classad::ClassAdParser parser;

	  void init(void);
          // void parseEventJobStatus( std::string& cream_job_id, std::string& job_status, long& tstamp, const std::string& _classad ) throw( glite::wms::ice::util::ClassadSyntax_ex& );
          void handleEvent( const Event& ev );

	protected:
	  eventStatusListener(const eventStatusListener&) : 
              CEConsumer(9999),
              pinger(0),
              T(""),
              P(0) 
              {}

	public:
	  static boost::recursive_mutex mutexJobStatusUpdate;

	  eventStatusListener(int i, const std::string& hostcert) ;
	  virtual ~eventStatusListener() {}

	  void acceptJobStatus(void);
	  virtual void operator()();
	  virtual void stop() { endaccept=true; }

	};

      }
    }
  }
}

#endif

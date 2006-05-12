/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE Event Status change listener
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_EVENTSTATUSLISTENER_H
#define GLITE_WMS_ICE_UTIL_EVENTSTATUSLISTENER_H

#include "glite/ce/monitor-client-api-c/CEConsumer.h"

#include "glite/ce/monitor-client-api-c/CEPing.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/scoped_ptr.hpp"

#include "iceThread.h"

// Forward declaration for the logger
namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
          
          class jobCache;
          class iceConfManager;
          class iceLBLogger;
	  class subscriptionManager;

	//! A class that receives notification from CEMon about job status changes
	/**!
	   \class eventStatusListener
	   This class is conceived to run as a boost::thread (this is the
	   motivation of the implementation of the operator()() ).
	   Its main purpose is to receive notifications from CEMon about all job status chages; the CEMon sending notifications runs on the same host of the CREAM service. In order to receive notifications, the listener must be subscribed to that CEMon service.

	*/
	class eventStatusListener : public CEConsumer, public iceThread {
	  std::string grid_JOBID, cream_JOBID;
	  glite::ce::cream_client_api::job_statuses::job_status status;
	  boost::scoped_ptr<CEPing> pinger;
	  std::vector<std::string> activeSubscriptions;
	  std::string proxyfile;
	  int tcpport;
	  std::string myname;
	  glite::wms::ice::util::iceConfManager* conf;
          log4cpp::Category *log_dev;
          iceLBLogger *_lb_logger;
	  bool _isOK;
          jobCache* cache;
	  subscriptionManager* subManager;

	  void init(void);

          void handleEvent( const monitortypes__Event& ev );

	protected:
	  eventStatusListener(const eventStatusListener&) : 
              CEConsumer(9999),
              pinger(0)
              {}
	  void createObject();

	public:
	  static boost::recursive_mutex mutexJobStatusUpdate;

	  eventStatusListener(const int& i, const std::string& hostcert);
	  
	  eventStatusListener(const int& i, 
	     		      const std::string& hostcert,
			      const std::string& cert,
			      const std::string& key);
			      
	  virtual ~eventStatusListener() {}

	  void acceptJobStatus(void);
	  virtual void body( void );
	  bool isOK() const { return _isOK; }
	};

      }
    }
  }
}

#endif

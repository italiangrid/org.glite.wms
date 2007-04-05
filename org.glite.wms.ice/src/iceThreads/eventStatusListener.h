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
#include "iceThread.h"

// Forward declaration for the logger
namespace log4cpp {
    class Category;
};

namespace glite {
  namespace wms {
    namespace common {
      namespace configuration {
        class ICEConfiguration;
      }
    }
  }
}

namespace glite {
  namespace wms {
    namespace ice {
        class Ice;

      namespace util {
          
          class iceConfManager;

	//! A class that receives notification from CEMon about job status changes
	/**!
	   \class eventStatusListener
	   This class is conceived to run as a boost::thread (this is the
	   motivation of the implementation of the operator()() ).
	   Its main purpose is to receive notifications from CEMon about all job status chages; the CEMon sending notifications runs on the same host of the CREAM service. In order to receive notifications, the listener must be subscribed to that CEMon service.

	*/
	class eventStatusListener : public CEConsumer, public iceThread {
	  std::string                                            m_myname;
	  const glite::wms::common::configuration::ICEConfiguration*  m_conf;
          log4cpp::Category                                     *m_log_dev;
	  bool                                                   m_isOK;
          glite::wms::ice::Ice*                                  m_ice_manager;

	protected:

	  eventStatusListener(const eventStatusListener&) : 
              CEConsumer(9999)
              {}

	  void createObject();

	public:

	  eventStatusListener(const int& i, const std::string& hostcert);
	  
	  eventStatusListener(const int& i, 
	     		      const std::string& hostcert,
			      const std::string& cert,
			      const std::string& key);
			      
	  virtual ~eventStatusListener() {}

	  void acceptJobStatus(void);
	  virtual void body( void );
	  bool isOK() const { return m_isOK; }
	};

      }
    }
  }
}

#endif

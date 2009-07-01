/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * ICE status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICECOMMANDSTATUSPOLLER_H
#define GLITE_WMS_ICE_ICECOMMANDSTATUSPOLLER_H

#undef soapStub_H

#include "iceAbsCommand.h"
//#include "creamJob.h"
//#include "glite/ce/cream-client-api-c/JobInfoWrapper.h"
//#include "iceDb/GetJobsToPoll.h" // needed for definition of type 'JobToPoll'

//#include <boost/scoped_ptr.hpp>

#include<string>
#include<list>
#include<utility>
#include<boost/tuple/tuple.hpp>

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace wms {
    namespace ice {

      class Ice;

      namespace util {
        
        class iceLBLogger;
	//	class jobCache;
	class iceConfManager;

	class iceCommandStatusPoller2 : public iceAbsCommand {
	  
	  //static const char    *s_iceid;
	  log4cpp::Category    *m_log_dev;
	  //iceLBLogger          *m_lb_logger;
	  Ice                  *m_iceManager;
	  iceConfManager       *m_conf;
	  bool                  m_stopped;
	  //static time_t         s_last_seen;
	  
	  void getUserDN_CreamURL( std::list< boost::tuple<std::string, std::string, time_t> >& ) const;
	  
	  time_t poll_userdn_ce( const std::string& userdn, 
				 const std::string& cream_url,
				 const time_t last_seen );
	  
	public:
	  iceCommandStatusPoller2( Ice* );
	  virtual ~iceCommandStatusPoller2() throw() { }
	  void execute( ) throw();
	  std::string get_grid_job_id() const { return std::string(); }
	  void stop() { m_stopped = true; }
	  
	};
      }
    }
  }
}

#endif

/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#ifndef GLITE_WMS_ICE_ICECOMMANDEVENTQUERY_H
#define GLITE_WMS_ICE_ICECOMMANDEVENTQUERY_H

#undef soapStub_H

#include "iceAbsCommand.h"
#include "iceUtils/CreamJob.h"

#include <boost/scoped_ptr.hpp>

#include<vector>
#include<string>
#include<list>

namespace log4cpp {
  class Category;
};

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
	
	class EventWrapper;
	
      }
    }
  }
}

namespace glite {
  namespace wms {
    namespace ice {

      class IceCore;

      namespace util {
        
        class iceLBLogger;
	class IceConfManager;

	class iceCommandEventQuery : public iceAbsCommand {

          log4cpp::Category                     *m_log_dev;
          glite::wms::ice::util::iceLBLogger    *m_lb_logger;
          IceCore                               *m_iceManager;
	  glite::wms::ice::util::IceConfManager *m_conf;
	  bool                                   m_stopped;
	  const std::string                      m_dn, m_ce;
	  
	  long long getEventID( const std::string&, const std::string& );
	  bool checkDatabaseID( const std::string&, const long long, long long& );
	  long long processEvents( const std::string& endpoint, std::list<glite::ce::cream_client_api::soap_proxy::EventWrapper*>& );
	  void processEventsForJob( const std::string&,
				    const std::list<glite::ce::cream_client_api::soap_proxy::EventWrapper*>& );
	  
	  void processSingleEvent( CreamJob&, 
				   glite::ce::cream_client_api::soap_proxy::EventWrapper*,
				   const bool is_last_one_event,
				   bool& );

	  //bool ignore_job( const std::string& CID, CreamJob& tmp_job, std::string& reason );
	   
	  void setEventID( const std::string&, const std::string&, const long long );

	  void getJobsByDbID( std::list<glite::wms::ice::util::CreamJob>& jobs, const long long db_id );

	  //	  void deleteJobsByDN( void ) throw();

	public:
	  iceCommandEventQuery( IceCore*, const std::string& dn, const std::string& ce );
	  ~iceCommandEventQuery( ) throw();// {}
	  
	  void execute( const std::string& ) throw();
	  
	  std::string get_grid_job_id() const;// { return std::string(); }
	  
	  void stop() { m_stopped = true; }
	  
	};
      }
    }
  }
}

#endif

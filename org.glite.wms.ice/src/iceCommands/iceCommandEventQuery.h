#ifndef GLITE_WMS_ICE_ICECOMMANDEVENTQUERY_H
#define GLITE_WMS_ICE_ICECOMMANDEVENTQUERY_H

#undef soapStub_H

#include "iceAbsCommand.h"
#include "creamJob.h"

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

      class Ice;

      namespace util {
        
        class iceLBLogger;
	class iceConfManager;

	class iceCommandEventQuery : public iceAbsCommand {

          log4cpp::Category                     *m_log_dev;
          glite::wms::ice::util::iceLBLogger    *m_lb_logger;
          Ice                                   *m_iceManager;
	  glite::wms::ice::util::iceConfManager *m_conf;
	  bool                                   m_stopped;
	  //	  const std::vector<std::string>         m_dnce_array;
	  const std::string                      m_dn, m_ce;
	  
	  long long getEventID( const std::string&, const std::string& );
	  bool checkDatabaseID( const std::string&, const long long );
	  long long processEvents( std::list<glite::ce::cream_client_api::soap_proxy::EventWrapper*>& );
	  void processEventsForJob( const std::string&,
				    const std::list<glite::ce::cream_client_api::soap_proxy::EventWrapper*>& );
	  
	  void processSingleEvent( CreamJob&, 
				   glite::ce::cream_client_api::soap_proxy::EventWrapper*,
				   const bool is_last_one_event,
				   bool& );

	  void setEventID( const std::string&, const std::string&, const long long );

	  void getJobsByDbID( std::list<glite::wms::ice::util::CreamJob>& jobs, const long long db_id );

	public:
	  iceCommandEventQuery( Ice*, const std::string& dn, const std::string& ce );
	  ~iceCommandEventQuery( ) throw() {}
	  
	  void execute( ) throw();
	  
	  std::string get_grid_job_id() const { return std::string(); }
	  
	  void stop() { m_stopped = true; }
	  
	};
      }
    }
  }
}

#endif

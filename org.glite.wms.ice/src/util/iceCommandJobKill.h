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
 * ICE job killer
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
 
#ifndef GLITE_WMS_ICE_ICECOMMANDJOBKILL_H
#define GLITE_WMS_ICE_ICECOMMANDJOBKILL_H

#include "iceAbsCommand.h"
//#include "jobCache.h"
//
//#include "creamJob.h"

#include <boost/scoped_ptr.hpp>

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
        class CreamJob;
	
      }
      }
      }
      };

namespace glite {
  namespace ce {
    namespace cream_client_api {
      namespace soap_proxy {
        
        class CreamProxy;
	class iceLBLogger;
	
      }
    }
  }
};

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {

namespace common {
namespace configuration {
    class Configuration;
} // namespace configuration
} // namespace common

namespace ice {
     
// Forward declarations
namespace util {                
    class iceConfManager;
    class iceLBLogger;
}
     
  
 class iceCommandJobKill : public iceAbsCommand {
   
   boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
   glite::wms::ice::util::CreamJob* m_theJob;
   log4cpp::Category *m_log_dev;
   time_t m_threshold_time;
   //time_t m_delay;
   glite::wms::ice::util::iceLBLogger* m_lb_logger;
   void killJob( glite::wms::ice::util::CreamJob&, time_t );
   
  public:
   iceCommandJobKill( glite::ce::cream_client_api::soap_proxy::CreamProxy*, 
   		      glite::wms::ice::util::CreamJob*) throw();
   
   virtual void execute( ) throw( );
   
   virtual ~iceCommandJobKill() {  }
 };

} // namespace ice
} // namespace wms
} // namespace glite


#endif

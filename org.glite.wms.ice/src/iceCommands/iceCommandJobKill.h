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
 * ICE job killer
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef GLITE_WMS_ICE_ICECOMMANDJOBKILL_H
#define GLITE_WMS_ICE_ICECOMMANDJOBKILL_H

#include "iceAbsCommand.h"
#include "creamJob.h"
#include "iceUtils.h"

#include <boost/scoped_ptr.hpp>

/* namespace glite { */
/*     namespace ce { */
/*         namespace cream_client_api { */
/*             namespace soap_proxy { */
                
/*                 class CreamProxy; */
                
/*             } */
/*         } */
/*     } */
/* }; */

namespace log4cpp {
    class Category;
};

namespace glite {
namespace wms {
namespace ice {
            
namespace util {                
    class iceLBLogger;
    
    class iceCommandJobKill : public iceAbsCommand {
        
      //boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
        //glite::wms::ice::util::CreamJob m_theJob;
        log4cpp::Category *m_log_dev;
        time_t m_threshold_time;
        glite::wms::ice::util::iceLBLogger* m_lb_logger;
        //void killJob( const time_t ) throw();
        void killJob( const std::pair< std::pair<std::string, std::string>, std::list< glite::wms::ice::util::CreamJob > >& ) throw();
	bool cancel_jobs(const std::string& proxy, const std::string& endpoint, 
					    const std::vector<std::string>& jobIdList) throw();
	void updateCacheAndLog( const std::pair< std::pair<std::string, std::string>, std::list< glite::wms::ice::util::CreamJob > >& aList ) throw();
	void checkExpiring( std::list<glite::wms::ice::util::CreamJob>& ) throw();

    public:
        iceCommandJobKill( /*const glite::wms::ice::util::CreamJob&*/ ) throw();
        
        virtual void execute( ) throw( );
        
        virtual ~iceCommandJobKill() 
        {  }
        
        std::string get_grid_job_id( void ) const 
        { 
	  return "";//m_theJob.getGridJobID(); 
        }
    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite


#endif

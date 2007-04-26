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
 * ICE lease updater
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
 
#ifndef GLITE_WMS_ICE_ICECOMMANDLEASEUPD_H
#define GLITE_WMS_ICE_ICECOMMANDLEASEUPD_H

#include "iceAbsCommand.h"
#include "creamJob.h"
#include "jobCache.h"
#include "iceUtils.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include <boost/scoped_ptr.hpp>

#include <ctime>

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {
namespace ice {
namespace util {
	
    class iceLBLogger;
    
    class iceCommandLeaseUpdater : public iceAbsCommand {
        boost::scoped_ptr< glite::ce::cream_client_api::soap_proxy::CreamProxy > m_theProxy;
        log4cpp::Category *m_log_dev;
        glite::wms::ice::util::iceLBLogger* m_lb_logger;
        time_t m_delta;
        glite::wms::ice::util::jobCache* m_cache;
        
      //void update_lease_for_job( const CreamJob& j ) throw();
      void update_lease_for_multiple_jobs( const std::vector<std::string>& jobids, const std::string& userproxy, const std::string& endpoint ) throw();
      void check_lease_expired( const CreamJob& ) throw();
      void handle_jobs(const std::pair< std::pair<std::string, std::string>, std::list< CreamJob > >&) throw();

    public:
        iceCommandLeaseUpdater( ) throw();
        
        ~iceCommandLeaseUpdater( ) throw() { }
        
        void execute( ) throw();
                
        std::string get_grid_job_id( void ) const { return std::string(); } 
        
    };
    
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif

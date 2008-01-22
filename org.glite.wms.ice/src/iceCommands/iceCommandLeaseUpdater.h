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

#include <ctime>

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {
namespace ice {
namespace util {
	
    class iceLBLogger;
    class Lease_manager;

    class iceCommandLeaseUpdater : public iceAbsCommand {
      
        log4cpp::Category *m_log_dev;
        glite::wms::ice::util::iceLBLogger* m_lb_logger;
        time_t m_frequency;
        glite::wms::ice::util::jobCache* m_cache;
        bool m_only_update;
        Lease_manager* m_lease_manager;

        /**
         * Returns true iff the CreamJob job can be removed from the
         * job cache because its lease expired.
         */
        bool job_can_be_removed( const CreamJob& job ) const throw();

        /**
         * This method returns true iff the lease associated to CreamJob 
         * job must be renewed.
         */ 
        bool lease_can_be_renewed( const CreamJob& job ) const throw();
      
    public:
      /**
       * Se only_update==true, si aggiornano i lease SENZA rimuovere i
       * job per cui il lease e' scaduto.
       */
        iceCommandLeaseUpdater( bool only_update = false ) throw();
        
        ~iceCommandLeaseUpdater( ) throw() { }
        
        void execute( ) throw();
                
        std::string get_grid_job_id( void ) const { return std::string(); } 
        
    };
    
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif

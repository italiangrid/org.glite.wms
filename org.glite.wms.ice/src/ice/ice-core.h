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
 * ICE core class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_ICE_CORE_H
#define GLITE_WMS_ICE_ICE_CORE_H

#include "iceInit_ex.h"
#include "eventStatusListener.h"
#include "subscriptionUpdater.h"
#include "eventStatusPoller.h"
#include "leaseUpdater.h"
#include "proxyRenewal.h"
#include "jobKiller.h"
#include "creamJob.h"
#include "iceThread.h"
#include "jobCache.h"
#include "filelist_request.h"

#include "glite/wms/common/utilities/FLExtractor.h"

#include "ClassadSyntax_ex.h"

#include <string>

typedef glite::wms::common::utilities::FLExtractor<std::string>::iterator FLEit;

namespace boost {
  class thread;
};

namespace log4cpp {
  class Category;
};

namespace glite {
namespace wms {
    namespace common {
        namespace configuration {
            class Configuration;
        }
    }

namespace ice {

    namespace util {
        class iceLBLogger;
    };
    
    class Ice {
        
        class IceThreadHelper { 
        public:
            IceThreadHelper( const std::string& name );
            virtual ~IceThreadHelper( );
            
            /**
             * Associates a newly created iceThread object
             * to this thread, and starts it. 
             *
             * @param obj a pointer to the iceThread object from
             * which a thread should be created. The caller
             * transfers ownership of parameter obj.
             */
            void start( util::iceThread* obj ) throw( iceInit_ex& );

            /**
             * Returns true iff this thread started (i.e., iff
             * this thread as an associated running iceThread
             * object). False otherwise.
             */
            bool is_started( void ) const;
        protected:
            
            /**
             * Stops this thread. This method has no effect if the
             * start() method was not called first, or if the
             * start() method raised an exception.
             */
            void stop( void );
            
            const std::string m_name; //! The name of this thread (can be any string)
            boost::thread* m_thread; //! The dynamically created thread object. 
            boost::shared_ptr< util::iceThread > m_ptr_thread; //! Used to instantiate the thread.
            log4cpp::Category* m_log_dev; //! Logging device
        };
        
        IceThreadHelper m_listener_thread;
        IceThreadHelper m_poller_thread;
        IceThreadHelper m_updater_thread;
        IceThreadHelper m_lease_updater_thread;
        IceThreadHelper m_proxy_renewer_thread;
        IceThreadHelper m_job_killer_thread;
        
        glite::wms::common::utilities::FileList<std::string> m_flns;
        
        log4cpp::Category* m_log_dev;
        
        glite::wms::ice::util::iceLBLogger* m_lb_logger;
        glite::wms::ice::util::jobCache* m_cache;
        glite::wms::common::configuration::Configuration* m_configuration;
        std::string m_ns_filelist;
        glite::wms::common::utilities::FLExtractor<std::string> m_fle;

        static glite::wms::ice::Ice* s_instance; ///< Singleton instance of this class
        
        Ice( ) throw(glite::wms::ice::iceInit_ex&);

        // Some utility functions
        void deregister_proxy_renewal( const util::CreamJob& job );
        void purge_wms_storage( const util::CreamJob& job );
        
    public:
        
        virtual ~Ice();
        
        // void clearRequests();
        void getNextRequests(std::vector< filelist_request >&);
        void removeRequest( const filelist_request& r );

        // Starter methods
        void startListener( void );
        void startPoller( void );
        void startLeaseUpdater( void );
        void startProxyRenewer( void );
        void startJobKiller( void );
        
        // Query methods
        bool is_listener_started( void ) const;
        bool is_poller_started( void ) const;
        bool is_lease_updater_started( void ) const;
        bool is_proxy_renewer_started( void ) const;
        bool is_job_killer_started( void ) const;

        // Misc job handling functions
        
        /**
         * Resubmits a failed job. This method takes care of logging
         * the relevant L&B events. The job is NOT removed from the
         * job cache!
         *
         * @param the_job the job to resubmit.
         *
         * @param reason the reason why this job is being resubmitted
         */
        void resubmit_job( util::CreamJob& the_job, const std::string& reason );
        
        /**
         * Purge a cancelled/terminated job. This method takes care
         * of logging the relevant L&B events. The job will be
         * removed from the jobCache.
         *
         * @param j an iterator to the job to purge. If j ==
         * jobCache.end(), the not action is done.
         *
         * @param reason the reason why the job is being purged
         *
         * @return an iterator pointing to the next job in cache, if
         * the purge(and subsequent removal) was succesful. jit if
         * not.
         */
        util::jobCache::iterator purge_job( util::jobCache::iterator j, const std::string& reason );
        
        /**
         * Resubmits or purge a given job.
         *
         * @param it the iterator to the job to purge or
         * resubmit. If it == end(), then nothing is done and this
         * method returns end();
         *
         * @return an iterator to the current job, if it is neither
         * purged nor resubmitted. An interator to the next job, if
         * the job is purged or resubmitted (and hence removed from
         * the job cache).
         */
        util::jobCache::iterator resubmit_or_purge_job( util::jobCache::iterator it );
        
        /**
         * returns the singleton instance of this class.
         */
        static glite::wms::ice::Ice* instance( void );
        
    }; // class ice
    
} // namespace ice
} // namespace ce
} // namespace glite

#endif

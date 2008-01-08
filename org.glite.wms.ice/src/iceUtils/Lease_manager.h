/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * youw may not use this file except in compliance with the License. 
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
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef ICE_LEASE_MANAGER_H
#define ICE_LEASE_MANAGER_H

#include "creamJob.h"

#include <iostream>
#include <string>
#include <functional>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/sequenced_index.hpp"
#include "boost/multi_index/composite_key.hpp"
#include "boost/thread/recursive_mutex.hpp"

namespace log4cpp {
    class Category;
}

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Lease_manager {
    protected:
        Lease_manager( );

        /**
         * Iterates over the lease ID set. Removes all lease IDs which
         * are associated to already expired leases.
         */
        void purge_old_lease_ids( void );

        static Lease_manager* s_instance;
        static boost::recursive_mutex m_mutex;

        log4cpp::Category* m_log_dev;
        unsigned int m_operation_count;
        const size_t m_max_size; ///< Maximum size of the delegation cache
        const unsigned int m_operation_count_max;

        /**
         * Entry of the delegation table
         */
        struct table_entry {
            std::string m_user_dn;
            std::string m_cream_url;
            time_t m_expiration_time;
            std::string m_lease_id;

            table_entry( const std::string& user_dn, const std::string& cream_url, time_t expiration_time, const std::string& lease_id ) :
                m_user_dn( user_dn ),
                m_cream_url( cream_url ),
                m_expiration_time( expiration_time ),
                m_lease_id( lease_id )
            { };
        };

        /**
         * Multi index container 
         */
        typedef boost::multi_index_container<
            table_entry,
            boost::multi_index::indexed_by<
              boost::multi_index::ordered_unique<
            // The index here is the (sha1_digest, cream_url) pair
                boost::multi_index::composite_key<
                table_entry,
            // The first composite element is the sha1_digest
                boost::multi_index::member<table_entry,std::string,&table_entry::m_user_dn>,
            // The second composite element is the cream_url
                boost::multi_index::member<table_entry,std::string,&table_entry::m_cream_url>
              > 
            >,
            boost::multi_index::ordered_non_unique<
              boost::multi_index::member<table_entry,time_t,&table_entry::m_expiration_time>
            >,
            boost::multi_index::sequenced<>
          >
        > t_lease_set;
        
        t_lease_set m_lease_set;

    public:
        ~Lease_manager( ) { };

        /**
         * Returns the singleton instance of this class. This method
         * is thread-safe: a mutex is acquired before checking whether
         * the instance already exists or must be created.
         *
         * @return The singleton instance of this class. The caller
         * DOES NOT OWN the returned pointer, and must NOT be
         * deallocated.
         */
        static Lease_manager* instance( );

        /**
         * Returns a lease ID to be used for a given job.
         *
         * This method must be called before submitting a job to the
         * CREAM service.
         *
         * This method first looks for there is a lease ID which:
         *
         * 1) is associated with a lease which has not expired yet;
         * 2) has been created for the same user DN/CREAM URL pair
         * as the job being submitted.
         *
         * If a lease ID satisfying both conditions above is found,
         * this method returns that lease ID. If there exist no
         * lease ID satisfying the conditions, a new lease ID is created
         * and returned to the caller.
         *
         * This method periodically (every N invocations of this
         * method) checks the lease ID cache to see whether there are
         * lease IDs associated with expired leases. Such entries are
         * purged from the cache.
         *
         * This method is thread-safe: internally, it acquires a mutex
         * so that multiple access to the (shared) lease ID cache do
         * not interfere each other.
         *
         * @param job the Job for which a lease ID should be
         * requested.  The method uses the following attributes of the
         * job parameter: user DN, cream URL. All these information
         * MUST be already present in the job object.
         *
         * @param force if true, then a new lease ID is returned,
         * even if a valid lease exists in the cache. In the case
         * a valid lease ID existed in the cache, the old lease ID
         * is discarded.
         *
         * @return The lease ID to use for job submission. This method
         * returns an empty string if some error occurs. 
         */
        std::string get_lease( const CreamJob& job, bool force = false );

    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif

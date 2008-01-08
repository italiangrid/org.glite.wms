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
 * 
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#ifndef ICE_DELEGATION_MANAGER_H
#define ICE_DELEGATION_MANAGER_H

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

#include <openssl/sha.h> // for using SHA1

namespace log4cpp {
    class Category;
}

namespace glite {
namespace wms {
namespace ice {
namespace util {

    class Delegation_manager {
    protected:
        Delegation_manager( );

        /**
         * Iterates over the delegation set. Removes all entries whose
         * proxy has expired.
         */
        void purge_old_delegations( void );

        static Delegation_manager* s_instance;
        static boost::recursive_mutex m_mutex;

        log4cpp::Category* m_log_dev;
        unsigned int m_operation_count;
        const size_t m_max_size; ///< Maximum size of the delegation cache
        const unsigned int m_operation_count_max;

        /**
         * Entry of the delegation table
         */
        struct table_entry {
            std::string m_sha1_digest;
            std::string m_cream_url;
            time_t m_expiration_time;
            std::string m_delegation_id;

            table_entry( const std::string& sha1_digest, const std::string& cream_url, time_t expiration_time, const std::string& delegation_id ) :
                m_sha1_digest( sha1_digest ),
                m_cream_url( cream_url ),
                m_expiration_time( expiration_time ),
                m_delegation_id( delegation_id )
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
                boost::multi_index::member<table_entry,std::string,&table_entry::m_sha1_digest>,
            // The second composite element is the cream_url
                boost::multi_index::member<table_entry,std::string,&table_entry::m_cream_url>
              > 
            >,
            boost::multi_index::ordered_non_unique<
              boost::multi_index::member<table_entry,time_t,&table_entry::m_expiration_time>
            >,
            boost::multi_index::sequenced<>
          >
        > t_delegation_set;
        
        t_delegation_set m_delegation_set;

    public:
        ~Delegation_manager( ) { };

        /**
         * Returns the singleton instance of this class. This method
         * is thread-safe: a mutex is acquired before checking whether
         * the instance already exists or must be created.
         *
         * @return The singleton instance of this class. The caller
         * DOES NOT OWN the returned pointer, and must NOT be
         * deallocated.
         */
        static Delegation_manager* instance( );

        /**
         * Delegates a proxy for a given job. 
         *
         * This method must be called before submitting a job to the
         * CREAM service.
         *
         * This method first looks for an already existent delegation
         * ID for the (proxyfile, cream URL) pair. If no delegation ID
         * is found, a new delegation operation is performed, and the
         * new delegation ID is put into the cache for future use.
         *
         * This method periodically checks the delegation id cache to
         * see whether there are delegations with expired proxies. Such
         * entries are purged from the cache.
         *
         * This method is thread-safe: internally, it acquires a mutex
         * so that multiple access to the (shared) delegation cache do
         * not interfere each other.
         *
         * @param theProxy CreamProxy object which will be used to
         * perform the actual delegation operation on the CREAM
         * service (if such an operation needs to be executed). The
         * Authenticate() method must have been called before invoking
         * the delegate() method. Caller retains ownership of the
         * theProxy object.
         *
         * @param job the Job for which a delegation should be
         * requested.  The method uses the following attributes of the
         * job parameter: grie job id, cream URL, delegation URL and
         * proxy cert path. All these information MUST be already
         * present in the job object.
         *
         * @param force if true, then a new delegation request is done
         * regardless whether an existing delegationID for that job
         * exists in the delegation cache). If an old delegationID is
         * already associated to the proxy/cream url pair and
         * force==true, then the old delegationID is returned, and a
         * new delegation operation to CREAM is attempted, with the
         * same delegationID.
         *
         * @return The delegation ID to use for job submission. This
         * method returns an empty string if some error occurs. The
         * empty delegation string can be used anyway to request
         * auto-delegation of the proxy with the Register CREAM
         * method.
         */
        std::string delegate( /*glite::ce::cream_client_api::soap_proxy::CreamProxy* theProxy, */const CreamJob& job, bool force = false );

    };

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif

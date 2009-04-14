::/*
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
 * ICE empty CEMonitor notification class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "emptyStatusNotification.h"

// ICE stuff
#include "subscriptionManager.h"
//#include "jobCache.h"
#include "DNProxyManager.h"
#include "iceDb/GetJobByCid.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByGid.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// other gLite stuff
#include "classad_distribution.h"

// boost
#include "boost/scoped_ptr.hpp"

namespace api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;


emptyStatusNotification::emptyStatusNotification( const std::string& cream_job_id ) :
    absStatusNotification( ),
    m_cream_job_id( cream_job_id )
{
    
}

void emptyStatusNotification::apply( void )
{
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    static const char *method_name = "emptyStatusNotification::apply() - ";

    //    boost::recursive_mutex::scoped_lock L( jobCache::mutex );
    boost::recursive_mutex::scoped_lock L( CreamJob::globalICEMutex );
    //jobCache* cache( jobCache::getInstance() );
    //jobCache::iterator job_it( cache->lookupByCompleteCreamJobID( m_cream_job_id ) );

    CreamJob theJob;
    {
      db::GetJobByCid getter( m_cream_job_id );
      db::Transaction tnx;
      tnx.execute( &getter );
      if( !getter.found() ) {
	CREAM_SAFE_LOG( m_log_dev->debugStream()
                        << method_name
                        << " Cannot locate CREAM job ID "
                        << m_cream_job_id
                        << " in the database. The empty status notification "
                        << "for this job cannot be applied"
                        );
        return;
      }
      
      theJob = getter.get_job();
    }

//     if ( cache->end() == job_it ) {
//         CREAM_SAFE_LOG( m_log_dev->debugStream()
//                         << method_name
//                         << " Cannot locate CREAM job ID "
//                         << m_cream_job_id
//                         << " in the job cache. The empty status notification "
//                         << "for this job cannot be applied"
//                         );
//         return;
//     }

    //job_it->set_last_empty_notification( time(0) );
    theJob.set_last_empty_notification_time( time(0) );
    CREAM_SAFE_LOG( m_log_dev->debugStream()
                    << method_name
                    << "Timestamp of last empty "
                    << "status notification for job "
                    << theJob.describe()//job_it->describe()
		    
                    << " set to [" 
                    << time_t_to_string( theJob.get_last_empty_notification()/*job_it->get_last_empty_notification()*/ )
		    << "]"
                    );
    //cache->put( *job_it );
    {
      //      db::UpdateLastEmpty updater( theJob );
      list< pair<string, string> > params;
      params.push_back( make_pair( "last_empty_notification", int_to_string(theJob.get_last_empty_notification())));
      db::UpdateJobByGid updater( theJob.getGridJobID(), params );
      db::Transaction tnx;
      tnx.execute( &updater );
    }
}

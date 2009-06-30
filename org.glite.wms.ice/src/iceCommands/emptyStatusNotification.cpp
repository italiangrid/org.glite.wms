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
 * ICE empty CEMonitor notification class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "emptyStatusNotification.h"

// ICE stuff
#include "subscriptionManager.h"
#include "DNProxyManager.h"
#include "iceDb/Transaction.h"
#include "iceDb/UpdateJobByCid.h"

// CREAM stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/scoped_timer.h"
// other gLite stuff
#include "classad_distribution.h"

// boost
#include "boost/scoped_ptr.hpp"
namespace api_util   = glite::ce::cream_client_api::util;
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
#ifdef ICE_PROFILE_ENABLE
  api_util::scoped_timer tmp_timer( "emptyStatusNotification::apply - ENTIRE METHOD" );
#endif
    log4cpp::Category *m_log_dev( api::util::creamApiLogger::instance()->getLogger() );
    static const char *method_name = "emptyStatusNotification::apply() - ";

    boost::recursive_mutex::scoped_lock L( CreamJob::globalICEMutex );

  CREAM_SAFE_LOG( m_log_dev->debugStream()
  		  << method_name
		  << "Updating last_empty_notification field for CREAM Job ID [" 
		  << m_cream_job_id <<"]"
		  );

    {
      list< pair<string, string> > params;
      params.push_back( make_pair( "last_empty_notification", int_to_string( time(0)/*theJob.get_last_empty_notification()*/)));
      
      db::UpdateJobByCid updater( m_cream_job_id, params );
      db::Transaction tnx;
      tnx.execute( &updater );
    }
}

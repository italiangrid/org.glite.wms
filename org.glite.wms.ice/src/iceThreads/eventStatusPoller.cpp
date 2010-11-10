/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
// ICE includes
#include "ice/IceCore.h"
#include "eventStatusPoller.h"
#include "iceCommands/iceCommandStatusPoller.h"
#include "iceCommands/iceCommandEventQuery.h"

#include "iceDb/GetAllDN.h"
#include "iceDb/GetCEUrl.h"
#include "iceDb/Transaction.h"
#include "iceDb/DNHasJobs.h"

// other glite includes
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

#include <algorithm>
#include <cstdlib>

namespace cream_api     = glite::ce::cream_client_api;
namespace configuration = glite::wms::common::configuration;
using namespace glite::wms::ice::util;
using namespace std;

//boost::recursive_mutex eventStatusPoller::s_proxymutex;

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::IceCore* manager, int d )
    : iceThread( "event status poller" ),
      m_delay( d ),
      m_iceManager( manager ),
      m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
      m_threadPool( manager->get_ice_commands_pool() )
{

}

//____________________________________________________________________________
eventStatusPoller::~eventStatusPoller()
{

}

//____________________________________________________________________________
void eventStatusPoller::body( void )
{

  while( !isStopped() ) {
    
    /**
     * We don't use boost::thread::sleep because right now
     * (18/11/2005) the documentation says it will be replaced by
     * a more robust mechanism in the future.
     */
    if(m_delay<=10) 
      sleep( m_delay );
    else {
      
      for(int i=0; i<=m_delay; i++) {
	if( isStopped() ) return;
	sleep(1);
      }
      
    }
    
    // Thread wakes up
    
    CREAM_SAFE_LOG( m_log_dev->infoStream()
		    << "eventStatusPoller::body - New iteration"
		    );
    
    set< string > dns;
    {
      db::GetAllDN getter( dns, "eventStatusPoller::body" );
	
      db::Transaction tnx(false, false);
      tnx.execute( &getter ); 
    }
    
    set< string > ces;
    {
      db::GetCEUrl getter( ces, "eventStatusPoller::body" );
	
      db::Transaction tnx(false, false);
      tnx.execute( &getter ); 
    }

    set<string>::const_iterator ceit = ces.begin();
    
    for( ceit = ces.begin(); ceit != ces.end(); ++ceit ) {

      if(  ceit->empty() ) {
	CREAM_SAFE_LOG(m_log_dev->debugStream() << "eventStatusPoller::body - "
		       << "Empty CE string! Skipping..."
		       );
	continue;// next CE
      }

      set<string>::const_iterator dnit = dns.begin();
      
      for( dnit = dns.begin(); dnit != dns.end(); ++dnit ) {
	
        if( dnit->empty() ) {
          CREAM_SAFE_LOG(m_log_dev->debugStream() << "eventStatusPoller::body - "
			 << "Empty DN string! Skipping... "
			 );
	  continue; // next DN
        }
	
        {
          db::DNHasJobs hasjob( *dnit, *ceit, "eventStatusPoller::body" );
          db::Transaction tnx(false, false);
          tnx.execute( &hasjob );
          if( !hasjob.found( ) ) {
            CREAM_SAFE_LOG(m_log_dev->warnStream() << "eventStatusPoller::body - "
		           << "DN [" 
		           << *dnit << "] has not job on the CE ["
			   << *ceit << "] in the ICE's database at the moment. Skipping query..."
		           );
	    continue;
          }
        }
      
        while( m_threadPool->get_command_count( ) >= 10 ) {
	  CREAM_SAFE_LOG( m_log_dev->debugStream()
	  		  << "eventStatusPoller::body - "
			  << "Too many commands in the queue. Waiting 10 seconds..."
			  );
	  sleep( 10 );
	}
      
        CREAM_SAFE_LOG( m_log_dev->debugStream()
 			    << "eventStatusPoller::body - "
 			    << "Adding EventQuery command for couple (" 
 			    << *dnit << ", "
 			    << *ceit << ") to the thread pool..."
 			    );
      
        m_threadPool->add_request( new iceCommandEventQuery( m_iceManager, *dnit , *ceit ) );
      
      }
    }

  }
}

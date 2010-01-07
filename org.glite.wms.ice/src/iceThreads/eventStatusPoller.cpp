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
 * Event status poller
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE includes
#include "ice-core.h"
#include "eventStatusPoller.h"
#include "iceCommandStatusPoller.h"
#include "iceCommandEventQuery.h"

#include "iceDb/GetAllDN.h"
#include "iceDb/GetCEUrl.h"
#include "iceDb/Transaction.h"

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
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
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
		    << "eventStatusPoller::body() - New iteration"
		    );
    
    list< string > dns;
    {
      db::GetAllDN getter( dns, "eventStatusPoller::body" );
	
      db::Transaction tnx(false, false);
      tnx.execute( &getter ); 
    }
    
    list< string > ces;
    {
      db::GetCEUrl getter( ces, "eventStatusPoller::body" );
	
      db::Transaction tnx(false, false);
      tnx.execute( &getter ); 
    }
    


    list<string>::const_iterator ceit = ces.begin();
    
    while( ceit != ces.end() ) {
    
      list<string>::const_iterator dnit = dns.begin();
      
      while( dnit != dns.end() ) {
      
        while( m_threadPool->get_command_count( ) > 10 /*iceCondiguration::getInstance()->ice()->get_max_ice_thread( )*/ ) {
	  CREAM_SAFE_LOG( m_log_dev->debugStream()
	  		  << "eventStatusPoller::body() - "
			  << "Too many commands in the queue. Waiting 10 seconds..."
			  );
	  sleep( 10 );
	}
      
        CREAM_SAFE_LOG( m_log_dev->debugStream()
 			    << "eventStatusPoller::body() - "
 			    << "Adding EventQuery command for couple (" 
 			    << *dnit << ", "
 			    << *ceit << ") to the thread pool..."
 			    );
      
        m_threadPool->add_request( new iceCommandEventQuery( m_iceManager, *dnit , *ceit ) );//.execute();
      
//         CREAM_SAFE_LOG( m_log_dev->warnStream()
//  			    << "eventStatusPoller::body() - "
//  			    << "Finished EventQuery command for couple (" 
//  			    << *dnit << ", "
//  			    << *ceit << ")..."
//  			    );
      
        ++dnit;
      }
    
      ++ceit;
    }
    


  }
}

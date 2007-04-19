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


// other glite includes
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>

namespace cream_api=glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
eventStatusPoller::eventStatusPoller( glite::wms::ice::Ice* manager, int d )
    : iceThread( "event status poller" ),
      m_delay( d ),
      m_iceManager( manager ),
      m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
      m_pool( manager->get_ice_commands_pool() )
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
        sleep( m_delay );

        // Thread wakes up

	

        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "eventStatusPoller::body() - New iteration"
                        << log4cpp::CategoryStream::ENDLINE );

        //m_pool->add_request( new iceCommandStatusPoller( m_iceManager ) );
	iceCommandStatusPoller( m_iceManager ).execute();
    }
}

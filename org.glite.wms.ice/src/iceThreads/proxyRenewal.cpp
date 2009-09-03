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
 * ICE Proxy renewal thread
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

/**
 *
 * ICE Headers
 *
 */
#include "ice-core.h"
#include "iceUtils.h"
#include "proxyRenewal.h"
#include "iceConfManager.h"
#include "iceCommandDelegationRenewal.h"


/**
 *
 * CE and WMS Headers
 *
 */
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

// boost includes
#include <boost/thread/thread.hpp>

// std includes
#include <vector>

/**
 *
 * System OS Headers
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
proxyRenewal::proxyRenewal() :
    iceThread( "ICE Proxy Renewer" ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
  m_delay = iceConfManager::getInstance()->getConfiguration()->ice()->proxy_renewal_frequency();
}


//______________________________________________________________________________
void proxyRenewal::body( void )
{
    while( !isStopped() ) {        
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "proxyRenewal::body() - new iteration"
                       );
        
	iceCommandDelegationRenewal().execute();

	if(m_delay<=10) 
	  sleep( m_delay );
	else {
	  
	  for(int i=0; i<=m_delay; i++) {
	    if( isStopped() ) return;
	    sleep(1);
	  }
	  
	}
	
        //sleep( m_delay );
    }
}


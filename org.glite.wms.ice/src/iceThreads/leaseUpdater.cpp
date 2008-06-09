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

#include "leaseUpdater.h"
#include "iceConfManager.h"
#include "iceCommandLeaseUpdater.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

using namespace glite::wms::ice::util;
using namespace std;

//____________________________________________________________________________
leaseUpdater::leaseUpdater( ) :
    iceThread( "ICE Lease Updater" ),
    m_delay( iceConfManager::getInstance()->getConfiguration()->ice()->lease_update_frequency() ),
    m_log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() )
{
   
}

//____________________________________________________________________________
leaseUpdater::~leaseUpdater( )
{

}

//______________________________________________________________________________
void leaseUpdater::body( void )
{
    while ( !isStopped() ) {
        CREAM_SAFE_LOG(m_log_dev->infoStream()
                       << "leaseUpdater::body() - new iteration"
                       );
	iceCommandLeaseUpdater().execute();

	if(m_delay<=10) 
	  sleep( m_delay );
	else {
	  
	  for(int i=0; i<=m_delay; i++) {
	    if( isStopped() ) return;
	    sleep(1);
	  }
	  
	}


	//        sleep( m_delay );
    }
}

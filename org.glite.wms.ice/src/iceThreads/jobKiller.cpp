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
 * ICE job killer
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// ICE stuff
#include "jobKiller.h"
#include "iceCommandJobKill.h"

// GLITE stuff
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

using namespace std;
using namespace glite::wms::ice::util;

//____________________________________________________________________________
jobKiller::jobKiller() : 
    iceThread( "Job Killer" ),
    m_valid( true ),
    m_threshold_time( iceConfManager::getInstance()->getConfiguration()->ice()->job_cancellation_threshold_time())
{
    if ( m_threshold_time < 60 ) 
        m_threshold_time = 60;
    m_delay = m_threshold_time/2;
}

//____________________________________________________________________________
jobKiller::~jobKiller()
{

}

//____________________________________________________________________________
void jobKiller::body()
{
    log4cpp::Category* log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() );
    static const char* method_name = "jobKiller::body() - ";
    
    while ( !isStopped() ) {

      if(m_delay<=10) 
        sleep( m_delay );
      else {
	
        for(int i=0; i<=m_delay; i++) {
          if( isStopped() ) return;
          sleep(1);
        }
	
      }
      
      
      //sleep( m_delay );
        CREAM_SAFE_LOG( log_dev->infoStream() << method_name
                        << "New iteration"
                         );
        iceCommandJobKill( ).execute();
    }
}

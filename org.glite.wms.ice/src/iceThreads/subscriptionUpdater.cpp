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
 * ICE Subscription Updater for CEMON
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "subscriptionUpdater.h"
#include "iceCommandSubUpdater.h"
#include "iceUtils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

namespace iceUtil = glite::wms::ice::util;
namespace api = glite::ce::cream_client_api;
using namespace std;

//______________________________________________________________________________
iceUtil::subscriptionUpdater::subscriptionUpdater( ) :
  iceThread( "subscription Updater" )
{
  m_iteration_delay = (int)(iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->subscription_update_threshold_time()/4);

  if(m_iteration_delay < 60) m_iteration_delay=60;

}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{

  while( !isStopped() ) {
  
    iceCommandSubUpdater cmd;
    cmd.execute();
    
    if(m_iteration_delay<=10) 
      sleep( m_iteration_delay );
    else {
      
      for(int i=0; i<=m_iteration_delay; i++) {
	if( isStopped() ) return;
	sleep(1);
      }
      
    }

    //sleep( m_iteration_delay );
  }
}

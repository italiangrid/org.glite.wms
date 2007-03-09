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

//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "subscriptionUpdater.h"
#include "iceCommandSubUpdater.h"
//#include "jobCache.h"
//#include "cemonUrlCache.h"
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
  m_iteration_delay = (int)(iceUtil::iceConfManager::getInstance()->getConfiguration()->ice()->subscription_update_threshold_time()/2);
  if(!m_iteration_delay) m_iteration_delay=5;

}

//______________________________________________________________________________
void iceUtil::subscriptionUpdater::body( void )
{

  while( !isStopped() ) {
  
    iceCommandSubUpdater cmd;
    cmd.execute();

    sleep( m_iteration_delay );
  } // while( !stopped )
}   // end function

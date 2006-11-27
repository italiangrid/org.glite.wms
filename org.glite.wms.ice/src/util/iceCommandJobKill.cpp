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

#include "iceCommandJobKill.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "iceConfManager.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"
#include "jobCache.h"
#include "creamJob.h"
#include "CreamProxyMethod.h"

#include <string>
#include <vector>

#include <boost/format.hpp>

using namespace std;

namespace api_util  = glite::ce::cream_client_api::util;
namespace cream_api = glite::ce::cream_client_api;
namespace ice_util  = glite::wms::ice::util;

namespace glite {
namespace wms {
namespace ice {
namespace util {

//______________________________________________________________________________
iceCommandJobKill::iceCommandJobKill( glite::ce::cream_client_api::soap_proxy::CreamProxy* _theProxy,
				      ice_util::CreamJob* theJob ) throw() : 
				      m_theJob( theJob ),
				      m_log_dev( api_util::creamApiLogger::instance()->getLogger()),
				      m_threshold_time( ice_util::iceConfManager::getInstance()->getJobKillThresholdTime() ),
				      m_lb_logger( ice_util::iceLBLogger::instance() )
{
  
  m_theProxy.reset( _theProxy );
  
  if( m_threshold_time < 60 ) m_threshold_time = 60;
  
  //m_delay = m_threshold_time/2;
}

//______________________________________________________________________________
void iceCommandJobKill::execute( ) throw()
{
  boost::recursive_mutex::scoped_lock M( ice_util::jobCache::mutex );
  time_t proxyTimeLeft = cream_api::certUtil::getProxyTimeLeft( m_theJob->getUserProxyCertificate() );
  if( proxyTimeLeft < m_threshold_time && proxyTimeLeft > 5 ) {
    CREAM_SAFE_LOG( m_log_dev->infoStream() 
                    << "iceCommandJobKill::execute() - Job ["
                    << m_theJob->getCreamJobID() << "]"
                    << " has proxy expiring in "
                    << proxyTimeLeft 
                    << " seconds, which is less than "
                    << "the threshold ("
                    << m_threshold_time << " seconds). "
                    << "Going to cancel it..."
                    << log4cpp::CategoryStream::ENDLINE);
		    
    killJob( *m_theJob, proxyTimeLeft );
  }
}

//______________________________________________________________________________
void iceCommandJobKill::killJob( ice_util::CreamJob& J, const time_t residual_proxy_time )
{
  try {
      m_theProxy->Authenticate( J.getUserProxyCertificate() );
      vector<string> url_jid(1);   
      url_jid[0] = J.getCreamJobID();
   
      J = m_lb_logger->logEvent( new ice_util::cream_cancel_request_event( J, boost::str( boost::format( "Killed by ice's iceCommandJobKill, as residual proxy time=%1%, which is less than the threshold=%2%" ) % residual_proxy_time % m_threshold_time ) ) );

      J.set_killed_by_ice();
      J.set_failure_reason( "The job has been killed because its proxy was expiring" );

      
      // m_theProxy->Cancel( J.getCreamURL().c_str(), url_jid );
      ice_util::CreamProxy_Cancel( J.getCreamURL(), url_jid ).execute( m_theProxy.get(), 3 );

      // The corresponding "cancel done event" will be notified by the
      // poller/listener, so it is not logged here

      // The poller takes care of purging jobs
      // theProxy->Purge( J.getCreamURL().c_str(), url_jid) ;
      CREAM_SAFE_LOG(
                     m_log_dev->infoStream() << "iceCommandJobKill::killJob() - "
                     << " Cancellation SUCCESFUL for job ["
                     <<  J.getCreamJobID() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
  } catch(std::exception& ex) {
      J = m_lb_logger->logEvent( new ice_util::cream_cancel_refuse_event( J, ex.what() ) );
      // The job will not be removed from the job cache. We keep
      // trying to cancel it until the residual proxy time is less
      // than a minimum threshold. After that, the statusPoller will
      // eventually take care of removing it from the cache.
      CREAM_SAFE_LOG (
                      m_log_dev->errorStream() 
                      << "iceCommandJobKill::killJob() - Error"
                      << " killing job [" << J.getCreamJobID() << "]: "
                      << ex.what()
                      << log4cpp::CategoryStream::ENDLINE);
  }

  // The cache is already locked at the beginning of the execute method
  ice_util::jobCache::getInstance()->put( J );
}



} // util
} // ice
} // ice
} // wms

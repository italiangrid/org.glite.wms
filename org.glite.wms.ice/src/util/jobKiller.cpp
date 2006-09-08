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
#include "jobCache.h"
#include "iceLBLogger.h"
#include "iceLBEvent.h"

// GLITE stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

// STL and Boost stuff
#include <vector>
#include <boost/format.hpp>

using namespace std;
namespace cream_api = glite::ce::cream_client_api;
using namespace glite::wms::ice::util;

//____________________________________________________________________________
jobKiller::jobKiller() : 
    iceThread( "Job Killer" ),
    m_valid( true ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_theProxy( new cream_api::soap_proxy::CreamProxy( false ) ),
    m_threshold_time( iceConfManager::getInstance()->getJobKillThresholdTime()),
    m_lb_logger( iceLBLogger::instance() )    
{
    if( m_threshold_time < 60 ) m_threshold_time = 60;
    m_delay = m_threshold_time/2;
}

//____________________________________________________________________________
jobKiller::~jobKiller()
{

}

//____________________________________________________________________________
void jobKiller::body()
{
    jobCache::iterator job_it;
    while( !isStopped() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "jobKiller::body() - New iteration..."
                        << log4cpp::CategoryStream::ENDLINE);
        { 
            // FIXME: perhaps this locking can be less rough...
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            for( job_it = jobCache::getInstance()->begin(); 
                 job_it != jobCache::getInstance()->end();
                 ++job_it) {
                time_t proxyTimeLeft = m_theProxy->getProxyTimeLeft( job_it->getUserProxyCertificate() );
                if( proxyTimeLeft < m_threshold_time ) {
                    CREAM_SAFE_LOG( m_log_dev->infoStream() 
                                    << "jobKiller::body() - Job ["
                                    << job_it->getJobID() << "]"
                                    << " has proxy expiring in "
                                    << proxyTimeLeft 
                                    << " seconds, which is less than "
                                    << "the threshold ("
                                    << m_threshold_time << " seconds). "
                                    << "Going to cancel it..."
                                    << log4cpp::CategoryStream::ENDLINE);
                    killJob( *job_it, proxyTimeLeft );
                }
            }
        }
        sleep( m_delay );
    }
}

//______________________________________________________________________________
void jobKiller::killJob( CreamJob& J, time_t residual_proxy_time )
{
  try {
      m_theProxy->Authenticate( J.getUserProxyCertificate() );
      vector<string> url_jid(1);   
      url_jid[0] = J.getJobID();
   
      J = m_lb_logger->logEvent( new cream_cancel_request_event( J, boost::str( boost::format( "Killed by cream::jobKiller, as residual proxy time=%1%, which is less than the threshold=%2%" ) % residual_proxy_time % m_threshold_time ) ) );

      J.set_killed_by_ice();
      J.set_failure_reason( "The job has been killed because its proxy was expiring" );

      m_theProxy->Cancel( J.getCreamURL().c_str(), url_jid );

      // The corresponding "cancel done event" will be notified by the
      // poller/listener, so it is not logged here

      // The poller takes care of purging jobs
      // theProxy->Purge( J.getCreamURL().c_str(), url_jid) ;
      CREAM_SAFE_LOG(
                     m_log_dev->infoStream() << "jobKiller::killJob() - "
                     << " Cancellation SUCCESFUL for job ["
                     <<  J.getJobID() << "]"
                     << log4cpp::CategoryStream::ENDLINE);
  } catch(std::exception& ex) {
      m_lb_logger->logEvent( new cream_cancel_refuse_event( J, ex.what() ) );
      CREAM_SAFE_LOG (
                      m_log_dev->errorStream() 
                      << "jobKiller::killJob() - Error"
                      << " killing job [" << J.getJobID() << "]: "
                      << ex.what()
                      << log4cpp::CategoryStream::ENDLINE);
  }

}

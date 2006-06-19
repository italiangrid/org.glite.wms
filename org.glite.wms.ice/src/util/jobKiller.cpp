
#include "jobKiller.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"

#include <vector>

using namespace std;

namespace iceUtil = glite::wms::ice::util;
namespace cream_api = glite::ce::cream_client_api;

//______________________________________________________________________________
iceUtil::jobKiller::jobKiller()
  : iceThread( "Job Killer" ),
    m_valid( true ),
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() ),
    m_threshold_time( iceUtil::iceConfManager::getInstance()->getJobKillThresholdTime())
    
{
  m_theProxy = cream_api::soap_proxy::CreamProxyFactory::getProxy();
  if( m_threshold_time < 60 ) m_threshold_time = 60;
  m_delay = m_threshold_time/2;
}

//______________________________________________________________________________
iceUtil::jobKiller::~jobKiller()
{

}

//______________________________________________________________________________
void iceUtil::jobKiller::body()
{
    //vector< iceUtil::creamJob > jobs;
    iceUtil::jobCache::iterator job_it;
    while( !isStopped() ) {
        CREAM_SAFE_LOG( m_log_dev->infoStream()
                        << "jobKiller::body() - New iteration..."
                        << log4cpp::CategoryStream::ENDLINE);
        { // FIXME: perhaps this locking can be less rough...
            boost::recursive_mutex::scoped_lock M( jobCache::mutex );
            for( job_it = iceUtil::jobCache::getInstance()->begin(); 
                 job_it != iceUtil::jobCache::getInstance()->end();
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
                    killJob( *job_it );
                }
            }
        }
        sleep( m_delay );
    }
}

//______________________________________________________________________________
void iceUtil::jobKiller::killJob( const iceUtil::CreamJob& J )
{
  try {
    m_theProxy->Authenticate( J.getUserProxyCertificate() );
    vector<string> url_jid(1);   
    url_jid[0] = J.getJobID();
    m_theProxy->Cancel( J.getCreamURL().c_str(), url_jid );
    // The poller takes care of purging jobs
    // theProxy->Purge( J.getCreamURL().c_str(), url_jid) ;
    CREAM_SAFE_LOG(
		   m_log_dev->infoStream() << "jobKiller::killJob() - "
		   << " Cancellation SUCCESFUL for job ["
		   <<  J.getJobID() << "]"
		   << log4cpp::CategoryStream::ENDLINE);
  } catch(std::exception& ex) {
    CREAM_SAFE_LOG (
		    m_log_dev->errorStream() << "jobKiller::killJob() - Error"
		    << " killing job [" << J.getJobID() << "]: "
		    << ex.what()
		    << log4cpp::CategoryStream::ENDLINE);
  }

// catch(cream_api::soap_proxy::auth_ex& ex) {
//     //m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("auth_ex: ") + ex.what() ) );
//     //throw iceCommandFatal_ex( string("auth_ex: ") + ex.what() );
//   } catch(cream_api::soap_proxy::soap_ex& ex) {
//     //m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("soap_ex: ") + ex.what() ) );
//     //throw iceCommandTransient_ex( string("soap_ex: ") + ex.what() );
//     // HERE MUST RESUBMIT
//   } catch(cream_api::cream_exceptions::BaseException& base) {
//     //m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("BaseException: ") + base.what() ) );
//     //throw iceCommandFatal_ex( string("BaseException: ") + base.what() );
//   } catch(cream_api::cream_exceptions::InternalException& intern) {
//     //m_lb_logger->logEvent( new util::cream_cancel_refuse_event( theJob, string("InternalException: ") + intern.what() ) );
//     //throw iceCommandFatal_ex( string("InternalException: ") + intern.what() );
//   }
}

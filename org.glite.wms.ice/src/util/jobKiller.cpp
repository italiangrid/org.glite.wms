
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
    m_log_dev( cream_api::util::creamApiLogger::instance()->getLogger() )
{
  theProxy = cream_api::soap_proxy::CreamProxyFactory::getProxy();
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
    { // FIXME: perhaps this locking can be less rough...
      boost::recursive_mutex::scoped_lock M( jobCache::mutex );
      for(job_it = iceUtil::jobCache::getInstance()->begin(); 
	  job_it != iceUtil::jobCache::getInstance()->end();
	  ++job_it)
	{
	  if((job_it->getEndLease() - time(NULL))<300)
	    {
	      CREAM_SAFE_LOG( m_log_dev->infoStream() << "jobKiller::body() - Job ["
			      << job_it->getJobID() << "]"
			      << " has proxy expiring in 5 minutes. Going to cancel and purge it..."
			      << log4cpp::CategoryStream::ENDLINE);
	      killJob( *job_it );
	    }
	}
    }
    sleep(120);
  }
}

//______________________________________________________________________________
void iceUtil::jobKiller::killJob( const iceUtil::CreamJob& J )
{
  try {
    theProxy->Authenticate( J.getUserProxyCertificate() );
    vector<string> url_jid(1);   
    url_jid[0] = J.getJobID();
    theProxy->Cancel( J.getCreamURL().c_str(), url_jid );
    theProxy->Purge( J.getCreamURL().c_str(), url_jid) ;
  } catch(std::exception& ex) {
    CREAM_SAFE_LOG (
		    m_log_dev->errorStream() << "jobKiller::killJob() - " 
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

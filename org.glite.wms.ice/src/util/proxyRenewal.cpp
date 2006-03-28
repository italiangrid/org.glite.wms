#include "proxyRenewal.h"
#include "jobCache.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/BaseException.h"
#include "glite/ce/cream-client-api-c/InternalException.h"
#include "glite/ce/cream-client-api-c/DelegationException.h"

// boost includes
#include <boost/thread/thread.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace iceUtil = glite::wms::ice::util;

//______________________________________________________________________________
iceUtil::proxyRenewal::proxyRenewal() :
    iceThread( "ICE Lease Updater" ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    creamClient( 0 ),
    delay( 1*60 ) // proxy renewer wakes up every minute
{
}


//______________________________________________________________________________
void iceUtil::proxyRenewal::body( void )
{
    while( !isStopped() ) {
	checkProxies();
        sleep( delay );
    }
}

//______________________________________________________________________________
void iceUtil::proxyRenewal::checkProxies()
{
  boost::recursive_mutex::scoped_lock M( jobCache::mutex );
  jobCache* cache = jobCache::getInstance();

  for(jobCache::iterator jobIt = cache->begin(); jobIt != cache->end(); ++jobIt) {
    struct stat buf;
    if( ::stat( jobIt->getUserProxyCertificate().c_str(), &buf) == 1 )
    {
	// WARNING CANNOT CHECK PROXY TIME // FIXME
	log_dev->warnStream() << "proxyRenewal::checkProxies() - Cannot stat proxy file ["
			      << jobIt->getUserProxyCertificate() << "] for job ["
			      << jobIt->getJobID() << "]. Wont check if it needs to be renewed."
			      << log4cpp::CategoryStream::ENDLINE;

    } else {
      if( buf.st_mtime > jobIt->getProxyCertLastMTime() ) {
	// INFO: RENEW PROXY
	log_dev->infoStream() << "proxyRenewal::checkProxies() - Need to renew proxy  ["
                              << jobIt->getUserProxyCertificate() << "] for job ["
                              << jobIt->getJobID() << "]"
			      << log4cpp::CategoryStream::ENDLINE;
	// update of lastmodification time of proxy file
	// put in cache
      }
    }
  }
}

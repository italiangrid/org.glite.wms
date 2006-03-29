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

// std includes
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace glite::ce::cream_client_api;
using namespace glite::wms::ice::util;
using namespace std;

//______________________________________________________________________________
proxyRenewal::proxyRenewal() :
    iceThread( "ICE Proxy Renewer" ),
    log_dev( glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger() ),
    cache( jobCache::getInstance() ),
    creamClient( 0 ),
    delay( 1*60 ) // proxy renewer wakes up every minute
{
    try {        
        soap_proxy::CreamProxy* p( new soap_proxy::CreamProxy( false ) );
        creamClient.reset( p ); // boost::scoped_ptr<>.reset() requires its argument not to throw anything, IIC
    } catch(soap_proxy::soap_ex& ex) {
        // FIXME: what to do??
    } 
}


//______________________________________________________________________________
void proxyRenewal::body( void )
{
    while( !isStopped() ) {

        log_dev->infoStream()
            << "proxyRenewal::body() - new iteration"
            << log4cpp::CategoryStream::ENDLINE;

	checkProxies();
        sleep( delay );
    }
}

//______________________________________________________________________________
void proxyRenewal::checkProxies()
{
  boost::recursive_mutex::scoped_lock M( jobCache::mutex );

  for(jobCache::iterator jobIt = cache->begin(); jobIt != cache->end(); ++jobIt) {
      if ( ! jobIt->is_active() ) 
          continue; // skip terminated jobs

    struct stat buf;
    if( ::stat( jobIt->getUserProxyCertificate().c_str(), &buf) == 1 )
    {
	log_dev->errorStream() 
            << "proxyRenewal::checkProxies() - Cannot stat proxy file ["
            << jobIt->getUserProxyCertificate() << "] for job ["
            << jobIt->getJobID() << "]. Wont check if it needs to be renewed."
            << log4cpp::CategoryStream::ENDLINE;
        // FIXME: what to do?
    } else {
      if( buf.st_mtime > jobIt->getProxyCertLastMTime() ) {
	log_dev->infoStream() 
            << "proxyRenewal::checkProxies() - Need to renew proxy  ["
            << jobIt->getUserProxyCertificate() << "] for job ["
            << jobIt->getJobID() << "]"
            << log4cpp::CategoryStream::ENDLINE;

        creamClient->clearSoap( );

        try {
            creamClient->Authenticate( jobIt->getUserProxyCertificate() );

            vector< string > theJob;
            theJob.push_back( jobIt->getJobID() );

            creamClient->renewProxy( jobIt->getDelegationId(),
                                     jobIt->getCreamURL(),
                                     jobIt->getCreamDelegURL(),
                                     jobIt->getUserProxyCertificate(),
                                     theJob );
        } catch( soap_proxy::soap_ex& ex ) {
            // FIXME: what to do?
        }

        jobIt->setProxyCertMTime( buf.st_mtime );
        cache->put( *jobIt );

	// update of lastmodification time of proxy file
	// put in cache
      }
    }
  }
}

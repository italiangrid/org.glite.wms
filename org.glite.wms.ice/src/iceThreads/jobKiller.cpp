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
#include "CreamProxyFactory.h"
#include "CreamProxyMethod.h"
#include "iceCommandJobKill.h"

// GLITE stuff
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

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
    m_theProxy( CreamProxyFactory::makeCreamProxy( false ) ),
    m_threshold_time( iceConfManager::getInstance()->getConfiguration()->ice()->job_cancellation_threshold_time()),
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
                time_t proxyTimeLeft = cream_api::certUtil::getProxyTimeLeft( job_it->getUserProxyCertificate() );
                if( proxyTimeLeft < m_threshold_time && proxyTimeLeft > 5 ) {
                    CREAM_SAFE_LOG( m_log_dev->debugStream() 
                                    << "jobKiller::body() - Job "
                                    << job_it->describe()
                                    << " has proxy expiring in "
                                    << proxyTimeLeft 
                                    << " seconds, which is less than "
                                    << "the threshold ("
                                    << m_threshold_time << " seconds). "
                                    << "Going to cancel it..."
                                    << log4cpp::CategoryStream::ENDLINE); 
		    
		    iceCommandJobKill( *job_it ).execute();
                } else {
                    CREAM_SAFE_LOG( m_log_dev->infoStream() 
                                    << "jobKiller::body() - Job ["
                                    << job_it->getCreamJobID() << "]"
                                    << " has proxy expiring in "
                                    << proxyTimeLeft 
                                    << " seconds, which is already/about "
                                    << "to expire, or more than "
                                    << "the threshold ("
                                    << m_threshold_time << " seconds). "
                                    << "Going to cancel it..."
                                    << log4cpp::CategoryStream::ENDLINE); 

                }
            } //loop over all jobCache's jobs
        } // unlock of the cache
	sleep( m_delay );
    } // while(!stopped)
}

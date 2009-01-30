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
 * ICE job cache Iterator
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "jobCacheIterator.h"
#include "jobCache.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include <boost/archive/text_iarchive.hpp>

namespace api_util = glite::ce::cream_client_api::util;
using namespace glite::wms::ice::util;
using namespace std;

boost::recursive_mutex jobCacheIterator::mutex;

//____________________________________________________________________
jobCacheIterator::jobCacheIterator() throw() :
    m_valid_it( false ),
    m_grid_job_id(),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger() )
{

}

//____________________________________________________________________
jobCacheIterator::jobCacheIterator( const std::string& an_id ) throw() :
    m_valid_it( false ), 
    m_grid_job_id( an_id ),
    m_log_dev( api_util::creamApiLogger::instance()->getLogger() )
{ 

}

//____________________________________________________________________
bool jobCacheIterator::operator==( const jobCacheIterator& anIt ) 
  const throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
    return ( m_grid_job_id == anIt.m_grid_job_id );
}

//____________________________________________________________________
bool jobCacheIterator::operator!=( const jobCacheIterator& anIt ) 
  const throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
  return !( m_grid_job_id == anIt.m_grid_job_id );
}

//____________________________________________________________________
jobCacheIterator&
jobCacheIterator::operator=( const jobCacheIterator& anIt ) 
  throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
    if ( this != &anIt ) {
        m_grid_job_id = anIt.m_grid_job_id;
        m_valid_it = false;
    }
    return *this;
}

//___________________________________________________________________
CreamJob*
jobCacheIterator::operator->() throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
   static const char* method_name = "jobCacheIterator::operator->() - ";

    if( m_grid_job_id.empty() ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                       << "Trying to dereference end() iterator. Aborting."
                        );        
        abort();
    }
    
    refresh();
    
    return &m_theJob;
}

//____________________________________________________________________
CreamJob
jobCacheIterator::operator*() throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
    static const char* method_name = "jobCacheIterator::operator*() - ";

    if( m_grid_job_id.empty() ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                       << "Trying to dereference end() iterator. Aborting."
                        );
        abort();
    }

    refresh();

    return m_theJob;
}

//____________________________________________________________________
void jobCacheIterator::refresh( ) throw()
{
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
    static const char* method_name = "jobCacheIterator::refresh() - ";

    if ( m_valid_it )
        return;

    m_theJob = CreamJob();
    jobCache* cache( jobCache::getInstance() );
    istringstream is;
    try {
        
	string tmpGid = cache->getDbManager()->getByGid( m_grid_job_id );
	if( tmpGid.empty() ) {
	  m_valid_it = false;
	  return;
	}
        is.str( /*cache->getDbManager()->getByGid( m_grid_job_id )*/ tmpGid );
// 	{
//           boost::archive::text_iarchive ia(is);
//           ia >> m_theJob;
// 	}
//         m_valid_it = true;
    } catch( JobDbException& ex ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                       << "Got JobDbException while loading job from cache. "
                       << "Reason is \"" << ex.what() << "\". Aborting."
                        );
        abort();
    } catch( std::exception& ex ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                       << "Got std::exception while loading job from cache. "
                       << "Reason is \"" << ex.what() << "\". Aborting."
                        );
        abort();            
    } catch( ... ) {
        CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                       << "Got unknown exception while loading job from cache. "
                       << "Aborting."
                        );
        abort();            
    }

    bool retry = true;
    int sertry = 0;
    while( retry ) {
      try
	{
	  boost::archive::text_iarchive ia(is);
	  ia >> m_theJob;
	  retry = false;
	}
      catch(SerializeException& ex) {
	sertry++;
	if(sertry>=3) {
	  // FIXME: put a FATAL log message here
	  exit(2);
	}
      }
      catch(boost::archive::archive_exception& ex) {
          CREAM_SAFE_LOG(m_log_dev->fatalStream() << method_name
                         << "Got archive_exception "<< ex.what()
                         << " on try " << sertry << "/3"
                         );
	sertry++;
	if(sertry>=3) {
	  // FIXME: put a FATAL log message here
	  exit(2);
	}
      }

      m_valid_it = true;

    } // while( retry )
}

//____________________________________________________________________
jobCacheIterator& jobCacheIterator::operator++() throw()          
{ 
  boost::recursive_mutex::scoped_lock L( jobCacheIterator::mutex );
    jobCache* cache( jobCache::getInstance() );
    m_valid_it = false;
    m_grid_job_id = cache->get_next_grid_job_id( m_grid_job_id );
    return *this;
}

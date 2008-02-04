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
 * ICE job cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

// PROJECT INCLUDES
#include "jobCache.h"
#include "iceConfManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// System INCLUDES
#include <iostream>
#include <sstream>
#include <exception>
#include <cstdio> // for ::rename(...)
#include <string>
#include <sstream>
#include <utility> // for make_pair
#include <ctime>
#include <cstdlib>
#include <sstream>

extern int errno;

using namespace std;

using namespace glite::wms::ice::util;
namespace apiutil = glite::ce::cream_client_api::util;

jobCache* jobCache::s_instance = 0;
string jobCache::s_persist_dir( DEFAULT_PERSIST_DIR );
bool   jobCache::s_recoverable_db = false;
bool   jobCache::s_read_only = false;
boost::recursive_mutex jobCache::mutex;

//____________________________________________________________________________
jobCache* jobCache::getInstance() throw()
{
    if ( !s_instance )
        s_instance = new jobCache( );
    return s_instance;
}

//____________________________________________________________________________
jobCache::jobCache( void ) :
    m_log_dev(apiutil::creamApiLogger::instance()->getLogger()),
    m_GridJobIDSet( )
{ 
  jobDbManager *dbm = new jobDbManager( s_persist_dir, s_recoverable_db, false, s_read_only );

    if ( !dbm->isValid() ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() 
                        << "jobCache::jobCache() - "
                        << "Failed to initialize the jobDbManager object. "
                        << "Reason is: " 
                        << dbm->getInvalidCause() 
                        << log4cpp::CategoryStream::ENDLINE );
        abort();
    }
    m_dbMgr.reset( dbm );
    
    load();
	 
    jobCacheIterator::s_cache = this;
}

//______________________________________________________________________________
jobCache::~jobCache( ) 
{
  
}

//______________________________________________________________________________
void jobCache::load( void ) throw()
{
  
  /**
   * new implementation using the boost serializer
   *
   */
  vector<string> records;
  try {
    m_dbMgr->initCursor();

    char *data;
    while( (data = (char*)m_dbMgr->getNextData()) != NULL ) 
    {
      boost::scoped_ptr< char > toFree;
      toFree.reset( data );
      
      CreamJob cj;
      istringstream tmpOs;//( string(data) );
      tmpOs.str( data );
      try {
        boost::archive::text_iarchive ia(tmpOs);
	ia >> cj;

        m_GridJobIDSet.insert( cj.getGridJobID() );
	
      } catch(exception& ex ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobCache::load() - boost::archive::text_iarchive raised an exception: "
                        << ex.what()
                        << log4cpp::CategoryStream::ENDLINE );
        abort();
      } catch(...) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobCache::load() - Unknown exception catched"
                        << log4cpp::CategoryStream::ENDLINE );
        abort();
      }
    }

    m_dbMgr->endCursor();

  } catch(JobDbException& dbex) {
    CREAM_SAFE_LOG( m_log_dev->fatalStream() 
		    << "jobCache::load() - "
		    << "Failed to get a record from the database. "
		    << "Reason is: " << dbex.what() << ". Giving up."
		    << log4cpp::CategoryStream::ENDLINE );
    abort();
  }
}

//______________________________________________________________________________
jobCache::iterator jobCache::put(const CreamJob& cj)
{   
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache? 
    try {
      ostringstream ofs;
      boost::archive::text_oarchive oa(ofs);
      oa << cj;
      m_dbMgr->put( ofs.str(), cj.getCompleteCreamJobID(), cj.getGridJobID() );
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() 
			<< dbex.what() 
			<< log4cpp::CategoryStream::ENDLINE );
        abort();
    }
    return jobCacheIterator( (m_GridJobIDSet.insert( cj.getGridJobID() )).first );
}


//______________________________________________________________________________
jobCache::iterator 
jobCache::lookupByCompleteCreamJobID( const string& completeCreamJID )
  throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    try {
      string serializedJob( m_dbMgr->getByCid( completeCreamJID ) );
      CreamJob cj;
      istringstream tmpOs;//( string(data) );
      tmpOs.str( serializedJob );
      boost::archive::text_iarchive ia(tmpOs);
      ia >> cj;
    
      return jobCacheIterator( m_GridJobIDSet.find( cj.getGridJobID() ) );
    } catch(JobDbNotFoundException& ex) {
      CREAM_SAFE_LOG( m_log_dev->errorStream() 
                      << "jobCache::lookupByCompleteCreamJobID() - " 
		      << ex.what() 
		      << log4cpp::CategoryStream::ENDLINE );
      return this->end();
      
    } catch(exception& ex) {
      CREAM_SAFE_LOG( m_log_dev->fatalStream() 
		      << ex.what() << log4cpp::CategoryStream::ENDLINE );
      abort();
    }
}

//______________________________________________________________________________
jobCache::iterator jobCache::lookupByGridJobID( const string& gridJID )
  throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    //return m_jobs.findJobByGID( gridJID );
    
    return jobCacheIterator( m_GridJobIDSet.find( gridJID ) );
}

//______________________________________________________________________________
jobCache::iterator jobCache::erase( jobCache::iterator it )
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?

    if( it == this->end() ) return it;

    jobCache::iterator result = it;
    ++result; // advance iterator
    // job found, log operation and remove
    
    CREAM_SAFE_LOG( m_log_dev->debugStream() 
                      << "jobCache::erase() - Removing CreamJobID ["
		      << it->getCompleteCreamJobID() << "] from BerkeleyDB..."
		      << log4cpp::CategoryStream::ENDLINE );
    
    try{
      m_dbMgr->delByCid( it->getCompleteCreamJobID() );
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() 
			<< dbex.what() 
			<< log4cpp::CategoryStream::ENDLINE );
        abort();
    }

    m_GridJobIDSet.erase( it.m_it );
    return result;
}

//______________________________________________________________________________
jobCache::iterator jobCache::begin() 
{
  return jobCacheIterator( m_GridJobIDSet.begin() );
}


//______________________________________________________________________________
jobCache::iterator jobCache::end() 
{
  return jobCacheIterator( m_GridJobIDSet.end() );
}


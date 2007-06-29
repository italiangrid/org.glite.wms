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

//#include "classad_distribution.h" // classad's stuff
//#include "source.h" // classad's stuff
//#include "sink.h"   // classad's stuff

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
boost::recursive_mutex jobCache::mutex;

// 
// Inner class definitions
//

//______________________________________________________________________________
jobCache::jobCacheTable::jobCacheTable( ) :
    m_jobs( ),
    m_cidMap( ),
    m_gidMap( )
{

}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator jobCache::jobCacheTable::putJob( const CreamJob& c )
{
    // Note: the GridJobID of a job MUST always be defined; the creamJobId
    // could be initially empty, so we need to check this.

    t_gidMapType::iterator it = m_gidMap.find( c.getGridJobID() );
    jobCacheTable::iterator pos;
    if ( it == m_gidMap.end() ) {
        // Inserts a new job
        m_jobs.push_back( c );
        pos = --m_jobs.end();
    } else {
        pos = it->second;
        // overwrites existing job
        *pos = c;
    }
    m_gidMap.insert( make_pair( c.getGridJobID(), pos ) );
    if ( !c.getCreamJobID().empty() ) {
        m_cidMap.insert( make_pair( c.getCreamJobID(), pos ) );
    }
    return pos;
}

//______________________________________________________________________________
void jobCache::jobCacheTable::delJob( const jobCacheTable::iterator& pos )
{
    if ( pos == end() )
        return;

    // Removes from the gidMap
    m_gidMap.erase( pos->getGridJobID() );

    // Removes from the cidMap
    if ( !pos->getCreamJobID().empty() ) {
        m_cidMap.erase( pos->getCreamJobID() );
    }

    // Finally, removes from the joblist
    m_jobs.erase( pos );
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByGID( const std::string& gid )
{
    t_gidMapType::iterator it = m_gidMap.find( gid );

    if ( it != m_gidMap.end() ) {
        return it->second;
    } else {
        return m_jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::findJobByCID( const std::string& cid )
{
    t_cidMapType::iterator it = m_cidMap.find( cid );

    if ( it != m_cidMap.end() ) {
        return it->second;
    } else {
        return m_jobs.end();
    }
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::begin( void )
{
    return m_jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::iterator 
jobCache::jobCacheTable::end( void )
{
    return m_jobs.end( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::begin( void ) const
{
    return m_jobs.begin( );
}

//______________________________________________________________________________
jobCache::jobCacheTable::const_iterator 
jobCache::jobCacheTable::end( void ) const
{
    return m_jobs.end( );
}

//______________________________________________________________________________
jobCache* jobCache::getInstance() throw()
{
    if ( !s_instance )
        s_instance = new jobCache( );
    return s_instance;
}

//______________________________________________________________________________
jobCache::jobCache( void )
    : m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
      m_jobs( )
{ 
    jobDbManager *dbm = new jobDbManager( s_persist_dir, s_recoverable_db );

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
    try {
        load(); 
    } catch( ClassadSyntax_ex& ex ) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream()
                        << "jobCache::jobCache() - "
                        << "Failed to load the jobCache from the database. "
                        << "Reason is: " << ex.what()
                        << ". Giving up"
                        << log4cpp::CategoryStream::ENDLINE );
        abort();
    }
}

//______________________________________________________________________________
jobCache::~jobCache( ) 
{

}

//______________________________________________________________________________
void jobCache::load( void ) throw(ClassadSyntax_ex&)
{
//     // retrieve all records from DB
//     vector<string> records;
//     try { 
//         m_dbMgr->getAllRecords( records );
//     } catch(JobDbException& dbex) {
//         // this error is severe: an access to the
//         // underlying database failed 
//         CREAM_SAFE_LOG( m_log_dev->fatalStream() 
//                         << "jobCache::load() - "
//                         << "Failed to get all records from the database. "
//                         << "Reason is: " << dbex.what() << ". Giving up."
//                         << log4cpp::CategoryStream::ENDLINE );
//         abort();
//     }
//     for(vector<string>::const_iterator it=records.begin();
//         it != records.end();
//         ++it)
//         {
//             CreamJob cj( *it ); // can raise a ClassAdSyntax_ex
//             m_jobs.putJob( cj ); // update in-memory data structure
//         }
  
  /**
   * new implementation using the boost serializer
   *
   */
  vector<string> records;
  try {
    m_dbMgr->getAllRecords( records );
  } catch(JobDbException& dbex) {
    CREAM_SAFE_LOG( m_log_dev->fatalStream() 
		    << "jobCache::load() - "
		    << "Failed to get all records from the database. "
		    << "Reason is: " << dbex.what() << ". Giving up."
		    << log4cpp::CategoryStream::ENDLINE );
    abort();
  }
  for(vector<string>::const_iterator it=records.begin();it != records.end();++it)
    {
      CreamJob cj;
      istringstream tmpOs( *it );
      //tmpOs << *it;
      boost::archive::text_iarchive ia(tmpOs);
      ia >> cj;
      m_jobs.putJob( cj ); // update in-memory data structure
    }
}

//______________________________________________________________________________
jobCache::iterator jobCache::put(const CreamJob& cj)
{   
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache? 
    try {
      //m_dbMgr->put(cj.serialize(), cj.getCreamJobID(), cj.getGridJobID() );
      ostringstream ofs;
      boost::archive::text_oarchive oa(ofs);
      oa << cj;
      m_dbMgr->put( ofs.str(), cj.getCreamJobID(), cj.getGridJobID() );
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << dbex.what() << log4cpp::CategoryStream::ENDLINE );
        abort();
    }
    return m_jobs.putJob( cj );
}

//______________________________________________________________________________
void jobCache::print(ostream& os) {
    jobCacheTable::const_iterator it;
    for ( it=m_jobs.begin(); it!=m_jobs.end(); it++ ) {
        os << "GID=" << it->getGridJobID() 
           << " CID=" << it->getCreamJobID() 
           << " STATUS=" << it->getStatus()
           << endl;
    }
}

//______________________________________________________________________________
jobCache::iterator jobCache::lookupByCreamJobID( const string& creamJID )
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    return m_jobs.findJobByCID( creamJID );
}

//______________________________________________________________________________
jobCache::iterator jobCache::lookupByGridJobID( const string& gridJID )
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    return m_jobs.findJobByGID( gridJID );
}

//______________________________________________________________________________
jobCache::iterator jobCache::erase( jobCache::iterator it )
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    if ( it == m_jobs.end() ) {
        return it;
    }

    jobCache::iterator result = it;
    result++; // advance iterator
    // job found, log operation and remove

    try{
      m_dbMgr->delByCid( it->getCreamJobID() );
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << dbex.what() << log4cpp::CategoryStream::ENDLINE );
        abort();
    }

    m_jobs.delJob( it );    
    return result;
}

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
#include "SerializeException.h"
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
                         );
        abort();
    }
    m_dbMgr.reset( dbm );
    
    load();	
}

//______________________________________________________________________________
jobCache::~jobCache( ) 
{
  
}

//______________________________________________________________________________
void jobCache::load( void ) throw()
{
    static const char* method_name = "jobCache::load() - ";
  
    /**
     * new implementation using the boost serializer
     *
     */
    vector<string> records;
    try {
        m_dbMgr->initCursor();

        char *data;
        while( (data = (char*)m_dbMgr->getNextData()) != NULL ) {
            boost::scoped_ptr< char > toFree;
            toFree.reset( data );
            
            CreamJob cj;
            istringstream tmpOs;//( string(data) );
            tmpOs.str( data );

	    bool retry = true;
	    int sertry = 0;
	    while( retry ) {
	      try {
		{
                  boost::archive::text_iarchive ia(tmpOs);
                  ia >> cj;
                }
                
                m_GridJobIDSet.insert( cj.getGridJobID() );
                
	      } catch(SerializeException& ex ) {
		//                 CREAM_SAFE_LOG( m_log_dev->fatalStream()
		//                                 << method_name
		//                                 << "boost::archive::text_iarchive raised an exception: "
		//                                 << ex.what()
		//                                  );
		sertry++;
		if( sertry >= 3 ) {
		  // FIXME: put a FATAL log message here
		  exit(2);
		}
		
	      } 
	    } // while( retry )
        }
        
        m_dbMgr->endCursor();
        
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() 
                        << method_name
                        << "Failed to get a record from the database. "
                        << "Reason is: " << dbex.what() << ". Giving up."
                         );
        abort();
    } catch(...) {
      CREAM_SAFE_LOG( m_log_dev->fatalStream()
		      << method_name
		      << "Unknown exception catched"
		      );
      abort();
    }
}

//______________________________________________________________________________
jobCache::iterator jobCache::put(const CreamJob& cj)
{   
    static const char* method_name = "jobCache::put() - ";

    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache? 

    bool retry = true;
    int sertry = 0;

    while( retry ) {
      try {
	ostringstream ofs;
	{
	  boost::archive::text_oarchive oa(ofs);
	  oa << cj;
	}
	
	m_dbMgr->put( ofs.str(), cj.getCompleteCreamJobID(), cj.getGridJobID() );
	
	retry = false;

      } catch(JobDbException& dbex) {

        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
			<< dbex.what() 
			);
        abort();

      } catch( SerializeException& ex ) {
	sertry++;
	if(sertry >= 3 ) {
	  // FIXME: put a FATAL log message here
	  exit(2);
	}
      } catch(...) {
	CREAM_SAFE_LOG( m_log_dev->fatalStream()
		      << method_name
		      << "Unknown exception catched"
		      );
	abort();
      }
    }
    
    return make_iterator( (m_GridJobIDSet.insert( cj.getGridJobID() )).first );
}


//______________________________________________________________________________
jobCache::iterator 
jobCache::lookupByCompleteCreamJobID( const string& completeCreamJID )
  throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?

    int sertry = 0;
    bool retry = true;
    CreamJob cj;

    while( retry ) {
      
      try {
        string serializedJob( m_dbMgr->getByCid( completeCreamJID ) );
        
        istringstream tmpOs;//( string(data) );
        tmpOs.str( serializedJob );
        {
          boost::archive::text_iarchive ia(tmpOs);
          ia >> cj;
	}
        
	retry = false;

      }  catch(JobDbNotFoundException& ex) {
	
	return this->end();    

      }  catch(SerializeException& ex ) {
	
	sertry++;
	if(sertry >= 3 ) {
	  // FIXME: put a FATAL log message here
	  exit(2); // the ICE-safe will restart ICE soon
	}
	
      } catch(exception& ex) {
	
	CREAM_SAFE_LOG( m_log_dev->fatalStream() 
			<< ex.what()  );
	abort();
	
      }
    }


    //    try {
      
      set<string>::const_iterator it = m_GridJobIDSet.find( cj.getGridJobID() );
      return make_iterator(m_GridJobIDSet.find( cj.getGridJobID() ));

//     } catch(JobDbNotFoundException& ex) {
      
//       return this->end();    

//     } catch(exception& ex) {
      
//       CREAM_SAFE_LOG( m_log_dev->fatalStream() 
// 		      << ex.what()  );
//       abort();
      
//     }

}

//______________________________________________________________________________
jobCache::iterator jobCache::lookupByGridJobID( const string& gridJID )
  throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?
    //return m_jobs.findJobByGID( gridJID );
    
    return make_iterator( m_GridJobIDSet.find( gridJID ) );
}

//______________________________________________________________________________
jobCache::iterator jobCache::erase( jobCache::iterator it )
{
    static const char* method_name = "jobCache::erase() - ";

    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?

    if( it == end() ) 
        return it;

    string gid = it.get_grid_job_id();
    string next_gid = get_next_grid_job_id( gid );

    jobCache::iterator result( next_gid );

    CREAM_SAFE_LOG( m_log_dev->debugStream() << method_name
                    << "Removing " << it->describe()
                     );        
    try {
      m_dbMgr->delByGid( gid );
    } catch(JobDbException& dbex) {
        CREAM_SAFE_LOG( m_log_dev->fatalStream() << method_name
			<< dbex.what() 
			 );
        abort();
    }
    
    m_GridJobIDSet.erase( gid );

    return result;
}

//______________________________________________________________________________
jobCache::iterator jobCache::begin() 
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?    
    return make_iterator( m_GridJobIDSet.begin() );
}


//______________________________________________________________________________
jobCache::iterator jobCache::end() 
{
    return jobCacheIterator( );
}

string jobCache::get_next_grid_job_id( const string& gid ) const throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?    

    if ( gid.empty() )
        return gid;

    set<string>::const_iterator it = m_GridJobIDSet.find( gid );
    if ( it == m_GridJobIDSet.end() || (++it) == m_GridJobIDSet.end() )
        return string();
    else 
        return *it;
}

jobCacheIterator jobCache::make_iterator( const set< string >::const_iterator& it ) const throw()
{
    boost::recursive_mutex::scoped_lock L( jobCache::mutex ); // FIXME: Should locking be moved outside the jobCache?    
    if ( it == m_GridJobIDSet.end() )
        return jobCacheIterator();
    else
        return jobCacheIterator( *it );
}

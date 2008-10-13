
#ifndef __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__

#include <db_cxx.h>
#include <string>
#include <vector>
#include <map>
#include "JobDbException.h"
#include "JobDbNotFoundException.h"

#include "glite/ce/cream-client-api-c/scoped_timer.h"

namespace log4cpp {
    class Category;
};

namespace api_util = glite::ce::cream_client_api::util;

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class jobDbManager {

    friend class jobCache;
    friend class Iterator;

    DbEnv         m_env;
    Db           *m_creamJobDb;
    //Db           *m_cidDb;
    //Db           *m_gidDb;
    std::string   m_envHome;
    bool          m_valid;
    std::string   m_invalid_cause;
    DbTxn*        m_txn_handler;
    bool          m_cream_open;
    //bool          m_cid_open;
    //bool          m_gid_open;
    bool          m_env_open;
    int	 	  m_op_counter;
    int		  m_op_counter_chkpnt;
    log4cpp::Category* m_log_dev;
    
    std::string getByID( const std::string& id, Db* db ) throw(DbException&);
    
    // DO NOT COPY a jobDbManager
    jobDbManager(const jobDbManager&) : m_env(0) {}
     
    Dbc* mainCursor;

    Dbt It_key, It_data;

  protected:
    void initCursor( void ) throw(JobDbException&);
    void endCursor( void ) throw(JobDbException&);
    void* getNextData( void ) const  throw(JobDbException&);

  public:

    /**
       Wrapper class to safely acquire/release a lock on the BerkeleyDB
    */
    class scoped_lock {

      friend class jobDbManager;

      
      DbLock * m_lock;
      static DbEnv* s_dbenv;
      bool m_locked;

    protected:
      static void initDbEnv( DbEnv* env ) {
	s_dbenv = env;
      }

    public:
      
      scoped_lock(const long caller, const char* data/*Dbt *obj_to_lock */, int dataLen, bool exclusive) 
	{
	  m_locked = false;

	  api_util::scoped_timer T( "jobDbManager::scoped_lock::CTOR - TIMER for DB lock acquisition: " );
	  

	  if( s_dbenv && data && dataLen ) {
	    
	    Dbt obj_to_lock( (void*)data, dataLen+1 );

	    if(exclusive)
	      s_dbenv->lock_get( caller, 0, &obj_to_lock, DB_LOCK_WRITE, m_lock);
	    else
	      s_dbenv->lock_get( caller, 0, &obj_to_lock, DB_LOCK_READ, m_lock);
	    
	    m_locked = true;//&lock;
	  }
	  
	}
      
      ~scoped_lock() 
	{
	  if( s_dbenv && m_locked )
	    s_dbenv->lock_put( m_lock );
	}

    };

    jobDbManager(const std::string& env_home, 
    		 const bool recover = false, 
		 const bool autopurgelog = false, 
		 const bool read_only = false); // the directory pointed by env_home
    					       // MUST exist
					       // If the object creation fails
					       // this->isValid() returns false
					       // and this->getInvalidCause()
					       // returns the human readable reason
					       // of failure.
					       // For now I do prefer this solution
					       // to that one of exception raising.
					       // Only one process should keep open
					       // a database with recover=true
					       // otherwise a txn_begin raises an
					       // exception.
					    
    ~jobDbManager() throw();
    
    // Puts a string couple (cid, serializedCreamJob) in the database
    // Also puts the string couple (gid, cid) in a database useful for reverse resolution
    // (i.e. resolving creamJobID from gridJobID)
    void put(const std::string& serializedCreamJob, 
             /*const std::string& cid,*/
	     const std::string& gid) throw(JobDbException&);
	 
    // Like put but for many objects to store with a single transaction (more performant)
    // The argument is an hash map with creamJobID as key and a string pair
    // (gridJobID, serializedCreamJob) as value    
    //void mput(const std::map<std::string, std::pair<std::string, std::string> >&) 
    //  throw(JobDbException&);// {}     
	     
    //void delByCid( const std::string& cid ) throw(JobDbException&);
    void del( const std::string& cid ) throw(JobDbException&);
    
    //void delByGid( const std::string& gid ) throw(JobDbException&);
    
    //std::string getByCid( const std::string& cid) throw(JobDbException&, JobDbNotFoundException&);
    
    //std::string getByGid( const std::string& gid ) throw(JobDbException&, JobDbNotFoundException&);

    std::string get( const std::string& gid ) throw(JobDbException&, JobDbNotFoundException&);
    
    //void getAllRecords( std::vector<std::string>& target ) throw(JobDbException&);
    
    bool isValid( void ) const { return m_valid; }
    
    std::string getInvalidCause( void ) const { return m_invalid_cause; }
   
    void checkPointing( void );
   
    void dbLogPurge( void ); 

  };

}
}
}
}

#endif

/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
#ifndef __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__

#include <db_cxx.h>
#include <string>
#include <vector>
#include <map>
#include "JobDbException.h"
#include "JobDbNotFoundException.h"

//#include "glite/ce/cream-client-api-c/scoped_timer.h"

namespace log4cpp {
    class Category;
};

//namespace api_util = glite::ce::cream_client_api::util;

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class jobDbManager {

    friend class jobCache;
    friend class Iterator;

    DbEnv         m_env;
    Db           *m_creamJobDb;
    Db           *m_cidDb;
    Db           *m_gidDb;
    std::string   m_envHome;
    bool          m_valid;
    std::string   m_invalid_cause;
    DbTxn*        m_txn_handler;
    bool          m_cream_open;
    bool          m_cid_open;
    bool          m_gid_open;
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
             const std::string& cid,
	     const std::string& gid) throw(JobDbException&);
	 
    // Like put but for many objects to store with a single transaction (more performant)
    // The argument is an hash map with creamJobID as key and a string pair
    // (gridJobID, serializedCreamJob) as value    
    void mput(const std::map<std::string, std::pair<std::string, std::string> >&) 
      throw(JobDbException&);// {}     
	     
    void delByCid( const std::string& cid ) throw(JobDbException&);
    
    void delByGid( const std::string& gid ) throw(JobDbException&);
    
    std::string getByCid( const std::string& cid) throw(JobDbException&, JobDbNotFoundException&);
    
    std::string getByGid( const std::string& gid ) throw(JobDbException&, JobDbNotFoundException&);
    
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


#ifndef __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_JOBDBMANAGER_H__

#include <db_cxx.h>
#include <string>
#include <vector>
#include "JobDbException.h"

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class jobDbManager {
    DbEnv m_env;
    Db *m_creamJobDb;
    Db *m_cidDb;
    Db *m_gidDb;
    std::string m_envHome;
    bool m_valid;
    std::string m_invalid_cause;
    DbTxn* m_txn_handler;
    bool m_cream_open, m_cid_open, m_gid_open, m_env_open;
    
    std::string getByID( const std::string& id, Db* db ) throw(DbException&);
    
   public:
    jobDbManager(const std::string& env_home); // the directory pointed by env_home
    					       // MUST exist
					       // If the object creation fails
					       // this->isValid() returns false
					       // and this->getInvalidCause()
					       // returns the human readable reason
					       // of failure.
					       // For now I do prefer this solution
					       // to that one of exception raising.
					    
    ~jobDbManager() throw();
    
    void put(const std::string& serializedCreamJob, 
             const std::string& cid,
	     const std::string& gid) throw(JobDbException&);
	     
    void delByCid( const std::string& cid ) throw(JobDbException&);
    
    void delByGid( const std::string& gid ) throw(JobDbException&);
    
    std::string getByCid( const std::string& cid) throw(JobDbException&);
    
    std::string getByGid( const std::string& gid ) throw(JobDbException&);
    
    void getAllRecords( std::vector<std::string>& target ) throw(JobDbException&);
    
    bool isValid( void ) const { return m_valid; }
    
    std::string getInvalidCause( void ) const { return m_invalid_cause; }
    
  };

}
}
}
}

#endif


#ifndef __GLITE_WMS_ICE_UTIL_CEDBMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_CEDBMANAGER_H__

#include <db_cxx.h>
#include <string>
#include <vector>
#include <map>
#include "CEDbException.h"
#include <utility>

//#define MAX_ICE_DB_LOGSIZE 65536

namespace log4cpp {
    class Category;
};

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class CEDbManager {
    DbEnv         m_env;
    Db           *m_CEDb;
    Db		 *m_DNDb;
    std::string   m_envHome;
    bool          m_valid;
    std::string   m_invalid_cause;
    DbTxn*        m_txn_handler;
    bool          m_ce_open;
    bool	  m_dn_open;    
    bool          m_env_open;
    int	 	  m_op_counter;
    int		  m_op_counter_chkpnt;
    log4cpp::Category* m_log_dev;
    
    //std::string getCEMonByCREAM( const std::string& url, Db* db ) throw(DbException&);
    
    // DO NOT COPY a urldnDbManager
    CEDbManager(const CEDbManager&) : m_env(0) {}
     
   public:
    CEDbManager(const std::string& env_home,
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
					    
    ~CEDbManager() throw();
    
    
    void put(const std::string& cream_url, const std::vector<std::pair<std::string, std::string> >& cemonDN ) throw(CEDbException&);
	 
    void mput(const std::map<std::string, std::pair<std::string, std::string> >&) throw(CEDbException&);// {}     

    void getByCreamUrl( const std::string&, std::string&, std::string&  ) throw(CEDbException&);
    
    bool isValid( void ) const { return m_valid; }
    
    std::string getInvalidCause( void ) const { return m_invalid_cause; }
   
    void checkPointing( void );
   
    void dbLogPurge( void ); 
    
    void getAllRecords( std::map<std::string, std::pair<std::string, std::string> >& ) throw(CEDbException&);
  };

}
}
}
}

#endif

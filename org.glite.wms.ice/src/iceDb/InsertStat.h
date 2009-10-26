#ifndef GLITE_WMS_ICE_ICEDB_INSERT_STAT_H
#define GLITE_WMS_ICE_ICEDB_INSERT_STAT_H

#include "AbsDbOperation.h"
//#include <string>
#include <ctime>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     * This operation creates a new job into the database. It does not
     * update an existing job with the same gridjobid.
     */
    class InsertStat : public AbsDbOperation { 

    public:
    InsertStat( const time_t timestamp,
		const short status, 
		const std::string& caller ) : AbsDbOperation(caller),
      m_timestamp( timestamp ),
      m_status( status ) {}
      
      virtual void execute( sqlite3* db ) throw( DbOperationException& );
      
  protected:
      const time_t            m_timestamp;
      const short             m_status;
  };
  
} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

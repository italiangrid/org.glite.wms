#ifndef GLITE_WMS_ICE_ICEDB_REMOVE_JOB_BYDBID_H
#define GLITE_WMS_ICE_ICEDB_REMOVE_JOB_BYDBID_H

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     * This operation removes a job from the database.
     */
    class RemoveJobsByDbID : public AbsDbOperation { 
    public:
        RemoveJobsByDbID( const long long );
        virtual void execute( sqlite3* db ) throw( DbOperationException& );
	
    protected:
        const long long  m_dbid;
    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

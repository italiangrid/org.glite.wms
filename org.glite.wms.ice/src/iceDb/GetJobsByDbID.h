
#ifndef GLITE_WMS_ICE_GET_JOBS_BYDBID_H
#define GLITE_WMS_ICE_GET_JOBS_BYDBID_H

#include "AbsDbOperation.h"
#include "iceUtils/creamJob.h"
#include <list>
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetJobsByDbID : public AbsDbOperation {
    protected:

	std::list< glite::wms::ice::util::CreamJob > *m_result;
	const long long                               m_dbid;

    public:
        GetJobsByDbID( std::list< glite::wms::ice::util::CreamJob >&,
		       const long long dbid );

        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

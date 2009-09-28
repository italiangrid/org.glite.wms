#ifndef GLITE_WMS_ICE_GET_DBID
#define GLITE_WMS_ICE_GET_DBID

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetDbID : public AbsDbOperation {
    protected:

	const std::string  m_creamurl;
	long long          m_result;
	bool               m_found;

    public:
        GetDbID( const std::string& );
		

	long long getDbID( void ) const { return m_result; }
	bool      found( void ) const { return m_found; }

        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

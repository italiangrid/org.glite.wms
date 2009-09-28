#ifndef GLITE_WMS_ICE_GET_EVENTID
#define GLITE_WMS_ICE_GET_EVENTID

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetEventID : public AbsDbOperation {
    protected:

	const std::string  m_userdn;
	const std::string  m_creamurl;
	long long          m_result;
	bool               m_found;

    public:
        GetEventID( const std::string&, 
		    const std::string&);

	long long getEventID( void ) const { return m_result; }
	bool      found( void ) const { return m_found; }
        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

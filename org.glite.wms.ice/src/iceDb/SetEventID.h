#ifndef GLITE_WMS_ICE_SET_EVENTID
#define GLITE_WMS_ICE_SET_EVENTID

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class SetEventID : public AbsDbOperation {
    protected:

	const std::string  m_userdn;
	const std::string  m_creamurl;
	const long long    m_new_eventid;

    public:
        SetEventID( const std::string& userdn, 
		    const std::string& ceurl,
		    const long long newid, 
		    const std::string& caller )
	  : AbsDbOperation( caller ),
    m_userdn( userdn ),
    m_creamurl( ceurl ),
    m_new_eventid( newid ) {}


        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

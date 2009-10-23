#ifndef GLITE_WMS_ICE_SET_DBID
#define GLITE_WMS_ICE_SET_DBID

#include "AbsDbOperation.h"

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class SetDbID : public AbsDbOperation {
    protected:

	const std::string  m_creamurl;
	const long long    m_new_dbid;

    public:
        SetDbID( const std::string& ceurl,
		 const long long newid, 
		 const std::string& caller )
  : AbsDbOperation(caller),
    m_creamurl( ceurl ),
    m_new_dbid( newid )
{

}


        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

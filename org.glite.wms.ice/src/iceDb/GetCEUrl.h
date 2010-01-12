#ifndef GLITE_WMS_ICE_GET_CEURL
#define GLITE_WMS_ICE_GET_CEURL

#include "AbsDbOperation.h"
#include <set>
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace db {

    /**
     *
     */
    class GetCEUrl : public AbsDbOperation {
    protected:

	std::set<std::string> *m_celist;

    public:
        GetCEUrl( std::set<std::string> &target, const std::string& caller ) 
	  : AbsDbOperation( caller ), m_celist( &target ) {}

        virtual void execute( sqlite3* db ) throw( DbOperationException& );

    };

} // namespace db
} // namespace ice
} // namespace wms
} // namespace glite

#endif

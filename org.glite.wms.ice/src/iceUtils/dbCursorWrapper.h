#ifndef __GLITE_WMS_ICE_UTIL_DBCW_H__
#define __GLITE_WMS_ICE_UTIL_DBCW_H__

#include <db_cxx.h>

namespace glite {
namespace wms {
namespace ice {
namespace util {

   class cursorWrapper {
     Dbc* cursor;
    public:
     cursorWrapper( Db* aDatabase ) throw(DbException&) : cursor(NULL) {
       aDatabase->cursor(NULL, &cursor, 0); // can raise a DbException
     }
     ~cursorWrapper() throw() {
       if(cursor) cursor->close();
     }
     int get(Dbt* key, Dbt* data) throw(DbException&) {
       return cursor->get(key, data, DB_NEXT); // can raise a DbException
     }
   };
   }}}}
#endif

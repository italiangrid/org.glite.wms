/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
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

/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *    http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Remove a job from the cache
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "RemoveJobByCid.h"

#include "boost/format.hpp"

#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

void RemoveJobByCid::execute( sqlite3* db ) throw ( DbOperationException& )
{
    string sqlcmd = boost::str( boost::format( 
      "DELETE FROM jobs " \
      " WHERE complete_cream_jobid = \'%1%\'; " ) % m_creamjobid );

    do_query( db, sqlcmd );
}

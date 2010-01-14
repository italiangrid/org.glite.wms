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
 * DB operation used to remove a job with given proxy certificate
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "RemoveJobByProxy.h"

#include "boost/format.hpp"

#include <iostream>

using namespace glite::wms::ice::db;
using namespace std;

// RemoveJobByProxy::RemoveJobByProxy( const string& proxy ) :
//     m_proxy( proxy )
// {

// }

void RemoveJobByProxy::execute( sqlite3* db ) throw ( DbOperationException& )
{
    string sqlcmd = boost::str( boost::format( 
      "DELETE FROM jobs " \
      " WHERE userproxy = \'%1%\'; " ) % m_proxy );

//  if(::getenv("GLITE_WMS_ICE_PRINT_QUERY") )
//    cout << "Executing query ["<<sqlcmd<<"]"<<endl;

    do_query( db, sqlcmd );
}

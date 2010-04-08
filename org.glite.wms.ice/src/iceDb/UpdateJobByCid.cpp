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

#include "UpdateJobByCid.h"
#include "ice-core.h"
#include <sstream>

using namespace glite::wms::ice;
using namespace std;

void db::UpdateJobByCid::execute( sqlite3* db ) throw ( DbOperationException& )
{

 
    ostringstream sqlcmd("");
 
    sqlcmd << "UPDATE jobs SET ";
    
    for(list< pair<string, string> >::const_iterator it=m_nameval_list.begin();
        it!=m_nameval_list.end();
	++it)
    {
    
  
    
      sqlcmd << it->first << "="
	     << Ice::instance()->get_tmp_name() 
	     << it->second
	     << Ice::instance()->get_tmp_name() << ",";
    }
    
    string tmp = sqlcmd.str();
    if( !tmp.empty() )
      tmp = tmp.substr(0, tmp.length()-1); // remove the trailing ","
    
    sqlcmd.str( "" );
    
    sqlcmd << tmp << " WHERE complete_cream_jobid="
	   << Ice::get_tmp_name() 
	   << m_cid << Ice::get_tmp_name() << ";";
     
    do_query( db, sqlcmd.str() );
}

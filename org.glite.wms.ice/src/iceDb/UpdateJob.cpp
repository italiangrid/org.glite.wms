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

#include "UpdateJob.h"
#include "ice-core.h"
#include <iostream>



//using namespace glite::wms::ice::db;
using namespace glite::wms::ice;
using namespace std;

void db::UpdateJob::execute( sqlite3* db ) throw ( DbOperationException& )
{
    string sqlcmd = "";

//    sqlcmd += "UPDATE jobs SET " + m_job.get_query_update_values();//fields( );
 //   sqlcmd += /*" VALUES(" + m_job.get_query_values( ) +*/ " WHERE ";
 //   sqlcmd += util::CreamJob::grid_jobid_field( ) + "=" + Ice::instance()->get_tmp_name() + m_job.grid_jobid( ) + Ice::instance()->get_tmp_name() + ";";
   
    m_job.update_database( sqlcmd );
    if( sqlcmd.empty( ) ) return;
    
    do_query( db, sqlcmd );
}

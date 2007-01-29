/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * youw may not use this file except in compliance with the License. 
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
 * Jobdir request class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "Request_jobdir.h"
#include <boost/filesystem/operations.hpp>
#include <fstream>

using namespace glite::wms::ice::util;

Request_jobdir::Request_jobdir( boost::filesystem::path old_path ) :
    m_old_path( old_path ),
    m_request( )
{
    std::ifstream is( old_path.string().c_str() );
    while ( !is.eof() ) {
        std::string val;
        is >> val;
        m_request.append( val );
    }
}

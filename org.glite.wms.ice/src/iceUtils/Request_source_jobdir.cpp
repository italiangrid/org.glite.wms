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
 * Request Source jobdir
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "Request_source_jobdir.h"
#include "Request_jobdir.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include <vector>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace utilities=glite::wms::common::utilities;
namespace fs=boost::filesystem;
using namespace glite::wms::ice::util;
using namespace std;

Request_source_jobdir::Request_source_jobdir( const std::string& jdir_name, bool create ) :
    Request_source( jdir_name ),
    m_jobdir( 0 )
{
    if ( create ) {
        // Tries to create the directory structure
        utilities::JobDir::create( jdir_name );
    }

    try {
        m_jobdir = new utilities::JobDir( jdir_name );
    } catch( std::exception& ex ) {
        cerr << "Jobdir creation failed!" << endl;
        abort(); // FIXME
    }
}
 
Request_source_jobdir::~Request_source_jobdir( )
{
    delete m_jobdir; // FIXME: use a scoped_ptr instead?
}

list<Request*> Request_source_jobdir::get_requests( void )
{
    list< Request* > result;
    
    utilities::JobDir::iterator b, e;
    boost::tie(b, e) = m_jobdir->new_entries();
    
    for ( ; b != e; ++b) {
        
        fs::path const& new_file = *b;
        fs::path const old_file = m_jobdir->set_old(new_file);
    
        result.push_back( new Request_jobdir( old_file ) );
    }
    
    return result;
}

void Request_source_jobdir::remove_request( Request* req )
{
    Request_jobdir* req_jobdir = dynamic_cast< Request_jobdir* >( req );
    if ( 0 != req_jobdir ) {
        fs::remove( req_jobdir->get_path() ); // FIXME? does it throw something?
    }
}

void Request_source_jobdir::put_request( const string& ad )
{
    m_jobdir->deliver( ad );
}


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
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace utilities=glite::wms::common::utilities;
namespace fs=boost::filesystem;
using namespace glite::wms::ice::util;
using namespace std;

Request_source_jobdir::Request_source_jobdir( const std::string& jdir_name ) :
    m_jobdir( jdir_name )
{

}
 
Request_source_jobdir::~Request_source_jobdir( )
{

}

typedef boost::function<void()> cleanup_type;

list<Request*> Request_source_jobdir::get_requests( void )
{
    //   requests_type result;

  utilities::JobDir::iterator b, e;
  boost::tie(b, e) = m_jobdir.new_entries();

  // classad::ClassAdParser parser;

  for ( ; b != e; ++b) {

    fs::path const& new_file = *b;
    fs::path const old_file = m_jobdir.set_old(new_file);
    cleanup_type cleanup(boost::bind(fs::remove, old_file));
    // if the request is not valid, clean it up automatically
    utilities::scope_guard cleanup_guard(cleanup);

    fs::ifstream is(old_file);
    // ClassAdPtr command_ad(parser.ParseClassAd(is));
//     if (command_ad) {
//       cleanup_guard.dismiss();
//       result.push_back(std::make_pair(command_ad, cleanup));
//     } else {
//       Info("invalid request");
//       continue;
//     }

  }

  //  return result;
}

void Request_source_jobdir::remove_request( Request* req )
{
    Request_jobdir* req_jobdir = dynamic_cast< Request_jobdir* >( req );
    if ( 0 != req_jobdir ) {
        fs::remove( req_jobdir->get_path() );
    }
}

void Request_source_jobdir::put_request( const string& ad )
{

}


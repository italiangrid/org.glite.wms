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
 * Request Source Filelist
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "Request_source_filelist.h"
#include "Request_filelist.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <vector>

namespace wmsutils_ns=glite::wms::common::utilities;
namespace fs = boost::filesystem;
using namespace glite::wms::ice::util;
using namespace std;

typedef wmsutils_ns::FLExtractor<std::string>::iterator FLEit;

Request_source_filelist::Request_source_filelist( const std::string& fl_name, bool create ) :
    Request_source( fl_name ),
    m_filelist_extractor( fl_name )
{
    if ( create ) {
        fs::path fl_p( fl_name, fs::native );
        
        if( ( !fl_p.native_file_string().empty()) && 
            !fs::exists(fl_p.branch_path()) ) {
            try {
                fs::create_directories( fl_p.branch_path() );
            } catch( ... ) {
                cerr << "Filelist creation failed!" << endl;
                abort(); // FIXME
            }
        }
    }

    try {
        m_filelist.open( m_name );
    } catch( ... ) {
//         CREAM_SAFE_LOG( m_log_dev->fatalStream()
//                         << "Ice::CTOR() - Catched unknown exception"
//                         << log4cpp::CategoryStream::ENDLINE
//                         );
        exit( 1 ); // FIXME
    }
}
 
Request_source_filelist::~Request_source_filelist( )
{

}
   
list<Request*> Request_source_filelist::get_requests( void )
{
    vector< FLEit > requests;
    list< Request* > result;
    try { 
        requests = m_filelist_extractor.get_all_available();
    }
    catch( exception& ex ) {
//         CREAM_SAFE_LOG(
//                        m_log_dev->fatalStream() 
//                        << "Ice::getNextRequest() - " << ex.what()
//                        << log4cpp::CategoryStream::ENDLINE
//                        );
        exit(1); // FIXME
    }
    for ( unsigned j=0; j < requests.size(); j++ ) {
        result.push_back( new Request_filelist( requests[j] ) );
    }
    return result;
}

void Request_source_filelist::remove_request( Request* req )
{
    Request_filelist* req_fl = dynamic_cast< Request_filelist* >( req );
    if ( 0 != req_fl ) {
        m_filelist_extractor.erase( req_fl->get_iterator() );
    }
}

void Request_source_filelist::put_request( const string& ad )
{
    wmsutils_ns::FileListMutex mx(m_filelist);
    wmsutils_ns::FileListLock  lock(mx);
    m_filelist.push_back( ad );
}


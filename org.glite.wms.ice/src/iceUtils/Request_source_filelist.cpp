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
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include <utility>
#include <string>

namespace wmsutils_ns=glite::wms::common::utilities;
namespace fs = boost::filesystem;
using namespace glite::wms::ice::util;
using namespace std;
namespace api_util  = glite::ce::cream_client_api::util;

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
            } catch(exception& ex) {
	    
	      CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
	      		     << "Request_source_filelist::CTOR - Error creating path ["
			     << fl_name << "]: " << ex.what()
			     );
	      abort();
	      
	    }catch( ... ) {
                //cerr << "Filelist creation failed!" << endl;
		CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
	      		     << "Request_source_filelist::CTOR - Error creating path ["
			     << fl_name << "]: Catched unknown exception"
			     );
                abort(); // FIXME
		
            }
        }
    }

    try {
    
        m_filelist.open( m_name );
	
    } catch(exception& ex) {
    
      CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
	      		     << "Request_source_filelist::CTOR - Error opening filelist ["
			     << m_name << "]: " << ex.what()
			     );
      abort();
	      
    } catch( ... ) {
         CREAM_SAFE_LOG( api_util::creamApiLogger::instance()->getLogger()->fatalStream()
                         << "Request_source_filelist::CTOR() - Error opening filelist: Catched unknown exception"
                         
                         );
	 abort(); // FIXME
    }
}
 
Request_source_filelist::~Request_source_filelist( )
{

}
   
list<Request*> Request_source_filelist::get_requests( size_t max_size )
{
#ifdef FOOBAR
    vector< FLEit > requests;
    list< Request* > result;
    try { 
        requests = m_filelist_extractor.get_all_available();
    }
    catch( exception& ex ) {
        CREAM_SAFE_LOG(
                       api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
                       << "Request_source_filelist::get_requests() - " << ex.what()
                       
                       );
        abort(); // FIXME
    }
    for ( unsigned j=0; j < requests.size(); j++ ) {
        result.push_back( new Request_filelist( requests[j] ) );
    }
    return result;
#else

    std::pair< FLEit , bool  > request;
    list< Request* > result;

    while( result.size() < max_size ) {

        try { 
            request = m_filelist_extractor.try_get_one();
        }
        catch( exception& ex ) {
            CREAM_SAFE_LOG(
                           api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
                           << "Request_source_filelist::get_requests() - Failed to get request due to exception: " << ex.what()
                           
                           );
            abort(); // FIXME
        }

        if ( !request.second ) 
            break; // exit the loop when no more requests are available
        else {
            result.push_back( new Request_filelist( request.first ) );
        }
    }
    return result;
#endif
}
  
// Request* Request_source_filelist::get_single_request( void )
// {
// #ifdef FOOBAR
//   std::pair< FLEit , bool  > request;
// //     //list< Request* > result;
//     try { 
//         request = m_filelist_extractor.try_get_one();
//     }
//     catch( exception& ex ) {
//         CREAM_SAFE_LOG(
//                        api_util::creamApiLogger::instance()->getLogger()->fatalStream() 
//                        << "Request_source_filelist::get_single_request() - " << ex.what()
//                        
//                        );
//         abort(); // FIXME
//     }
// //     for ( unsigned j=0; j < requests.size(); j++ ) {
// //         result.push_back( new Request_filelist( requests[j] ) );
// //     }
// //     return result;
// 
//   if( !request.second ) return NULL;
//   return new Request_filelist( request.first );
// #else
//   return 0;
// #endif
// }

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


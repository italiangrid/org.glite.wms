#include <list>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <getopt.h>

#include "iceDb/Transaction.h"
#include "iceDb/GetFields.h"
#include "iceConfManager.h"

#include "boost/algorithm/string.hpp"

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace db = glite::wms::ice::db;
void printhelp( void ) {}

// bool validate( const string& field ) {
//   if( field == "gridjobid" ||
//       field == "creamjobid" ||
//       field == "complete_cream_jobid" ||
//       field == "userdn" ||
//       field == "delegationid" ||
//       field == "userproxy" ||
//       field == "creamurl" ||
//       field == "myproxyurl" ||
//       field == "proxy_renewable" ||
//       field == "failure_reason" ||
//       field == "status" ||
//       field == "leaseid" ||
//       field == "proxycert_timestamp"
//       )
//     return true;
//   return false;
// }

string status_to_numstr( const std::string& status ) {
  if(status == "REGISTERED" ) return "0";
  if(status == "PENDING" ) return "1";
  if(status == "IDLE" ) return "2";
  if(status == "RUNNING" ) return "3";
  if(status == "REALLY_RUNNING" ) return "4";
  if(status == "CANCELLED" ) return "5";
  if(status == "HELD" ) return "6";
  if(status == "ABORTED" ) return "7";
  if(status == "DONE_OK" ) return "8";
  if(status == "DONE_FAILED" ) return "9";
  if(status == "UNKNOWN" ) return "10";
  if(status == "PURGED" ) return "11";

  cerr << "Invalid status [" << status << "]. STOP!" << endl;
  exit(1);
}

int main( int argc, char* argv[] )
{
  string           conf_file( "glite_wms.conf" );
  string           states("");
  string           sqlquery("");
  bool             all_status(true);
  bool             debug( false );
  int              option_index( 0 );
  list< string >   fields_to_retrieve;

  while( 1 ) {
    int c;
    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {"debug", 0, 0, 'd'},
      {"status-filter", 1, 0, 's'},
      {"conf", 1, 0, 'c'},
      /*****************************/
      {"userdn", 0, 0, 'u'},
      {"creamjobid",0,0,'C'},
      {"gridjobid",0,0,'G'},
      {"userproxy",0,0,'p'},
      {"cream-url",0,0,'r'},
      {"myproxy-url",0,0,'m'},
      {"status",0,0,'S'},
      {"lease-id",0,0,'L'},
      {"delegation-id",0,0,'D'},
      {"proxy-exptime",0,0,'t'},
      {"worker-node",0,0,'w'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "hds:c:uCGprmSLDtw", long_options, &option_index);
    
    if ( c == -1 )
      break;
    switch(c) {
    case 'h':
      printhelp();
      exit(0);
      break;
    case 'd':
      debug = true;
      break;
    case 's':
      states = string(optarg);
      all_status = false;
      break;
    case 'c':
      conf_file = string(optarg);
      break;
    case 'u':
      fields_to_retrieve.push_back( "userdn" );
      break;
    case 'C':
      fields_to_retrieve.push_back( "creamjobid" );
      break;
    case 'G':
      fields_to_retrieve.push_back( "gridjobid" );
      break;
    case 'p':
      fields_to_retrieve.push_back( "userproxy" );
      break;
    case 'r':
      fields_to_retrieve.push_back( "creamurl" );
      break;
    case 'm':
      fields_to_retrieve.push_back( "myproxyurl" );
      break;
    case 'S':
      fields_to_retrieve.push_back( "status" );
      break;
    case 'L':
      fields_to_retrieve.push_back( "leaseid" );
      break;
    case 'D':
      fields_to_retrieve.push_back( "delegationid" );
      break;
    case 't':
      fields_to_retrieve.push_back( "proxycert_timestamp" );
      break;
    case 'w':
      fields_to_retrieve.push_back( "worker_node" );
      break;
    default:
      cerr << "Type " << argv[0] << " --help|-h for an help" << endl;
      exit(1);
    }
  }

  
  iceUtil::iceConfManager::init( conf_file );
  try{
    iceUtil::iceConfManager::getInstance();
  }
  catch(iceUtil::ConfigurationManager_ex& ex) {
    cerr << ex.what() << endl;
    exit(1);
  }

  vector< string > _fields_array;
//   boost::split(_fields_array, fields_to_retrieve, boost::is_any_of(","));

//   if( !_fields_array.size() ) exit( 0 );

//   for(vector< string >::const_iterator it = _fields_array.begin();
//       it != _fields_array.end();
//       ++it )
//     {
//       if( !validate( *it ) ) {
// 	cerr << "Field [" << *it << "] is not defined in the job table of ICE. STOP!" << endl;
// 	exit(1);
//       }
//     }

//   list< string > fields_list;

//   copy( fields_to_retrieve.begin(), fields_to_retrieve.end(), back_inserter( fields_array ) );

  vector< string > _states_array;
  boost::split(_states_array, states, boost::is_any_of(","));
  
  list< string > states_array;
  copy( _states_array.begin(), _states_array.end(), back_inserter( states_array ) );
  
  list< pair< string, string > > params;
  
  for( list< string >::const_iterator it = states_array.begin();
       it != states_array.end();
       ++it )
    {
      params.push_back( make_pair( "status", status_to_numstr(*it) ));
    }

  if( fields_to_retrieve.empty() )
    {
      fields_to_retrieve.push_back( "*" );
    }

  db::GetFields getter( fields_to_retrieve, params );
  db::Transaction tnx;
  tnx.execute( &getter );
  list< vector< string > > result = getter.get_values();
  cout << result.size() << " item(s) found" << endl;

  // 
//   db::GetFields getter( fields_array, params );
//   db::Transaction tnx;
//   tnx.execute( &getter );
// 
//   list< vector< string > > result = getter.get_values();
// 
//   cout << result.size() << " item(s) found" << endl;
  
}

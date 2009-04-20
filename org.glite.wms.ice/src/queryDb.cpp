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

std::string status_to_numstr( const std::string& status ) {
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
  
  std::cerr << "Invalid status [" << status << "]. STOP!" << std::endl;
  exit(1);
}

using namespace std;
namespace iceUtil = glite::wms::ice::util;
namespace db = glite::wms::ice::db;
void printhelp( void ) {
  cout << "USAGE: queryDb --conf|-c <WMS CONFIGURATION FILE> [options]" << endl;
  cout << endl << "options: " << endl;
  cout << "  --verbose|-v\t\tVerbose output (print each db's record" << endl;
  //  cout << "\t--debug|-d\tPrint the SQL query before executing it" << endl;
  cout << "  --status-filter|-s\tSelect only records in which the status column is one"<<endl
       << "  \t\t\tof those specified as option argument; more states can"<<endl
       << "  \t\t\tbe ',' separated and they must be:"<<endl;
  cout << "\t\t\tREGISTERED"<<endl;
  cout << "\t\t\tREGISTERED"<<endl;
  cout << "\t\t\tPENDING"<<endl;
  cout << "\t\t\tIDLE"<<endl;
  cout << "\t\t\tRUNNING"<<endl;
  cout << "\t\t\tREALLY-RUNNING"<<endl;
  cout << "\t\t\tCANCELLED"<<endl;
  cout << "\t\t\tHELD"<<endl;
  cout << "\t\t\tABORTED"<<endl;
  cout << "\t\t\tDONE_OK"<<endl;
  cout << "\t\t\tDONE_FAILED"<<endl;
  cout << "\t\t\tUNKNOWN"<<endl;
  cout << "\t\t\tPURGED"<<endl<<endl;
  cout << "  --userdn|-u\t\tPrint the USERDN column of the job table" << endl;
  cout << "  --creamjobid|-C\tPrint the CREAM JOB ID column of the job table" << endl;
  cout << "  --gridjobid|-G\tPrint the GRID JOB ID column of the job table" << endl;
  cout << "  --userproxy|-p\tPrint the USER PROXY column of the job table" << endl;
  cout << "  --cream-url|-r\tPrint the CREAM URL column of the job table" << endl;
  cout << "  --myproxy-url|-m\tPrint the MYPROXY URL column of the job table" << endl;
  cout << "  --status|-S\t\tPrint the STATUS column of the job table" << endl;
  cout << "  --lease-id|-L\t\tPrint the LEASE-ID column of the job table" << endl;
  cout << "  --delegation-id|-D\tPrint the DELEGATION-ID column of the job table" << endl;
  cout << "  --proxy-exptime|-t\tPrint the PROXY EXPIRATION TIME column of the job table" << endl;
  cout << "  --worker-node|-w\tPrint the WORKER-NODE column of the job table" << endl;
  cout << "  --help|-h\t\tPrint this help" << endl;
}


int main( int argc, char* argv[] )
{
  string           conf_file( "glite_wms.conf" );
  string           states("");
  string           sqlquery("");
  bool             all_status(true);
  bool             debug( false );
  bool             verbose( false );
  int              option_index( 0 );
  list< string >   fields_to_retrieve;

  while( 1 ) {
    int c;
    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
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
      {"verbose", 0, 0, 'v'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "hs:c:uCGprmSLDtwv", long_options, &option_index);
    
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
      fields_to_retrieve.push_back( "complete_cream_jobid" );
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
    case 'v':
      verbose = true;
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
  getter.use_or_clause();
  db::Transaction tnx;
  tnx.execute( &getter );
  list< vector< string > > result = getter.get_values();
  if(verbose) {
    for( list< vector< string > >::const_iterator it=result.begin();
	 it != result.end();
	 ++it)
      {
	for( vector< string >::const_iterator jt=it->begin();
	     jt != it->end();
	     ++jt)
	  {
	    cout << "[" << *jt << "] ";
	  }
	cout << endl;
      }
    cout << endl << "------------------------------------------------" << endl;
  }
  cout << result.size() << " item(s) found" << endl;

}

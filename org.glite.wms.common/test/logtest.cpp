#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <boost/thread/tss.hpp>

#include "../src/logger/logstream.h"
#include "../src/logger/manipulators.h"
#include "../src/logger/edglog.h"
#include "../src/utilities/streamdescriptor.h"

USING_COMMON_NAMESPACE;

using namespace std;

namespace ts = edg::workload::common::logger::threadsafe;

void cunicolo( logger::logstream &log )
{
  logger::StatePusher    pusher( log, "cunicolo()" );

  log << logger::setlevel( logger::null ) << "Ma allora non capisci..." << endl;

  return;
}

void funzione( logger::logstream &log )
{
  logger::StatePusher    pusher( log, "funzione()" );

  cunicolo( log );
  log << logger::setlevel( logger::null ) << "Ciao, comprati Arrahpaho..." << endl;

  return;
}

int main( void )
{
  utilities::create_file( "edglog.log" );

  int                       n;
  fstream                   ofs( "edglog.log", ios::out | ios::in );
  //  logger::logstream         log( "edglog.log", logger::medium );
  //  logger::logstream         log( cerr, logger::medium );
  logger::logstream         log( ofs, logger::medium );
  logger::StatePusher       pusher( log, "::main()" );

  log.activate_log_rotation( 100000, "edglog.rot", 20 );

  cout << "log.bad() = " << log.bad() << " log.good() = " << log.good() << endl;

//    ts::edglog.unsafe_attach( log );

//    ts::edglog << logger::setfunction( "main()" ) << logger::setlevel( logger::high )
//  	     << "Hello world !!!" << endl;
//    ts::edglog << logger::setlevel( logger::low ) << "Starting stress test..." << endl;
//    ts::edglog << logger::setlevel( logger::high ) << "Very very very very very very very very very very very long thing..." << endl;
//    ts::edglog << logger::setlevel( logger::low ) << "Short thing..." << endl;

  log << logger::setlevel( logger::null );
  for( n = 0; n < 10000; ++n )
    log << "Log n. " << n << endl;

  log << logger::setlevel( logger::high )
      << "Hello world !!!" << endl;
  log << logger::setlevel( logger::low ) << "Starting stress test..." << endl;
  log << logger::setlevel( logger::high ) << "Very very very very very very very very very very very long thing..." << endl;
  log << logger::setlevel( logger::low ) << "Short thing..." << endl << endl;
  funzione( log );

  log << logger::setmultiline( true ) << "Cosa\nCosa\nCosa" << endl;
  log << "Ape\nMaya" << endl;
  log << "Hello World..." << endl;
  log << logger::setmultiline( false );

//    for( n = 90; n < 10000; ++n )
//      log << "Log n. " << n << endl;

  return 0;
}

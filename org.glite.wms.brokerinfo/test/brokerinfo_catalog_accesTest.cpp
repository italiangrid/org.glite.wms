#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"


#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/utilities/LineParser.h"
#include "glite/wms/common/utilities/LineParserExceptions.h"




#include <glite/wms/common/utilities/classad_utils.h>

#include <classad_distribution.h>

#include <iostream>
#include <string>
#include <fstream>

namespace logger = glite::wms::common::logger;

using namespace std;
using namespace glite::wms::common::utilities;
using namespace glite::wms::common::configuration;

using namespace glite::wms::brokerinfo;


#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

LineOption  options[] = {
    { 'c', 1, "conf_file", "\t use conf_file as configuration file. glite_wms.conf is the default" },
    { 'j', 1, "jdl_file", "\t use jdl_file as input file." },
    { 'n', 1, "number", "\t number of retry" },
//    { 'v', no_argument, "verbose",     "\t be verbose" },
    { 'l', no_argument, "verbose",     "\t be verbose on log file" }
};

int main(int argc, char* argv[])
{
  std::vector<LineOption> optvec( options, options + sizeof(options)/sizeof(LineOption) );
  LineParser options( optvec, 0 );

  string conf_file;

  string req_file;

  try {
     options.parse( argc, argv );
     conf_file.assign( options.is_present('c') ? options['c'].getStringValue() : "glite_wms.conf" );

     Configuration conf(conf_file.c_str(), ModuleType::network_server);

     NSConfiguration const* const ns_config(conf.ns());

//     if( options.is_present('v') && !options.is_present('l'))   logger::threadsafe::edglog.open(std::clog, glite::wms::common::logger::debug);
                                                                                        
     if( options.is_present('l') ) logger::threadsafe::edglog.open(ns_config->log_file(), glite::wms::common::logger::debug);
     else 
        logger::threadsafe::edglog.open(std::clog, glite::wms::common::logger::debug);

     if( ! options.is_present('j') )
     {
        edglog(error) << "an input file with the jdl must be passed"<< endl;
        return -1;
     }
     else
        req_file.assign(options['j'].getStringValue());

     int count = 1;
     if( options.is_present('n') )  count = options['n'].getIntegerValue();

//     logger::threadsafe::edglog.open(
//           //ns_config->log_file(),
//           std::clog, 
//           static_cast<logger::level_t>(ns_config->log_level()) );
                                                                                                                        
     edglog_fn("Main: catalog access test");
     edglog(debug) << "BEGIN" << endl;
     edglog(debug) << "---------------------------------------------------" << std::endl;

     BrokerInfo<brokerinfoGlueImpl> bi;
    
     ifstream fin(req_file.c_str());    
     if( fin ) 
     { 
        classad::ClassAdParser parser;
    	classad::ClassAd *reqAd = parser.ParseClassAd(fin);

        for (int i=0; i < count; i++) 
   	   bi.retrieveSFNsInfo(*reqAd);
     }
     else edglog(warning) << "cannot open jdl file" << endl;

     edglog(debug) << "---------------------------------------------------" << std::endl;
     edglog(debug) << "END"<< endl;
  
  }
  catch ( LineParsingError &er ) {
    cerr << er << endl;
    exit( er.return_code() );
  }
  catch( CannotConfigure &er ) {
    cerr << er << endl;
  }
  catch( ... ) {
    cout << "Uncaught exception..." << endl;
  }	 
  return 0;

}

#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/ii-purchaser.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/broker/brokerinfo.h"

#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/broker/ResourceBroker.h"
#include "RBMaximizeFilesISMImpl.h"
#include "RBSimpleISMImpl.h"
#include "glite/jdl/JobAdManipulation.h"

#include "matchmaking.h"
#include "exceptions.h"
#include "glite/jdl/ManipulationExceptions.h"

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/utilities/LineParser.h"
#include "glite/wms/common/utilities/LineParserExceptions.h"
#include "glite/wms/common/utilities/scope_guard.h"


#include <glite/wmsutils/classads/classad_utils.h>

#include "glite/wms/classad_plugin/classad_plugin_loader.h"
#include "glite/wms/brokerinfo/brokerinfo.h"

#include <classad_distribution.h>
#include <boost/progress.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "dynamic_library.h"
#include "ism_utils.h"
#include "signal_handling.h"

namespace logger = glite::wms::common::logger;

using namespace std;
using namespace glite;
using namespace glite::wms;
using namespace glite::wms::ism;
using namespace glite::wms::common::utilities;
using namespace glite::wmsutils::classads;
using namespace glite::wms::common::configuration;
using namespace glite::wms::broker;

typedef boost::shared_ptr<glite::wms::manager::server::DynamicLibrary> DynamicLibraryPtr;
typedef boost::shared_ptr<ism::purchaser> PurchaserPtr;

//FIXME: should be moved back to ckassad_plugin if/when only this DL is used
//FIXME: for matchmaking.
glite::wms::classad_plugin::classad_plugin_loader init;


#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

LineOption  options[] = {
    { 'c', 1, "conf_file", "\t use conf_file as configuration file. glite_wms.conf is the default" },
    { 'j', 1, "jdl_file", "\t use jdl_file as input file." },
    { 'r', 1, "repeat", "\t repeat n times the matchmaking process." },
    { 'w', 1, "wait", "\t time to wait before starting the actual MM." },
    { 'C', no_argument, "show-ce-ad", "\t show computing elements classad representation" },
    { 'B', no_argument, "show-brokerinfo-ad", "\t show brokerinfo classad representation" },
    { 'q', no_argument, "quiet", "\t do not log anything" },
    { 'S', no_argument, "show-se-ad", "\t show storage elements classad representation" },
    { 'v', no_argument, "verbose",     "\t be verbose" },
    { 'l', no_argument, "verbose",     "\t be verbose on log file" }
};

void
show_slice_content(MutexSlicePtr const& mt_slice)
{
  Mutex::scoped_lock l(mt_slice->mutex);
  Slice::const_iterator it = mt_slice->slice->begin();
  Slice::const_iterator const e = mt_slice->slice->end();
  for( ; it != e; ++it) cout << *it << endl;
/*
  copy(
    mt_slice->slice->begin(),
    mt_slice->slice->end(),
    ostream_iterator<slice_type::value_type>(cout, "\n")
  );
*/
}

int main(int argc, char* argv[])
{

  std::vector<LineOption> optvec( options, options + sizeof(options)/sizeof(LineOption) );
  LineParser options( optvec, 0 );

  string conf_file;
  size_t time_2_wait;
  string req_file;

  Ism the_ism;
  set_ism(&the_ism);

  try {
     options.parse( argc, argv );
     conf_file.assign( options.is_present('c') ? options['c'].getStringValue() : "glite_wms.conf" );

     Configuration conf(conf_file.c_str(), ModuleType::network_server);

     NSConfiguration const* const ns_config(conf.ns());
     WMConfiguration const* const wm_config(conf.wm());

     if( options.is_present('v') && !options.is_present('l'))   logger::threadsafe::edglog.open(std::clog, glite::wms::common::logger::debug);
                     
     time_2_wait = options.is_present('w') ? options['w'].getIntegerValue() : 20;
     if( options.is_present('l') ) logger::threadsafe::edglog.open(ns_config->log_file(), glite::wms::common::logger::debug);
     else {
        if( options.is_present('q') ) {
          logger::threadsafe::edglog.open(
            "/dev/null",
            glite::wms::common::logger::debug
          );
        } else {
          logger::threadsafe::edglog.open(
            std::clog, 
            glite::wms::common::logger::debug
          );
        }
     }
     
     if( ! options.is_present('j') )
     {
        edglog(error) << "an input file with the jdl must be passed"<< endl;
        return -1;
     }
     else
        req_file.assign(options['j'].getStringValue());

     if (!glite::wms::manager::server::signal_handling()) {
       edglog(error) <<"cannot initialize signal handling"<< std::endl;
       return -1;
     }

     glite::wms::manager::main::ISM_Manager ism_manager;
     sleep(time_2_wait);

    
/*     
    std::string const dll_ii_name("libglite_wms_ism_ii_purchaser.so");
    DynamicLibraryPtr dll_ii(
      new glite::wms::manager::server::DynamicLibrary(
        dll_ii_name,
        glite::wms::manager::server::DynamicLibrary::immediate_binding)
    );

    std::string const create_ii_function_name("create_ii_purchaser");
    purchaser::ii::create_t* create_ii_function;
    dll_ii->lookup(create_ii_function_name, create_ii_function);

    std::string const destroy_ii_function_name("destroy_ii_purchaser");
    purchaser::ii::destroy_t* destroy_ii_function;
    dll_ii->lookup(destroy_ii_function_name, destroy_ii_function);

    PurchaserPtr ii_pch(
      create_ii_function( 
        ns_config->ii_contact(), ns_config->ii_port(),
        ns_config->ii_dn(),ns_config->ii_timeout(), once 
      ),
      destroy_ii_function
    );
    scope_guard clear_ism_ce(
      boost::bind(
        &ism_type::clear, &the_ism[ce]
      )
    );

    scope_guard clear_ism_se(
      boost::bind(
        &ism_type::clear, &the_ism[se]
      )
    );
    ii_pch->do_purchase();
*/
    if(options.is_present('C')){
      for_each(the_ism.computing.begin(), the_ism.computing.end(),
       show_slice_content
      );
    }

    if(options.is_present('S')){
      for_each(the_ism.storage.begin(), the_ism.storage.end(),
       show_slice_content
      );
    }


     edglog(debug) << "-Reading-JDL-------------------------------------------------" << std::endl;

     ifstream fin(req_file.c_str());    
     if( !fin ) { 
        edglog(warning) << "cannot open jdl file" << endl;
        return 0;
     }

     classad::ClassAdParser parser;
     boost::scoped_ptr<classad::ClassAd> reqAd(parser.ParseClassAd(fin));

     try {

        boost::scoped_ptr<classad::ClassAd> result;
      
        std::string vo(jdl::get_virtual_organisation(*reqAd.get()));
      
        bool input_data_exists = false;
        std::vector<std::string> input_data;
        jdl::get_input_data(*reqAd.get(), input_data, input_data_exists);

        glite::wms::broker::ResourceBroker rb;
        if (input_data_exists) {
          rb.changeImplementation( 
            boost::shared_ptr<glite::wms::broker::ResourceBroker::Impl>(
              new glite::wms::broker::RBMaximizeFilesISMImpl()
            )
          );
          edglog(debug) << "-Using-RBMaximizeFiles-implementation------------------------" << std::endl;
        }
        // If fuzzy_rank is true in the request ad we have
        // to use the stochastic selector...
        bool use_fuzzy_rank = false;
        if (jdl::get_fuzzy_rank(*(reqAd.get()), use_fuzzy_rank) && use_fuzzy_rank) {
          rb.changeSelector("stochasticRankSelector");
          edglog(debug) << "-Using-sthochastic-rank-selections---------------------------" << std::endl;
         
        }

        size_t n = options.is_present('r') ? options['r'].getIntegerValue() : 0; 
        size_t matches = n;
        boost::timer t0;
        do {  
          boost::shared_ptr<matchtable> suitable_CEs;
          boost::shared_ptr<filemapping> filemapping;
          boost::shared_ptr<storagemapping> storagemapping;
          boost::tie(suitable_CEs,filemapping,storagemapping) = rb.findSuitableCEs(reqAd.get());
        
          edglog(debug) << " ----- SUITABLE CEs -------" << std::endl;

          matchtable::iterator ces_it = suitable_CEs->begin();
          matchtable::iterator const ces_end = suitable_CEs->end();

          while( ces_it != ces_end ) {
             edglog(debug) << boost::tuples::get<broker::Id>(*ces_it) << std::endl;
             ++ces_it;
          }

          matchtable::const_iterator best_ce_it;
          if ( suitable_CEs->empty() ) {
            std::cerr << "no suitable ce found" << std::endl;      
          }
          else {
            edglog(debug) << ".....trying selectBestCE" << std::endl;
            best_ce_it = rb.selectBestCE(*suitable_CEs);
          }

          edglog(debug) << " ----- BEST CE -------" << std::endl;
          edglog(debug) << boost::tuples::get<broker::Id>(*best_ce_it) << std::endl;
          
          if (options.is_present('B')) {
            edglog(debug) << " ----- BROKEINFO AD -------" << std::endl;
            boost::scoped_ptr<classad::ClassAd> biAd(
              glite::wms::broker::make_brokerinfo_ad(
               filemapping,storagemapping,
               *boost::tuples::get<broker::Ad>(*best_ce_it)
              )
            );
            edglog(debug) << *biAd.get() << std::endl;
          }
       } while(n--);
       edglog(debug) << matches << " matchmaking iterations performed in " << t0.elapsed() << " seconds." << std::endl;
     }
     catch (InformationServiceError const& e) {
     
       std::cerr << "matchmaking::InformationServiceError: "
                 <<e.what() << std::endl;
     
     } catch (RankingError const& e) {
     
       std::cerr << "matchmaking::RankingError: "
                 << e.what() << std::endl;
     
     } catch (jdl::CannotGetAttribute const& e) {
     
       std::cerr << "jdl::CannotGetAttribute: "
                 << e.what() << std::endl; 
     
     } catch (jdl::CannotSetAttribute const& e) {
     
       std::cerr << "jdl::CannotSetAttribute: "
                 << e.what() << std::endl;

     }

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
  catch ( glite::wms::manager::server::CannotLoadDynamicLibrary &ex ) {
    cerr << "CannotLoadDynamicLibrary" << endl;
    cerr << ex.error() << endl;
    exit( -1 );
  }
  catch( glite::wms::manager::server::CannotLookupSymbol &ex ) {
    cerr << "CannotLookupSymbol" << endl;
    cerr << ex.error() << endl;
    exit( -1 );
  }
  catch( ... ) {
    cout << "Uncaught exception..." << endl;
  }	 
  return 0;


}


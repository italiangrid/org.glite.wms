#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/brokerinfo/brokerinfo.h"

#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/broker/ResourceBroker.h"
#include "glite/wms/broker/RBMaximizeFilesISMImpl.h"
#include "glite/wms/broker/RBSimpleISMImpl.h"
#include "glite/jdl/JobAdManipulation.h"

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/matchmaking/exceptions.h"
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

#include <classad_distribution.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "dynamic_library.h"

namespace logger = glite::wms::common::logger;

using namespace std;
using namespace glite;
using namespace glite::wms;
using namespace glite::wms::ism;
using namespace glite::wms::ism::purchaser;
using namespace glite::wms::common::utilities;
using namespace glite::wmsutils::classads;
using namespace glite::wms::common::configuration;

namespace matchmaking = glite::wms::matchmaking;
namespace brokerinfo = glite::wms::brokerinfo;

typedef boost::shared_ptr<glite::wms::manager::server::DynamicLibrary> DynamicLibraryPtr;
typedef boost::shared_ptr<ism_purchaser> PurchaserPtr;

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

LineOption  options[] = {
    { 'c', 1, "conf_file", "\t use conf_file as configuration file. glite_wms.conf is the default" },
    { 'j', 1, "jdl_file", "\t use jdl_file as input file." },
    { 'C', no_argument, "show-ce-ad", "\t show computing elements classad representation" },
    { 'S', no_argument, "show-se-ad", "\t show storage elements classad representation" },
//    { 'b', no_argument, "black-list",  "\t use configuration file black-list." },
    { 'v', no_argument, "verbose",     "\t be verbose" },
    { 'l', no_argument, "verbose",     "\t be verbose on log file" }
};

namespace {
ism_type the_ism[2];
ism_mutex_type the_ism_mutex[2];
}

void print_ism_entry(ism_type::value_type const& e)
{
 classad::ClassAd  ad;
 ad.InsertAttr("id", e.first);
 ad.InsertAttr("update_time", boost::tuples::get<0>(e.second));
 ad.InsertAttr("expiry_time", boost::tuples::get<1>(e.second));
 ad.Insert("info", boost::tuples::get<2>(e.second).get()->Copy());
 edglog(debug) <<  ad << std::endl;
}

int main(int argc, char* argv[])
{

    set_ism(the_ism,the_ism_mutex,ce);
    set_ism(the_ism,the_ism_mutex,se);
/*
    ism_slice_type_ptr bdii_ce_ism_ptr( new ism_slice_type ); // allocate multi_index_container and a pointer
    ism_slice_type_ptr bdii_se_ism_ptr( new ism_slice_type ); // that takes the ownership

    mt_ism_slice_type_ptr bdii_ce_mx_ism ( new mt_ism_slice_type ); //allocate struct that contains
    mt_ism_slice_type_ptr bdii_se_mx_ism ( new mt_ism_slice_type ); //multi_index_container_ptr and
                                                                    //related mutex...and the pointer that takes
                                                                    // the ownership

    bdii_ce_mx_ism->slice = bdii_ce_ism_ptr; //assign the multi_index_container_ptr to struct field
    bdii_se_mx_ism->slice = bdii_se_ism_ptr;

    std::vector<mt_ism_slice_type_ptr> the_ism(ism_boundary.second + 1); // allocate vector of pointers to structs

    the_ism[ce_bdii] = bdii_ce_mx_ism ; //insert the pointer to the struct in the vector
    the_ism[se_bdii] = bdii_se_mx_ism ; //

    set_ism( &the_ism );
*/

  std::vector<LineOption> optvec( options, options + sizeof(options)/sizeof(LineOption) );
  LineParser options( optvec, 0 );

  string conf_file;

  string req_file;

  try {
     options.parse( argc, argv );
     conf_file.assign( options.is_present('c') ? options['c'].getStringValue() : "glite_wms.conf" );

     Configuration conf(conf_file.c_str(), ModuleType::network_server);

     NSConfiguration const* const ns_config(conf.ns());
     WMConfiguration const* const wm_config(conf.wm());

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
        std::mem_fun(&ism_type::clear), &the_ism[ce]
      )
    );

    scope_guard clear_ism_se(
      boost::bind(
        std::mem_fun(&ism_type::clear), &the_ism[se]
      )
    );
    ii_pch->do_purchase();

    if(options.is_present('C'))
    {

        ism_mutex_type::scoped_lock ce_l(get_ism_mutex(glite::wms::ism::ce));
        for_each(
          get_ism(glite::wms::ism::ce).begin(),
          get_ism(glite::wms::ism::ce).end(),
          print_ism_entry
        ); 
/*
         edglog(debug) << "-----CE ISM----------------" << std::endl;
         ism::ism_mutex_type::scoped_lock ce_lock ( the_ism[ism::ce_bdii]->mutex );

         ism_slice_type::nth_index<1>::type& ce_index = (the_ism[ism::ce_bdii]->slice)->get<1>();

         ism_slice_type::nth_index<1>::type::iterator ce_iter = ce_index.begin();

         for ( ; ce_iter != ce_index.end() ; ce_iter++ ) {
                 classad::ClassAd  ad_ism_dump;
                 ad_ism_dump.InsertAttr( "id", boost::tuples::get<4>(*ce_iter) );
                 ad_ism_dump.InsertAttr( "expiry_time", boost::tuples::get<1>(*ce_iter) );
                 ad_ism_dump.Insert( "info",  boost::tuples::get<2>(*ce_iter).get()->Copy() );
                 edglog(debug) << ad_ism_dump <<std::endl;

         }

*/
     }
     if(options.is_present('S'))
     {

        ism_mutex_type::scoped_lock se_l(get_ism_mutex(glite::wms::ism::se));
        for_each(
          get_ism(glite::wms::ism::se).begin(),
          get_ism(glite::wms::ism::se).end(),
          print_ism_entry
        ); 
/*
         edglog(debug) << "-----SE ISM----------------" << std::endl;
         ism::ism_mutex_type::scoped_lock se_lock ( the_ism[ism::se_bdii]->mutex );

         ism_slice_type::nth_index<1>::type& se_index = (the_ism[ism::se_bdii]->slice)->get<
1>();

         ism_slice_type::nth_index<1>::type::iterator se_iter = se_index.begin();

         for ( ; se_iter != se_index.end() ; se_iter++ ) {
                 classad::ClassAd  ad_ism_dump;
                 ad_ism_dump.InsertAttr( "id", boost::tuples::get<4>(*se_iter) );
                 ad_ism_dump.InsertAttr( "expiry_time", boost::tuples::get<1>(*se_iter) );
                 ad_ism_dump.Insert( "info",  boost::tuples::get<2>(*se_iter).get()->Copy()
);
                 edglog(debug) << ad_ism_dump <<std::endl;

         }

*/
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
        boost::tuple<
          boost::shared_ptr<glite::wms::matchmaking::matchtable>,
          boost::shared_ptr<glite::wms::brokerinfo::filemapping>,
          boost::shared_ptr<glite::wms::brokerinfo::storagemapping>
        > brokering_result(
          rb.findSuitableCEs(reqAd.get())
        );
        boost::shared_ptr<glite::wms::matchmaking::matchtable>& suitable_CEs(
          boost::tuples::get<0>(brokering_result)
        );

        edglog(debug) << " ----- SUITABLE CEs -------" << std::endl;

        glite::wms::matchmaking::matchtable::iterator ces_it = suitable_CEs->begin();
        glite::wms::matchmaking::matchtable::iterator const ces_end = suitable_CEs->end();

        while( ces_it != ces_end ) {
           edglog(debug) <<ces_it->first << std::endl;
           ++ces_it;
        }

        glite::wms::matchmaking::matchtable::const_iterator best_ce_it;
        if ( suitable_CEs->empty() ) {
          std::cerr << "no suitable ce found" << std::endl;      
        }
        else {
          edglog(debug) << ".....trying selectBestCE" << std::endl;
          best_ce_it = rb.selectBestCE(*suitable_CEs);
        }

        edglog(debug) << " ----- BEST CE -------" << std::endl;
        edglog(debug) << best_ce_it->first << std::endl;

     }
     catch (glite::wms::matchmaking::InformationServiceError const& e) {
     
       std::cerr << "matchmaking::InformationServiceError: "
                 <<e.what() << std::endl;
     
     } catch (glite::wms::matchmaking::RankingError const& e) {
     
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


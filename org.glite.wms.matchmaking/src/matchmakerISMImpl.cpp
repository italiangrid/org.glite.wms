// File: matchmakerISMImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/ii_attr_utils.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "glite/wms/ism/ism.h"
#include "matchmakerISMImpl.h"
#include "glue_attributes.h"
#include "jdl_attributes.h"
#include "exceptions.h"
#include "glite/wms/classad_plugin/classad_plugin_loader.h"
#include "classad_distribution.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);
using namespace std;

namespace glite {
namespace wms {
  
namespace configuration = common::configuration;
namespace utilities     = common::utilities;
namespace logger        = common::logger;

namespace matchmaking { 
namespace 
{

  struct insertMatchingRegEx : binary_function<vector<string>*, string, vector<string>*>
  {
    
    insertMatchingRegEx(const string& p) 
    {
      expression.reset( new boost::regex(p) );
    }
    
    vector<string>* operator()(vector<string>* v, string a)
    {
      try {
  
  if( boost::regex_match(a, *expression) ) {
    
    v -> push_back(a);
  }
      }
      catch( boost::bad_expression& e ){

  edglog(fatal) << "Kaboomm" << endl;
      }    
      return v;
    }
    boost::shared_ptr<boost::regex> expression;
  };

  struct BadCEIdFormatEx
  {
  };

} //anonymous namespace

boost::scoped_ptr< classad::ClassAd > matchmakerISMImpl::gang_match_storageAd;
  
matchmakerISMImpl::matchmakerISMImpl()
{
}

matchmakerISMImpl::~matchmakerISMImpl()
{
}
void matchmakerISMImpl::prefetchCEInfo(const classad::ClassAd* requestAd, match_table_t& suitableCEs) 
{
}    
/**
 * Check requirements.
 * This method fills suitableCEs vector with CEs satisfying requirements as expressed in the requestAd.
 * @param requestAd
 * @param suitableCEs
 */
void matchmakerISMImpl::checkRequirement(const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces)
{
  edglog_fn(checkRequirement);       

  boost::mutex::scoped_lock l(ism::get_ism_mutex());
  
  for (ism::ism_type::const_iterator it = ism::get_ism().begin(); it != ism::get_ism().end(); it++) {

    boost::shared_ptr<classad::ClassAd> ceAd = boost::tuples::get<1>((*it).second);
  
    // Cadidates CE's are those where the user is authorized to submit...
    // ...we should check for authorization first...
    classad::ExprTree *expr;
    string requirement;
    requirement.assign( "AuthorizationCheck" );
    classad::ClassAdParser parser;
    parser.ParseExpression(requirement, expr);
    ceAd -> Insert("requirements", expr);
    bool request_is_authorized = utilities::right_matches_left(*ceAd, *requestAd);
  
    if (request_is_authorized) {
  
      // if the requirements expression of the request ad contains
      // gang-match functions we have to load the plugin library
      // it could be possible to check whether the requirements 
      // contains a function call to such function and then load
      // the plugin library... in order to avoid the overhead due 
      // to such a search we prefer to load the library always.
    
      // gang-match requires a nested classad within the CEad containing
      // call to extended functions which allow to acquire all the required
      // information...since this classad does not change it is possible
      // we will parse it just once...
    
      if (!gang_match_storageAd) {
     
        string adstr("[CEid = parent.GlueCEUniqueID; \
    VO = parent.other.VirtualOrganisation; \
    additionalSESAInfo = listAttrRegEx(\"^GlueS[EA].*\", parent.other.requirements); \
    CloseSEs = retrieveCloseSEsInfo( CEid, VO, additionalSESAInfo ); ]");
        gang_match_storageAd.reset( utilities::parse_classad(adstr) );            
      } 

      ceAd->Insert("storage", gang_match_storageAd->Copy());  
    
      //
      // Construct the CE's requirement expression as follows:
      // requirements = member(GlueCEUniqueID, other.edg_wm_ces_to_exclude) == false
    
      bool CEad_matches_requestAd = false;
    
      if (requestAd -> Lookup("edg_previous_matches")) {
        
        classad::ExprTree *expr;
        string CErequirement;
        CErequirement.assign( "member(GlueCEUniqueId, other.edg_previous_matches) == false" );
      
        classad::ClassAdParser parser;
        parser.ParseExpression(CErequirement, expr);
        ceAd -> Insert("requirements", expr);
        CEad_matches_requestAd = utilities::symmetric_match(*ceAd, *requestAd);

        if (!CEad_matches_requestAd) {
  
    // The match with the requirement to remove previous matches
          // failed, but if we get here, we still haven't exhausted
          // the job RetryCount, so let's ignore the previous matches,
          // and apply the normal ranking.
    ceAd -> Delete("requirements");
    CEad_matches_requestAd = utilities::left_matches_right(*ceAd,*requestAd);
        }
      }  
      else {
  
        CEad_matches_requestAd = utilities::left_matches_right(*ceAd, *requestAd);
      }
      if (CEad_matches_requestAd) {

        edglog( info ) << it->first << ", Ok!" << endl << flush;
  suitableCEs[ it->first ] = ceAd;
      }
    } 
  }
}

/**
 * Checks the rank of CE in suitableCEs vector.
 * @param context a pointer to the matchmaking context.
 */
void matchmakerISMImpl::checkRank(const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces)
{
  edglog_fn(checkRank);
  bool unable_to_rank_all = true;
  boost::mutex::scoped_lock l(ism::get_ism_mutex());
  for(match_table_t::iterator mit = suitableCEs.begin(); mit != suitableCEs.end(); mit++) {
  
    string CEid( mit -> first );
    ism::ism_type::const_iterator ism_entry = ism::get_ism().find( CEid );
    if( ism_entry != ism::get_ism().end() ) {
      
      boost::shared_ptr<classad::ClassAd> ceAd = boost::tuples::get<1>(ism_entry->second);
      try {
        mit -> second.setRank( utilities::right_rank(*ceAd, *requestAd) );
        unable_to_rank_all = false;
      }
        catch( utilities::UndefinedRank& ) {
      
        edglog( severe ) << "Unexpected result while ranking " << 
      mit -> first << " rank does not evaluate to number..." << endl;
      }
    }
    else {
      
      edglog( severe ) << "ISM search failed for: " << mit -> first << endl;
    }
  }
  if(unable_to_rank_all) throw matchmaking::RankingError();
}



} // namespace matchmaking
} // namespace wms
} // namespace glite

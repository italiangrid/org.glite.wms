// File: matchmakerISMImpl.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <algorithm>
#include <boost/shared_ptr.hpp>

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wmsutils/classads/classad_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/jdl/PrivateAdManipulation.h"
#include "glite/wms/ism/ism.h"
#include "matchmakerISMImpl.h"
#include "exceptions.h"

namespace jdlc = glite::jdl;

namespace glite {
namespace utils = wmsutils::classads;
namespace wms {
namespace matchmaking {

void
matchmakerISMImpl::checkRequirement(
  classad::ClassAd& jdl,
  matchtable& suitableCEs
)
{
  ism::ism_mutex_type::scoped_lock l(ism::get_ism_mutex(ism::ce));
  ism::ism_type::const_iterator const ism_end(
    ism::get_ism(ism::ce).end()
  );
  ism::ism_type::const_iterator ism_it(
    ism::get_ism(ism::ce).begin()
  );
  for ( ; ism_it != ism_end; ++ism_it) {

    if (ism::is_void_ism_entry(ism_it->second)) {
      continue;
    }

    boost::shared_ptr<classad::ClassAd> ce_ad_ptr(
      boost::tuples::get<2>(ism_it->second)
    );
    classad::ClassAd ce_ad(*ce_ad_ptr);
    std::string const ism_id(ism_it->first);

    if (utils::symmetric_match(ce_ad, jdl)) {
      Info(ism_id << ": ok!");
      suitableCEs[ism_id] = boost::tuples::make_tuple(
        std::make_pair(false, 0.0),
        ce_ad_ptr
      );
    }
  }
  
  typedef std::vector<std::string> previous_matches_type;
  previous_matches_type previous_matches;
  
  bool previous_matches_exists = false;
  jdlc::get_edg_previous_matches(
    jdl, 
    previous_matches, 
    previous_matches_exists
  );
  
  if (previous_matches_exists) {
    previous_matches_type::iterator const 
    previous_matches_begin(
      previous_matches.begin()
    );
    previous_matches_type::iterator const 
    previous_matches_end(
      previous_matches.end()
    );
    matchtable::iterator ces_it = suitableCEs.begin();
    matchtable::iterator const ces_end = suitableCEs.end();
    while( ces_it != ces_end ) {
      std::string const& ce_id = ces_it->first;
      if (
        find(
          previous_matches_begin,
          previous_matches_end, 
          ce_id) != previous_matches_end) {
  
        suitableCEs.erase(ces_it++);
      }
      else {
        ++ces_it;
      }
    }
  }
}

void
matchmakerISMImpl::checkRequirement(
  classad::ClassAd& jdl,
  std::set<matchtable::key_type> const& ce_ids,
  matchtable& suitableCEs
)
{
  ism::ism_mutex_type::scoped_lock l(ism::get_ism_mutex(ism::ce));

  ism::ism_type::const_reverse_iterator const ism_rend(
    ism::get_ism(ism::ce).rend()
  );
  ism::ism_type::const_iterator const ism_end(
    ism::get_ism(ism::ce).end()
  );

  std::set<matchtable::key_type>::const_iterator ce_ids_it = ce_ids.begin();
  std::set<matchtable::key_type>::const_iterator const ce_ids_end = ce_ids.end();
  
  for( ; ce_ids_it != ce_ids_end ; ++ce_ids_it ) {
      
    ism::ism_type::const_iterator ism_it(
      std::find_if(
        ism::get_ism(ism::ce).begin(),
        ism::get_ism(ism::ce).end(),
        ism::key_starts_with(*ce_ids_it)
      )
    );

    if(ism_it == ism_end) continue;

    ism::ism_type::const_reverse_iterator ism_r_it(
      std::find_if(
        ism::get_ism(ism::ce).rbegin(),
        ism::get_ism(ism::ce).rend(),
        ism::key_starts_with(*ce_ids_it)
      )
    );

    if(ism_r_it == ism_rend) continue;
      
    do {

      boost::shared_ptr<classad::ClassAd> ce_ad_ptr(
        boost::tuples::get<2>(ism_it->second)
      );
      classad::ClassAd ce_ad(*ce_ad_ptr);
      if (utils::symmetric_match(ce_ad, jdl)) {
        
        std::string const ism_id(ism_it->first);
        
        Info(ism_id << ": ok!");
        suitableCEs[ism_id] = boost::tuples::make_tuple(
          std::make_pair(false, 0.0),
          ce_ad_ptr
        );
      }
    }
    while(++ism_it != ism_r_it.base());
  }

  typedef std::vector<std::string> previous_matches_type;
  previous_matches_type previous_matches;
  
  bool previous_matches_exists = false;
  jdlc::get_edg_previous_matches(
    jdl, 
    previous_matches, 
    previous_matches_exists
  );
  
  if (previous_matches_exists) {
    previous_matches_type::iterator const 
    previous_matches_begin(
      previous_matches.begin()
    );
    previous_matches_type::iterator const 
    previous_matches_end(
      previous_matches.end()
    );
    matchtable::iterator ces_it = suitableCEs.begin();
    matchtable::iterator const ces_end = suitableCEs.end();
    while( ces_it != ces_end ) {
      std::string const& ce_id = ces_it->first;
      if (
        find(
          previous_matches_begin,
          previous_matches_end, 
          ce_id) != previous_matches_end) {
  
        suitableCEs.erase(ces_it++);
      }
      else {
        ++ces_it;
      }
    }
  }
}
 
void
matchmakerISMImpl::checkRank(
  classad::ClassAd& jdl,
  matchtable& suitableCEs
)
{
  bool unable_to_rank_all = true;

  matchtable::iterator ces_it = suitableCEs.begin();
  matchtable::iterator const ces_end = suitableCEs.end();
  for ( ; ces_it != ces_end; ++ces_it) {

    std::string const& ce_id = ces_it->first;
    classad::ClassAd ce_ad(*getAd(ces_it->second).get());

    try {
      setRank(ces_it->second,utils::right_rank(ce_ad, jdl));
      unable_to_rank_all = false;
    } catch (utils::UndefinedRank&) {
      Error("Unexpected result while ranking " << ce_id);
    }
  }

  if (unable_to_rank_all) {
    throw matchmaking::RankingError();
  }
}

}}}

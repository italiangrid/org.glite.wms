// File: matchmakerISMImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/shared_ptr.hpp>
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/ism/ism.h"
#include "matchmakerISMImpl.h"
#include "exceptions.h"

namespace glite {
namespace wms {
  
namespace utilities     = common::utilities;

namespace matchmaking { 

void 
matchmakerISMImpl::prefetchCEInfo(
  const classad::ClassAd* requestAd,
  match_table_t& suitableCEs
) 
{
}    

/**
 * Check requirements.
 * This method fills suitableCEs vector with CEs satisfying requirements as expressed in the requestAd.
 * @param requestAd
 * @param suitableCEs
 */
void
matchmakerISMImpl::checkRequirement(
  classad::ClassAd& jdl,
  match_table_t& suitableCEs,
  bool use_prefetched_ces
)
{
  ism::ism_mutex_type::scoped_lock l(ism::get_ism_mutex());

  ism::ism_type::const_iterator ism_it = ism::get_ism().begin();
  ism::ism_type::const_iterator const ism_end = ism::get_ism().end();
  for ( ; ism_it != ism_end; ++ism_it) {

    if (ism::is_void_ism_entry(ism_it->second)) {
      continue;
    }

    std::string const ce_id(ism_it->first);
    boost::shared_ptr<classad::ClassAd> ce_ad_ptr(
      boost::tuples::get<2>(ism_it->second)
    );
    classad::ClassAd ce_ad(*ce_ad_ptr);

    if (utilities::symmetric_match(ce_ad, jdl)) {
      Info(ce_id << ": ok!");
      suitableCEs[ce_id] = ce_ad_ptr;
    }
  }

  typedef std::set<std::string> previous_matches_type;
  previous_matches_type previous_matches;
  previous_matches_type::iterator const previous_matches_end(
    previous_matches.end()
  );
  std::vector<match_table_t::iterator> v;
  match_table_t::iterator ces_it = suitableCEs.begin();
  match_table_t::iterator const ces_end = suitableCEs.end();
  for ( ; ces_it != ces_end; ++ces_it) {
    std::string const ce_id = ces_it->first;
    if (previous_matches.find(ce_id) == previous_matches_end) {
      v.push_back(ces_it);
    }
  }
  if (v.size() != suitableCEs.size()) {
    std::vector<match_table_t::iterator>::iterator v_it = v.begin();
    std::vector<match_table_t::iterator>::iterator const v_end = v.end();
    for ( ; v_it != v_end; ++v_it) {
      suitableCEs.erase(*v_it);
    }
  }
}

/**
 * Checks the rank of CE in suitableCEs vector.
 * @param context a pointer to the matchmaking context.
 */
void
matchmakerISMImpl::checkRank(
  classad::ClassAd& jdl,
  match_table_t& suitableCEs,
  bool use_prefetched_ces
)
{
  bool unable_to_rank_all = true;

  match_table_t::iterator ces_it = suitableCEs.begin();
  match_table_t::iterator const ces_end = suitableCEs.end();
  for ( ; ces_it != ces_end; ++ces_it) {

    std::string const ce_id = ces_it->first;
    classad::ClassAd ce_ad(*ces_it->second.getAd());

    try {
      ces_it->second.setRank(utilities::right_rank(ce_ad, jdl));
      unable_to_rank_all = false;
    } catch (utilities::UndefinedRank&) {
      Error("Unexpected result while ranking " << ce_id);
    }
  }

  if (unable_to_rank_all) {
    throw matchmaking::RankingError();
  }
}

} // namespace matchmaking
} // namespace wms
} // namespace glite

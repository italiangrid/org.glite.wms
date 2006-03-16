// File: matchmakerISMImpl.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/shared_ptr.hpp>
#include <algorithm>
#include "glite/wmsutils/classads/classad_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/jdl/PrivateAdManipulation.h"
#include "glite/wms/ism/ism.h"
#include "matchmakerISMImpl.h"
#include "exceptions.h"

namespace glite {

namespace utils = wmsutils::classads;
namespace jdl = jdl;

namespace wms {

namespace utilities = common::utilities;

namespace matchmaking {

void
matchmakerISMImpl::prefetchCEInfo(
  classad::ClassAd const* requestAd,
  match_table_t& suitableCEs
)
{
}    

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

    boost::shared_ptr<classad::ClassAd> ce_ad_ptr(
      boost::tuples::get<2>(ism_it->second)
    );
    classad::ClassAd ce_ad(*ce_ad_ptr);

    std::string const ce_id(
      utils::evaluate_attribute(ce_ad, "GlueCEUniqueID")
    );

    if (utils::symmetric_match(ce_ad, jdl)) {
      Info(ce_id << ": ok!");
      suitableCEs[ce_id] = ce_ad_ptr;
    }
  }

  typedef std::vector<std::string> previous_matches_type;
  previous_matches_type previous_matches;
  jdl::get_edg_previous_matches(jdl, previous_matches);
  previous_matches_type::iterator const previous_matches_begin(
    previous_matches.begin()
  );
  previous_matches_type::iterator const previous_matches_end(
    previous_matches.end()
  );
  match_table_t::iterator ces_it = suitableCEs.begin();
  match_table_t::iterator const ces_end = suitableCEs.end();
  while( ces_it != ces_end ) {
    std::string const& ce_id = ces_it->first;
    if (find(previous_matches_begin,
             previous_matches_end, ce_id) != previous_matches_end) {
      suitableCEs.erase(ces_it++);
    }
    else {
      ++ces_it;
    }
  }
}

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
      ces_it->second.setRank(utils::right_rank(ce_ad, jdl));
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

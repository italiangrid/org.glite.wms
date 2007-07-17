// File: match.cpp
// Author: Francesco Giacomini
// Author: Salvatore Monforte

// $Id$

#include "glite/wms/broker/match.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/timer.hpp>

#include "storage_utils.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wmsutils/classads/classad_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/jdl/PrivateAdManipulation.h"
#include "glite/jdl/JobAdManipulation.h"
#include "glite/wms/ism/ism.h"
#include "exceptions.h"

namespace jdl = glite::jdl;

namespace glite {

namespace classad_utils = wmsutils::classads;
namespace utils = wms::common::utilities;

namespace wms {
namespace broker {

namespace {

bool
match_and_rank(
  classad::ClassAd* lhs, 
  classad::ClassAd* rhs,
  double& rank_result
)
{
  classad::MatchClassAd match_ad(lhs, rhs);

  bool match_result = false;
  match_ad.EvaluateAttrBool("symmetricMatch", match_result);
  utils::scope_guard remove_left_ad(
    boost::bind(&classad::MatchClassAd::RemoveLeftAd, boost::ref(match_ad))
  );
  utils::scope_guard remove_right_ad(
    boost::bind(&classad::MatchClassAd::RemoveRightAd, boost::ref(match_ad))
  );

  if (match_result)
  try {
    rank_result =
      classad_utils::evaluate_attribute(match_ad, "rightRankValue");
  } catch (classad_utils::InvalidValue&) {
    throw classad_utils::UndefinedRank();
  }
  return match_result;
}

class match_slice_content
{
  classad::ClassAd* jdl_ptr;

public:
  match_slice_content(classad::ClassAd& j)
  : jdl_ptr(&j) 
  {
  }
  MatchTable*
  operator()(MatchTable* matches, ism::MutexSlicePtr mt_slice) const
  {
    bool has_jobid = false;
    std::string const job_id(jdl::get_edg_jobid(*jdl_ptr, has_jobid));
    boost::timer t;
    ism::Mutex::scoped_lock l(mt_slice->mutex);
    double t_lock = t.elapsed();
    
    ism::Slice::const_iterator it = mt_slice->slice->begin();
    ism::Slice::const_iterator const e = mt_slice->slice->end();

    size_t const n = std::distance(it, e);

    for( ; it != e; ++it) {

      if (ism::is_entry_void(*it)) continue;

      boost::shared_ptr<classad::ClassAd> ce_ad_ptr =
        boost::tuples::get<ism::Ad>(*it);

      classad::ClassAd& ce_ad(*ce_ad_ptr);

      std::string const& ism_id(boost::tuples::get<ism::Id>(*it));

      try {
        double rank;
        if (match_and_rank(&ce_ad, jdl_ptr, rank)) {
          matches->push_back(
            MatchInfo(ism_id, rank, ce_ad_ptr)
          );
        }
      } catch (classad_utils::UndefinedRank&) {
        Error("Unexpected result while ranking " << ism_id);
      }
    }
    l.unlock();

    if (has_jobid) {
      Info(
        "MM for job: " << job_id << " (" << matches->size() << '/' << n <<
        " [" << t_lock << ", " << t.elapsed() << "] )"
      );
    } else {
      Info(
        "MM for listmatch (" << matches->size() << '/' << n <<
        " [" << t_lock << ", " << t.elapsed() << "] )"
      );
    }

    return matches;
  }
};

class match_slice_content_if
{
  classad::ClassAd* jdl_ptr;
  boost::function< bool(ism::SliceEntry const&) > pred;

public:
  match_slice_content_if(
    classad::ClassAd& j, 
    boost::function< bool(ism::SliceEntry const&)> p
  )
  : jdl_ptr(&j), pred(p)
  {
  }
  MatchTable*
  operator()(MatchTable* matches, ism::MutexSlicePtr mt_slice) const
  {
    ism::Mutex::scoped_lock l(mt_slice->mutex);
    ism::Slice::const_iterator it = mt_slice->slice->begin();
    ism::Slice::const_iterator const e = mt_slice->slice->end();

    for( ; it != e; ++it) {

      if (ism::is_entry_void(*it)) continue;
      if (!pred(*it)) continue;

      boost::shared_ptr<classad::ClassAd> ce_ad_ptr =
        boost::tuples::get<ism::Ad>(*it);

      classad::ClassAd ce_ad(*ce_ad_ptr);

      std::string const& ism_id(boost::tuples::get<ism::Id>(*it));

      try {
        double rank;
        if (match_and_rank(&ce_ad, jdl_ptr, rank)) {
          Info(ism_id << ": ok!");
          matches->push_back(
            MatchInfo( ism_id, rank, ce_ad_ptr )
          );
        }
      } catch (classad_utils::UndefinedRank&) {
        Error("Unexpected result while ranking " << ism_id);
      }
    }
    return matches;
  }
};

struct in 
{
  std::vector<std::string>::const_iterator m_begin;
  std::vector<std::string>::const_iterator const m_end;

  in(std::vector<std::string> const& pm)
   : m_begin(pm.begin()), m_end(pm.end()) 
  {
  }
  bool operator()(MatchInfo const& i)
  {
    std::string const s(
      classad_utils::evaluate_attribute(*(i.ce_ad), "GlueCEUniqueID")
    );
    return std::find(m_begin, m_end, s) != m_end;
  }
};

} // anonymous namespace

MatchTable
match(
  classad::ClassAd& jdl,
  std::vector<std::string> const& skipping_ces
)
{
  ism::Ism const& the_ism = *ism::get_ism();

  MatchTable matches;

  std::accumulate(
    the_ism.computing.begin(),
    the_ism.computing.end(),
    &matches,
    match_slice_content(jdl)
  );
  
  if (!skipping_ces.empty()) {
    MatchTable::iterator _(      
    std::remove_if(
        matches.begin(), matches.end(), in(skipping_ces)
      )
    );
    matches.erase(_, matches.end());
  }
  return matches;
}

MatchTable
match(
  classad::ClassAd& jdl,
  std::vector<std::string> const& skipping_ces,
  std::set<std::string> const& ce_ids
)
{
  std::set<std::string>::const_iterator ce_ids_it = ce_ids.begin();
  std::set<std::string>::const_iterator const ce_ids_end(
    ce_ids.end()
  );

  ism::Ism const& the_ism = *ism::get_ism();
  MatchTable matches;

  // TODO: change the loop using a new predicate for
  // the match_slice_content_if which look in the ordered container
  for( ; ce_ids_it != ce_ids_end ; ++ce_ids_it ) {
  
    accumulate(
      the_ism.computing.begin(),
      the_ism.computing.end(),
      &matches,
      match_slice_content_if(jdl, ism::KeyStartsWith(*ce_ids_it))
    );
  }
  
  if (!skipping_ces.empty()) {
    MatchTable::iterator _(
    std::remove_if(
        matches.begin(), matches.end(), in(skipping_ces)
      )
    );
    matches.erase(_, matches.end());
  }
  return matches;
}

MatchTable
match(
  classad::ClassAd& jdl,
  std::vector<std::string> const& skipping_ces,
  DataInfo& data_info

) 
{
  // Collects SFNs and involved SEs.
  data_info.fm = resolve_filemapping_info(jdl);
  data_info.sm = resolve_storagemapping_info(data_info.fm);

  // Selects only comptatible storage
  std::vector<std::string> dap;
  jdl::get_data_access_protocol(jdl, dap);
  std::vector<StorageMapping::const_iterator> compatible_storage(
    select_compatible_storage(data_info.sm,dap)
  );

  // std::set required in order to remove duplicates 
  std::set<std::string> close_computing_elements_id;
  // selects id of those computing elements bound to 
  // compatible storage 
  std::accumulate(
    compatible_storage.begin(), 
    compatible_storage.end(),
    &close_computing_elements_id,
    extract_computing_elements_id()
  );

  // Filter computing elements on requirements
  MatchTable matches(
    match(jdl, skipping_ces, close_computing_elements_id)
  );

  MatchTable::iterator ce_it = matches.begin();
  MatchTable::iterator const ce_end = matches.end();
  
  std::map<
    size_t, 
    std::vector<MatchTable::const_iterator> 
  > unique_logical_files_per_ce;

  size_t max_files = 0;
  for( ; ce_it != ce_end; ) {
 
    std::string const& id = ce_it->ce_id;
    std::vector<StorageMapping::const_iterator>::iterator
    storage_part_end(
      std::partition(
        compatible_storage.begin(),
        compatible_storage.end(),
        is_storage_close_to(id)
    ));

    size_t n = count_unique_logical_files(
      compatible_storage.begin(), storage_part_end
    );

    Debug(
      id << " has #" << 
      storage_part_end - compatible_storage.begin() << 
      " close compatible storage element(s) providing #" << n <<
      " accessible file(s)"
    );
    
    if ( n > max_files ) max_files = n;
      
    std::map< 
      size_t, std::vector<MatchTable::const_iterator>
    >::iterator it;
    
    bool inserted = false;
    boost::tie(it,inserted) = unique_logical_files_per_ce.insert(
      std::make_pair(n,std::vector<MatchTable::const_iterator>())
    );
    
    it->second.push_back(ce_it);
    ++ce_it;
  }
  // If full list match result is not requested we should return
  // only ces accessing the max number of logical files
  bool FullListMatchResult = false;
  if ( !jdl.EvaluateAttrBool("FullListMatchResult", FullListMatchResult) ||
    !FullListMatchResult 
  ) {
    std::vector<MatchTable::const_iterator> const& max_logical_files_ces(
      unique_logical_files_per_ce[max_files]
    );
    
    MatchTable::iterator it = matches.begin();
    MatchTable::iterator const e = matches.end();
    for( ; it != e ; ) {
      if( std::find(
        max_logical_files_ces.begin(),
        max_logical_files_ces.end(),
      it) == max_logical_files_ces.end()) {
        matches.erase(it++);
      }
      else {
        ++it;
      }
    }
  }
  return matches;
}

}}}

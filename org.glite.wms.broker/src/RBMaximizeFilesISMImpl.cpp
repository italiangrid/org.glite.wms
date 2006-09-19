// File: RBMaximizeFilesISMImpl.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector> 

#include <classad_distribution.h>

#include "glite/jdl/ManipulationExceptions.h"
#include "glite/jdl/JobAdManipulation.h"

#include "glite/wms/matchmaking/matchmakerISMImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "RBMaximizeFilesISMImpl.h"
#include "utility.h"

#include "glite/wms/common/logger/logger_utils.h"

using namespace std;

namespace requestad = glite::jdl;

namespace glite {
namespace wms {

namespace logger        = common::logger;

namespace broker {

namespace brokerinfo  = wms::brokerinfo;
namespace matchmaking = wms::matchmaking;

boost::tuple<
  boost::shared_ptr<matchmaking::matchtable>,
  boost::shared_ptr<brokerinfo::filemapping>,
  boost::shared_ptr<brokerinfo::storagemapping>
>
RBMaximizeFilesISMImpl::findSuitableCEs(
  const classad::ClassAd* requestAd
) {

  boost::shared_ptr<matchmaking::matchtable> suitableCEs;
  boost::shared_ptr<brokerinfo::filemapping> fm;
  boost::shared_ptr<brokerinfo::storagemapping> sm;

  if (requestAd) {

    suitableCEs.reset(new matchmaking::matchtable);
    classad::ClassAd jdl(*requestAd);

    // Collects SFNs and involved SEs.
    boost::shared_ptr<brokerinfo::filemapping> fm(
      brokerinfo::resolve_filemapping_info(jdl)
    );
    boost::shared_ptr<brokerinfo::storagemapping> sm(
      brokerinfo::resolve_storagemapping_info(fm)
    );

    // Selects only comptatible storage
    vector<string> dap;
    requestad::get_data_access_protocol(jdl, dap);
    vector<brokerinfo::storagemapping::const_iterator> compatible_storage(
      brokerinfo::select_compatible_storage(*sm,dap)
    );

    std::set<std::string> close_computing_elements_id;
    // Inserts in suitableCEs id of those computing elements
    // bound to compatible storage 
    std::accumulate(
      compatible_storage.begin(), 
      compatible_storage.end(),
      &close_computing_elements_id,
      extract_computing_elements_id()
    );     
    
    // Filter computing elements on requirements/rank
    matchmaking::MatchMaker MM;

    MM.checkRequirement(
      jdl,
      close_computing_elements_id, 
      *suitableCEs);

    MM.checkRank(jdl, *suitableCEs);

    matchmaking::matchtable::iterator ce_it = suitableCEs->begin();
    matchmaking::matchtable::iterator const ce_end = suitableCEs->end();
    
    map<
      size_t, 
      vector<matchmaking::matchtable::const_iterator> 
    > unique_logical_files_per_ce;
    
    size_t max_files = 0;
    for( ; ce_it != ce_end; ) {
 
      // Remove invalid matchinfo
      if(matchmaking::isRankUndefined(ce_it->second)) {
        suitableCEs->erase(ce_it++);
        continue;
      }
      vector<brokerinfo::storagemapping::const_iterator>::iterator
      storage_part_end(
        std::partition(
          compatible_storage.begin(),
          compatible_storage.end(),
          is_storage_close_to(ce_it->first)
        )
      );
      
      size_t n =
        brokerinfo::count_unique_logical_files(
          compatible_storage.begin(),
          storage_part_end
      );

      Debug(
        ce_it->first << " has #" << 
        storage_part_end - compatible_storage.begin() << 
        " close compatible storage element(s) providing #" << n <<
        " accessible file(s)"
      );

      if ( n > max_files ) max_files = n;
      
      map<
        size_t, 
        vector<matchmaking::matchtable::const_iterator> 
      >::iterator it;
      bool inserted = false;
      boost::tie(it,inserted) =
        unique_logical_files_per_ce.insert(
          std::make_pair(n,vector<matchmaking::matchtable::const_iterator>())
        );
      it->second.push_back(ce_it);
      ++ce_it;
    }
	
    bool FullListMatchResult = false;
    if (
      !jdl.EvaluateAttrBool("FullListMatchResult", FullListMatchResult) || 
      !FullListMatchResult) {

      vector<matchmaking::matchtable::const_iterator> const& max_logical_files_ces(
         unique_logical_files_per_ce[max_files]
      );
      matchmaking::matchtable::iterator it = suitableCEs->begin();
      matchmaking::matchtable::iterator const e = suitableCEs->end();
      for( ; it != e ; ) {
        if( std::find(
             max_logical_files_ces.begin(),
             max_logical_files_ces.end(),
             it) == max_logical_files_ces.end()) {

          suitableCEs->erase(it++);
        }
        else {
          ++it;
        }
      }
    }  
  }
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite


// File: RBMaximizeFilesISMImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector> 

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

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

using namespace brokerinfo;
using namespace matchmaking;

namespace broker {

RBMaximizeFilesISMImpl::RBMaximizeFilesISMImpl(
  BrokerInfo<brokerinfoISMImpl> *bi, 
  bool do_prefetch)
{
  BI = bi;
  m_prefetch = do_prefetch;
}
 
RBMaximizeFilesISMImpl::~RBMaximizeFilesISMImpl()
{
}

matchmaking::match_table_t* 
RBMaximizeFilesISMImpl::findSuitableCEs(
  const classad::ClassAd* requestAd
) {
  matchmaking::match_table_t* suitableCEs = 0;
  if (requestAd) {
    matchmaking::MatchMaker<matchmaking::matchmakerISMImpl> MM;
    suitableCEs = new matchmaking::match_table_t;
    classad::ClassAd jdl(*requestAd);
    MM.checkRequirement(jdl, *suitableCEs, m_prefetch);

    // Now we have to filter this CEs to check whether they 
    // satisfy data requirements, or not.
	
    // Instantiates a BI object and collects all the information
    // about SFNs and involved SEs.
    // NOTE: this information is not CE dependent.
    BI->retrieveSFNsInfo(jdl);
    BI->retrieveSEsInfo (jdl);
	
    std::vector<std::string> deletingCEs;
    std::vector<std::string> data_access_protocols;
    requestad::get_data_access_protocol(jdl, data_access_protocols);
    map<size_t, set<string> > nFiles2CEs;
    size_t max_files = 0;
	
    match_const_iterator ce = suitableCEs->begin();
    match_const_iterator const ce_end = suitableCEs->end();

    for(;ce != ce_end; ++ce) {
		
      BI->retrieveCloseSEsInfo(ce->first);
      BrokerInfoData::SE_container_type compatibleCloseSEs( 
        (*BI)->getCompatibleCloseSEs(data_access_protocols) 
      );
      if (compatibleCloseSEs.empty()) {
		        
        Debug(
          ce->first << " has no compatible CloseSE..."
        );
        deletingCEs.push_back(ce->first);	
      }
      else {
        // Now we have to count the number of files the CloseSEs supply the CE with.
        BrokerInfoData::LFN_container_type providedLFNs( 
          (*BI)->getProvidedLFNs(compatibleCloseSEs)
        );
        size_t n = providedLFNs.size();
        Debug(
          ce->first << " has #" << compatibleCloseSEs.size() <<
          " providing #" << n << " accessible file(s)"
        );

        if( n > max_files ) max_files=n;
        nFiles2CEs[n].insert(ce->first);
      }
    }
	
    // Remove the CEs which have no CloseSEs speaking the requested protocols
    std::for_each(
      deletingCEs.begin(), 
      deletingCEs.end(), 
      removeCEFromMatchTable(suitableCEs) 
    );
    MM.checkRank(jdl, *suitableCEs, m_prefetch);
	
    //Remove CEs with undefined rank 
    std::accumulate( 
      suitableCEs -> begin(),
      suitableCEs -> end(), 
      &deletingCEs, 
      insertUnRankedCEsInVector()
    );
    std::for_each(
      deletingCEs.begin(), 
      deletingCEs.end(), 
      removeCEFromMatchTable(suitableCEs)
    ); 
    
    bool FullListMatchResult = false;
    if (!jdl.EvaluateAttrBool("FullListMatchResult", FullListMatchResult) || 
      !FullListMatchResult) {

      const set<string>& CEs_class( nFiles2CEs[max_files] );

      deletingCEs.clear();
      std::accumulate( 
        suitableCEs -> begin(), 
        suitableCEs -> end(), 
        &deletingCEs, 
        insertNotInClassCEsInVector(CEs_class) 
      );
      std::for_each(
        deletingCEs.begin(), 
        deletingCEs.end(), 
        removeCEFromMatchTable(suitableCEs)
      );
    }
  }
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite


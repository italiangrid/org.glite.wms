// File: RBSimpleImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector> 

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"

#include "glite/wms/matchmaking/matchmakerGlueImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "glite/wms/broker/RBMaximizeFilesImpl.h"
#include "glite/wms/broker/utility.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)

using namespace std;

namespace glite {
namespace wms {

namespace requestad     = jdl;
namespace logger        = common::logger;

using namespace matchmaking;
using namespace brokerinfo;
	
namespace broker {

RBMaximizeFilesImpl::RBMaximizeFilesImpl(BrokerInfo<brokerinfoGlueImpl> *bi, bool do_prefetch)
{
	BI = bi;
	m_prefetch = do_prefetch;
}
 
RBMaximizeFilesImpl::~RBMaximizeFilesImpl()
{
}

match_table_t* RBMaximizeFilesImpl::findSuitableCEs(const classad::ClassAd* requestAd)
{
  match_table_t* suitableCEs = 0;
  if( requestAd ) { 
	
      	// retrieves all the CEs which satisfy user requiremets...	
	MatchMaker<matchmakerGlueImpl> MM;
  	suitableCEs = new match_table_t;
        if (m_prefetch) {
          MM.prefetchCEInfo(requestAd, *suitableCEs);
        }

  	MM.checkRequirement(requestAd, *suitableCEs, m_prefetch);
	
	// Now we have to filter this CEs to check whether they 
	// satisfy data requirements, or not.
	
	// Instantiates a BI object and collects all the information
	// about SFNs and involved SEs.
	// NOTE: this information is not CE dependent.
	BI->retrieveSFNsInfo(*requestAd);
	BI->retrieveSEsInfo (*requestAd);
	
	vector<string> deletingCEs;
	vector<string> data_access_protocols;
	requestad::get_data_access_protocol(*requestAd, data_access_protocols);
	map<size_t, set<string> > nFiles2CEs;
	size_t max_files = 0;
	
	for(match_const_iterator ce = suitableCEs->begin();
	    ce != suitableCEs->end(); ce++) {
		
		BI->retrieveCloseSEsInfo(ce->first);
		BrokerInfoData::SE_container_type compatibleCloseSEs( (*BI)->getCompatibleCloseSEs(data_access_protocols) );
		if(compatibleCloseSEs.empty()) {
		        
			edglog( debug ) << ce->first << " has no compatible CloseSE..." << endl;		
			deletingCEs.push_back(ce->first);	
		}
		else {
			// Now we have to count the number of files the CloseSEs supply the CE with.
			BrokerInfoData::LFN_container_type providedLFNs( (*BI)->getProvidedLFNs(compatibleCloseSEs) );
			size_t n = providedLFNs.size();
			edglog( debug ) << ce->first << " has #" << compatibleCloseSEs.size()
				<< " providing #" << n << " accessible file(s)" << endl;

			if( n > max_files ) max_files=n;
			nFiles2CEs[n].insert(ce->first);
		}
	}
	
	// Remove the CEs which have no CloseSEs speaking the requested protocols
	std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs) );
  	MM.checkRank       (requestAd, *suitableCEs, m_prefetch);
	
	// Remove CEs with undefined rank
	deletingCEs.clear();
	std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &deletingCEs, insertUnRankedCEsInVector() );
	std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs));
	
 		
	const set<string>& CEs_class( nFiles2CEs[max_files] );
	
	deletingCEs.clear();
	std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &deletingCEs, insertNotInClassCEsInVector(CEs_class) );
  	std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs));
  }
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite

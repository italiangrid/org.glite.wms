// File: RBSimpleImpl.cpp
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

#include "glite/wms/matchmaking/matchmakerGlueImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "RBSimpleImpl.h"
#include "utility.h"

using namespace std;

namespace glite {
namespace wms {
namespace broker {

RBSimpleImpl::RBSimpleImpl(bool do_prefetch)
{
  m_prefetch = do_prefetch;
}
 
RBSimpleImpl::~RBSimpleImpl()
{
}

matchmaking::match_table_t* RBSimpleImpl::findSuitableCEs(const classad::ClassAd* requestAd)
{
  matchmaking::match_table_t* suitableCEs = 0;
  if( requestAd ) { 
	matchmaking::MatchMaker<matchmaking::matchmakerGlueImpl> MM;
  	suitableCEs = new matchmaking::match_table_t;
	if (m_prefetch) {
  	  MM.prefetchCEInfo(requestAd, *suitableCEs);
	}
        classad::ClassAd jdl(*requestAd);
  	MM.checkRequirement(jdl, *suitableCEs, m_prefetch);
  	MM.checkRank       (jdl, *suitableCEs, m_prefetch);
  	//Remove CEs with undefined rank 
  	std::vector<std::string> deletingCEs;
  	std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &deletingCEs, insertUnRankedCEsInVector() );
  	std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs) ); 
  }
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite

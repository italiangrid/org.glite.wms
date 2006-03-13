// File: RBSimpleISMImpl.cpp
// Author: Salvatore Monforte
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
#include <classad_distribution.h>

#include "glite/wms/matchmaking/matchmakerISMImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "glite/wms/ism/ism.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "RBSimpleISMImpl.h"
#include "utility.h"

namespace glite {
namespace wms {
namespace broker {

RBSimpleISMImpl::RBSimpleISMImpl(bool do_prefetch)
{
}

RBSimpleISMImpl::~RBSimpleISMImpl()
{
}

matchmaking::match_table_t*
RBSimpleISMImpl::findSuitableCEs(classad::ClassAd const* requestAd)
{
  if (!requestAd) {
    return 0;
  }

  classad::ClassAd jdl(*requestAd);
  matchmaking::match_table_t* suitableCEs = 0;
  matchmaking::MatchMaker<matchmaking::matchmakerISMImpl> MM;
  suitableCEs = new matchmaking::match_table_t;
  bool const do_prefetch = false;
  MM.checkRequirement(jdl, *suitableCEs, do_prefetch);
  MM.checkRank       (jdl, *suitableCEs, do_prefetch);
  //Remove CEs with undefined rank 
  std::vector<std::string> deletingCEs;
  std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &deletingCEs, insertUnRankedCEsInVector() );
  std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs) ); 
  return suitableCEs;
}

}}}

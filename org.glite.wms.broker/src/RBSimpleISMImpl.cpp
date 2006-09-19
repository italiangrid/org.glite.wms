// File: RBSimpleISMImpl.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <functional>
#include <vector> 

#include <classad_distribution.h>
#include "glite/wms/matchmaking/matchmakerISMImpl.h"

#include "RBSimpleISMImpl.h"
#include "utility.h"

namespace glite {
namespace wms {
namespace broker {

boost::tuple<
  boost::shared_ptr<matchmaking::matchtable>,
  boost::shared_ptr<brokerinfo::filemapping>,
  boost::shared_ptr<brokerinfo::storagemapping>
>
RBSimpleISMImpl::findSuitableCEs(
  classad::ClassAd const* requestAd
){

  if (!requestAd) {
    return boost::tuples::make_tuple(
      boost::shared_ptr<matchmaking::matchtable>(),
      boost::shared_ptr<brokerinfo::filemapping>(),
      boost::shared_ptr<brokerinfo::storagemapping>()
    );
  }

  classad::ClassAd jdl(*requestAd);

  boost::shared_ptr<matchmaking::matchtable> suitableCEs(
    new matchmaking::matchtable
  );

  matchmaking::MatchMaker MM;
  MM.checkRequirement(jdl, *suitableCEs);
  MM.checkRank       (jdl, *suitableCEs);

  matchmaking::matchtable::iterator it(suitableCEs->begin());
  matchmaking::matchtable::iterator const e(suitableCEs->end());

  for( ; it != e ; ) {
    if(matchmaking::isRankUndefined(it->second)) {
      suitableCEs->erase(it++);
    }
    else {
      ++it;
    }
  }

  return boost::tuples::make_tuple(
    suitableCEs,
    boost::shared_ptr<brokerinfo::filemapping>(),
    boost::shared_ptr<brokerinfo::storagemapping>()
  );
}

}}}

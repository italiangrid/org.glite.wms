// File: matchmakerGlueISM.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MATCHMAKING_MATCHMAKERISMIMPL_H
#define GLITE_WMS_MATCHMAKING_MATCHMAKERISMIMPL_H

#include "glite/wms/matchmaking/matchmaker.h"

namespace glite {
namespace wms {
namespace matchmaking { 

class matchmakerISMImpl : public MatchMakerImpl
{
 public:
  matchmakerISMImpl();
  ~matchmakerISMImpl();
  void prefetchCEInfo  (const classad::ClassAd* requestAd, match_table_t& suitableCEs);
  void checkRequirement(const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces=false);
  void checkRank       (const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces=false);
 private:
    static boost::scoped_ptr< classad::ClassAd > gang_match_storageAd;
};

} // namespace matchmaking
} // namespace wms
} // namespace glite

#endif

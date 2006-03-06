// File: matchmakerGlueISM.h
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MATCHMAKING_MATCHMAKERISMIMPL_H
#define GLITE_WMS_MATCHMAKING_MATCHMAKERISMIMPL_H

#include "glite/wms/matchmaking/matchmaker.h"

namespace glite {
namespace wms {
namespace matchmaking { 

class matchmakerISMImpl: public MatchMakerImpl
{
public:
  void prefetchCEInfo(
    const classad::ClassAd* requestAd,
    match_table_t& suitableCEs
  );
  void checkRequirement(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ces = false
  );
  void checkRank(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ces = false
  );
};

}}}

#endif

// File: matchmakerGlueImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef _GLITE_WMS_MATCHMAKING_MATCHMAKERGLUEIMPL_H
#define _GLITE_WMS_MATCHMAKING_MATCHMAKERGLUEIMPL_H

#include "matchmaker.h"
#include "glite/wms/common/ldif2classad/LDIFObject.h"

namespace ldif2classad = glite::wms::common::ldif2classad;

namespace glite {
namespace wms {
namespace matchmaking { 

class matchmakerGlueImpl : public MatchMakerImpl
{
 public:
  matchmakerGlueImpl();
  ~matchmakerGlueImpl();
  void prefetchCEInfo  (const classad::ClassAd* requestAd, match_table_t& suitableCEs);
  void checkRequirement(const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces=false);
  void checkRank       (const classad::ClassAd* requestAd, match_table_t& suitableCEs, bool use_prefetched_ces=false);
 private:
#ifdef ENABLE_SHARED_LIBRARY_FUNCTIONS
    static boost::scoped_ptr< classad::ClassAd > gang_match_storageAd;
#endif
    std::vector<ldif2classad::LDIFObject> m_CE_info_cache;
    bool m_CE_info_prefetched;
};

} // namespace matchmaking
} // namespace wms
} // namespace glite

#endif

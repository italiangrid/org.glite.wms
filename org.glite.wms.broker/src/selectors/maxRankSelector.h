/*
 * File: maxRankSelector.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef GLITE_WMS_BROKER_SELECTORS_MAXRANKSELECTOR_H_
#define GLITE_WMS_BROKER_SELECTORS_MAXRANKSELECTOR_H_

#include "RBSelectionSchema.h"

namespace matchmaking = glite::wms::matchmaking;

namespace glite {
namespace wms {
namespace broker {

class maxRankSelector : public RBSelectionSchema
{
 public:
  typedef std::vector<matchmaking::match_table_t::const_iterator> match_container_type;
  typedef std::map<double, match_container_type> rank_to_match_container_map_type;
  maxRankSelector::maxRankSelector();
  maxRankSelector::~maxRankSelector();	
  matchmaking::match_const_iterator selectBestCE(const matchmaking::match_table_t& match_table);
};	

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

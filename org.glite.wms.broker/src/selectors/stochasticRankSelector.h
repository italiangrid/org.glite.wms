/*
 * File: stochasticRankSelector.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef GLITE_WMS_BROKER_SELECTORS_STOCHASTICRANKSELECTOR_H_
#define GLITE_WMS_BROKER_SELECTORS_STOCHASTICRANKSELECTOR_H_

#include "glite/wms/broker/selectors/RBSelectionSchema.h"

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_01.hpp>

namespace glite {
namespace wms {
namespace broker {

class stochasticRankSelector : public RBSelectionSchema
{
 public:
  stochasticRankSelector::stochasticRankSelector();
  stochasticRankSelector::~stochasticRankSelector();	
  matchmaking::match_const_iterator selectBestCE(const matchmaking::match_table_t& match_table);
};	

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

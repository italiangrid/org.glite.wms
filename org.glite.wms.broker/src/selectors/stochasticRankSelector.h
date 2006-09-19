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
  matchmaking::matchtable::const_iterator 
  selectBestCE(matchmaking::matchtable const& match_table);
};	

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

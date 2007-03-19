/*
 * File: maxRankSelector.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef GLITE_WMS_BROKER_MAX_SELECTOR_H_
#define GLITE_WMS_BROKER_MAX_SELECTOR_H_

#include "matchmaking.h"

namespace glite {
namespace wms {
namespace broker {

matchtable::iterator
max_selector(matchtable& matches);

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

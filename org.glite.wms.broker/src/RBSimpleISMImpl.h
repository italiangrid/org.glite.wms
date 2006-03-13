// File: RBSimpleISMImpl.h
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_BROKER_RBSIMPLEISMIMPL_H
#define GLITE_WMS_BROKER_RBSIMPLEISMIMPL_H

#include "glite/wms/broker/ResourceBroker.h"

namespace glite {
namespace wms {
namespace broker {

struct RBSimpleISMImpl: ResourceBrokerImpl
{
   RBSimpleISMImpl(bool = false);
  ~RBSimpleISMImpl();
  matchmaking::match_table_t* findSuitableCEs(classad::ClassAd const* requestAd);
};

}}}

#endif

// File: RBSimpleISMImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef GLITE_WMS_BROKER_RBSIMPLEISMIMPL_H
#define GLITE_WMS_BROKER_RBSIMPLEISMIMPL_H

#include "glite/wms/broker/ResourceBroker.h"

namespace glite {
namespace wms {
namespace broker {

class RBSimpleISMImpl : public ResourceBrokerImpl
{
public:
  RBSimpleISMImpl(bool do_prefetch=false);
  ~RBSimpleISMImpl();
  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd);
private:
  bool m_prefetch;
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

// File: RBSimpleImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef _GLITE_WMS_BROKER_RBSIMPLEIMPL_H_
#define _GLITE_WMS_BROKER_RBSIMPLEIMPL_H_

#include "glite/wms/broker/ResourceBroker.h"

namespace glite {
namespace wms {
namespace broker {

class RBSimpleImpl : public ResourceBrokerImpl
{
public:
  RBSimpleImpl(bool do_prefetch=false);
  ~RBSimpleImpl();
  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd);
private:
  bool m_prefetch;
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

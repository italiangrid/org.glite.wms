// File: RBMaximizeFilesISMImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef GLITE_WMS_BROKER_RBMAXIMIZEFILESISMIMPL_H
#define GLITE_WMS_BROKER_RBMAXIMIZEFILESISMIMPL_H

#include "glite/wms/broker/ResourceBroker.h"
#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

namespace glite {
namespace wms {
namespace broker {

namespace brokerinfo  = wms::brokerinfo;

class RBMaximizeFilesISMImpl : public ResourceBrokerImpl
{
public:
  RBMaximizeFilesISMImpl(brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> *, bool do_prefetch=false);
  ~RBMaximizeFilesISMImpl();
  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd);
private:
  brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl>* BI;
  bool m_prefetch;
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

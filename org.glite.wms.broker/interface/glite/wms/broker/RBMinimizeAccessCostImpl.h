// File: RBMinimizeAccessCostImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef _GLITE_WMS_BROKER_RBMINIMIZEACCESSCOSTIMPL_H_
#define _GLITE_WMS_BROKER_RBMINIMIZEACCESSCOSTIMPL_H_

#include "ResourceBroker.h"
#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

namespace brokerinfo  = glite::wms::brokerinfo;
namespace matchmaking = glite::wms::matchmaking;
namespace glite {
namespace wms {
namespace broker {

class RBMinimizeAccessCostImpl : public ResourceBrokerImpl
{
public:
  RBMinimizeAccessCostImpl(brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> *);
  ~RBMinimizeAccessCostImpl();
  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd);
private:
  brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> *BI;
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

// File: RBMaximizeFilesImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef _GLITE_WMS_BROKER_RBMAXIMIZEFILESIMPL_H_
#define _GLITE_WMS_BROKER_RBMAXIMIZEFILESIMPL_H_

#include "glite/wms/broker/ResourceBroker.h"
#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

namespace glite {
namespace wms {
namespace broker {	

namespace brokerinfo  = wms::brokerinfo;
namespace matchmaking = wms::matchmaking;

class RBMaximizeFilesImpl : public ResourceBrokerImpl
{
public:
   RBMaximizeFilesImpl(brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> *, bool do_prefetch=false);
  ~RBMaximizeFilesImpl();
  matchmaking::match_table_t* findSuitableCEs(const classad::ClassAd* requestAd);
private:
  brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl>* BI;
  bool m_prefetch;
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

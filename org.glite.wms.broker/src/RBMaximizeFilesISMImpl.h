// File: RBMaximizeFilesISMImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef GLITE_WMS_BROKER_RBMAXIMIZEFILESISMIMPL_H
#define GLITE_WMS_BROKER_RBMAXIMIZEFILESISMIMPL_H

#include "glite/wms/broker/ResourceBroker.h"

namespace glite {
namespace wms {
namespace broker {

namespace brokerinfo  = wms::brokerinfo;

struct RBMaximizeFilesISMImpl : ResourceBroker::Impl
{
  boost::tuple<
    boost::shared_ptr<matchmaking::matchtable>,
    boost::shared_ptr<brokerinfo::filemapping>,
    boost::shared_ptr<brokerinfo::storagemapping>
  >
  findSuitableCEs(const classad::ClassAd* requestAd);
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

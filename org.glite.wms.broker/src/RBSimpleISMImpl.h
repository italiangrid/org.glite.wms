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

struct RBSimpleISMImpl : ResourceBroker::Impl
{
  boost::tuple<
    boost::shared_ptr<matchmaking::matchtable>,
    boost::shared_ptr<brokerinfo::filemapping>,
    boost::shared_ptr<brokerinfo::storagemapping>
  > 
  findSuitableCEs(classad::ClassAd const*);
};

}}}

#endif

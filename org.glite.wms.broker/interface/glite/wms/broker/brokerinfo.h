/*
 * File: BrokerInfo.h
 * Author: Monforte Salvatore
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef _GLITE_WMS_BROKER_BROKERINFO_H_
#define _GLITE_WMS_BROKER_BROKERINFO_H_

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <storage_utils.h>
#include <map>
#include <string>
#include <vector>
#include <utility>

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

classad::ClassAd*
make_brokerinfo_ad(
  boost::shared_ptr<filemapping>,
  boost::shared_ptr<storagemapping>,
  classad::ClassAd const&
);

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

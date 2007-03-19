// File: RBMaximizeFilesISMImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

#ifndef GLITE_WMS_BROKER_MAXIMIZE_FILES_H
#define GLITE_WMS_BROKER_MAXIMIZE_FILES_H

#include "ResourceBroker.h"

namespace glite {
namespace wms {
namespace broker {


struct maximize_files : ResourceBroker::strategy
{
  boost::tuple<
    boost::shared_ptr<matchtable>,
    boost::shared_ptr<filemapping>,
    boost::shared_ptr<storagemapping>
  >
  operator()(const classad::ClassAd* requestAd);
};

} // namespace broker
} // namespace wms
} // namespace glite

#endif

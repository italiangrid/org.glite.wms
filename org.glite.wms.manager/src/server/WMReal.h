// File: WMReal.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_WMREAL_H
#define GLITE_WMS_MANAGER_SERVER_WMREAL_H

#include "WMImpl.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {
	
class WMReal: public glite::wms::manager::common::WMImpl
{
public:
  void submit(classad::ClassAd const* request_ad);
  void cancel(glite::wmsutils::jobid::JobId const& request_id);
};

} // server
} // manager
} // wms
} // glite

#endif // GLITE_WMS_MANAGER_SERVER_WMREAL_H

// File: DispatcherImpl.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H

#include <boost/utility.hpp>
#include "glite/wms/common/task/Task.h"
#include "TaskQueue.hpp"

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {


class DispatcherImpl: boost::noncopyable
{
public:
  DispatcherImpl();
  virtual ~DispatcherImpl();

  virtual void run(glite::wms::common::task::PipeWriteEnd<RequestPtr>& write_end) = 0;
};

}}}} // glite::wms::manager::server

#endif // GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H

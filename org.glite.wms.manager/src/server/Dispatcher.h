// File: Dispatcher.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHER_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

#ifndef BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#ifndef BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#include "pipedefs.h"
#endif
#ifndef GLITE_WMS_COMMON_TASK_TASK_H
#include "glite/wms/common/task/Task.h"
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl;

class Dispatcher:
    public common::task::PipeWriter<pipe_value_type>,
    boost::noncopyable
{
  boost::scoped_ptr<DispatcherImpl> m_impl;

public:
  Dispatcher();
  virtual ~Dispatcher();

  virtual void run();
};


} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

// File: RequestHandler.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_REQUESTHANDLER_H
#define GLITE_WMS_MANAGER_SERVER_REQUESTHANDLER_H

#ifndef BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef GLITE_WMS_COMMON_TASK_TASK_H
#include "glite/wms/common/task/Task.h"
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#include "pipedefs.h"
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class RequestHandler:
    public common::task::PipeReader<pipe_value_type>,
    boost::noncopyable
{
  class Impl;
  Impl* m_impl;

public:
  RequestHandler();
  virtual ~RequestHandler();

  virtual void run();
};


} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_MANAGER_SERVER_REQUESTHANDLER_H

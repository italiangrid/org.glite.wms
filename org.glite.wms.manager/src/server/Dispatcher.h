// File: Dispatcher.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHER_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include "glite/wms/common/task/Task.h"
#include "TaskQueue.hpp"

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl;

class Dispatcher:
    public glite::wms::common::task::PipeWriter<RequestPtr>,
    boost::noncopyable
{
  boost::scoped_ptr<DispatcherImpl> m_impl;

public:
  Dispatcher();
  virtual ~Dispatcher();

  virtual void run();
};


}}}}

#endif // GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

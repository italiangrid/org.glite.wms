// File: Dispatcher.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHER_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

#include <boost/shared_ptr.hpp>
#include "pipedefs.h"

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl;

class Dispatcher:
    public glite::wms::common::task::PipeWriter<RequestPtr, queue_type>
{
  boost::shared_ptr<DispatcherImpl> m_impl;

public:
  Dispatcher();

  void operator()();
};


}}}}

#endif // GLITE_WMS_MANAGER_SERVER_DISPATCHER_H

// File: RequestHandler.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_REQUESTHANDLER_H
#define GLITE_WMS_MANAGER_SERVER_REQUESTHANDLER_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include "glite/wms/common/task/Task.h"

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class Request;

class RequestHandler:
    public glite::wms::common::task::PipeReader<boost::shared_ptr<Request> >,
    boost::noncopyable
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  RequestHandler();
  void run();
};


}}}} // glite::wms::manager::server

#endif // EDG_WORKLOAD_PLANNING_MANAGER_REQUESTHANDLER_H

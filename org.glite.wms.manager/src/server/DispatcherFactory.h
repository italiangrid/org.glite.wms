// File: DispatcherFactory.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFACTORY_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFACTORY_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#ifndef GLITE_WMS_X_BOOST_SCOPED_PTR_HPP
#define GLITE_WMS_X_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHING_UTILS_H
#include "dispatching_utils.h"
#endif

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl;

class DispatcherFactory: boost::noncopyable
{
  class Impl;

  boost::scoped_ptr<Impl> m_impl;

  static DispatcherFactory* s_instance;

  DispatcherFactory();

  typedef DispatcherImpl product_type;
  typedef product_type* (*product_creator_type)();

public:
  static DispatcherFactory* instance();
  ~DispatcherFactory();

public:

  bool register_dispatcher(dispatcher_type const& id, product_creator_type creator);
  bool unregister_dispatcher(dispatcher_type const& id);
  product_type* create_dispatcher(dispatcher_type const& id);
};

} // server
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:

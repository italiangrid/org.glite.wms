// File: DispatcherFactory.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFACTORY_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFACTORY_H

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <exception> 

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl;

struct NoCreateDispatcherException: public std::exception
{
  const char* what() const throw () 
  { 
    return "Unknown Dispatcher"; 
  }
};


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

  bool register_dispatcher(std::string const& id, product_creator_type creator);
  bool unregister_dispatcher(std::string const& id);
  product_type* create_dispatcher(std::string const& id);
};

}}}} // glite::wms::manager::server

#endif

// Local Variables:
// mode: c++
// End:

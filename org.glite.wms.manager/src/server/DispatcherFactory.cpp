// File: DispatcherFactory.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DispatcherFactory.h"

#include <string>
#include <map>


namespace glite {
namespace wms {
namespace manager {
namespace server {

DispatcherFactory* DispatcherFactory::s_instance = 0;

class DispatcherFactory::Impl: boost::noncopyable
{
  typedef std::map<std::string, product_creator_type> factory_type;
  factory_type m_factory;

public:
  
  bool register_dispatcher(std::string const& id, product_creator_type creator);
  bool unregister_dispatcher(std::string const& id);
  product_type* create_dispatcher(std::string const& id);
};
  
bool
DispatcherFactory::Impl::register_dispatcher(std::string const& id, product_creator_type creator)
{
  return m_factory.insert(std::make_pair(id, creator)).second;
}

bool
DispatcherFactory::Impl::unregister_dispatcher(std::string const& id)
{
  return m_factory.erase(id) == 1;
}

DispatcherFactory::product_type*
DispatcherFactory::Impl::create_dispatcher(std::string const& id)
{
  factory_type::iterator i = m_factory.find(id);
  if (i == m_factory.end())
    {
      throw NoCreateDispatcherException();
    }
   
  return (i->second)();  
}

DispatcherFactory*
DispatcherFactory::instance()
{
  if (s_instance == 0) {
    s_instance = new DispatcherFactory;
  }

  return s_instance;
}

DispatcherFactory::DispatcherFactory()
  : m_impl(new Impl)
{
}

bool
DispatcherFactory::register_dispatcher(std::string const& id, product_creator_type creator)
{
  return m_impl->register_dispatcher(id, creator);
}

bool
DispatcherFactory::unregister_dispatcher(std::string const& id)
{
  return m_impl->unregister_dispatcher(id);
}

DispatcherFactory::product_type*
DispatcherFactory::create_dispatcher(std::string const& id)
{
  return m_impl->create_dispatcher(id);
}

} // server
} // manager
} // wms
} // glite


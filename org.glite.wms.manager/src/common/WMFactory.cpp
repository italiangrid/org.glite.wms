// File: WMFactory.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMFactory.h"
#include <map>
#include <boost/utility.hpp>


namespace glite {
namespace wms {
namespace manager {
namespace common {

WMFactory* WMFactory::s_instance = 0;

class WMFactory::Impl: boost::noncopyable
{
  typedef std::map<wm_type, product_creator_type> factory_type;
  factory_type m_factory;
  
public:
  
  bool register_wm(wm_type const& id, product_creator_type creator);
  bool unregister_wm(wm_type const& id);
  product_type* create_wm(wm_type const& id);
};

bool
WMFactory::Impl::register_wm(wm_type const& id, product_creator_type creator)
{
  return m_factory.insert(std::make_pair(id, creator)).second;
}

bool
WMFactory::Impl::unregister_wm(wm_type const& id)
{
  return m_factory.erase(id) == 1;
}

WMFactory::product_type*
WMFactory::Impl::create_wm(wm_type const& id)
{
  factory_type::iterator i = m_factory.find(id);
  if (i == m_factory.end())
    {
      throw NoCreateWMException();
    }
  return (i->second)();   
}

WMFactory*
WMFactory::instance()
{
  if (s_instance == 0) {
    s_instance = new WMFactory;
  }

  return s_instance;
}

WMFactory::WMFactory()
  : m_impl(new Impl)
{
}

bool
WMFactory::register_wm(wm_type const& id, product_creator_type creator)
{
  return m_impl->register_wm(id, creator);
}

bool
WMFactory::unregister_wm(wm_type const& id)
{
  return m_impl->unregister_wm(id);
}

WMFactory::product_type*
WMFactory::create_wm(wm_type const& id)
{
  return m_impl->create_wm(id);
}

}}}} // glite::wms::manager::common

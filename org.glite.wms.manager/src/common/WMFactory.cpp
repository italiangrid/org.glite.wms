// File: WMFactory.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "WMFactory.h"
#include <boost/utility.hpp>
#include "glite/wms/thirdparty/loki/Factory.h"

namespace glite {
namespace wms {
namespace manager {
namespace common {

WMFactory* WMFactory::s_instance = 0;

class WMFactory::Impl: boost::noncopyable
{
  typedef Loki::Factory<product_type, wm_type, product_creator_type> factory_type;
  factory_type m_factory;

public:

  bool register_wm(wm_type const& id, product_creator_type creator);
  bool unregister_wm(wm_type const& id);
  product_type* create_wm(wm_type const& id);
};

bool
WMFactory::Impl::register_wm(wm_type const& id, product_creator_type creator)
{
  return m_factory.Register(id, creator);
}

bool
WMFactory::Impl::unregister_wm(wm_type const& id)
{
  return m_factory.Unregister(id);
}

WMFactory::product_type*
WMFactory::Impl::create_wm(wm_type const& id)
{
  return m_factory.CreateObject(id);
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

// File: HelperFactory.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <utility>
#include <classad_distribution.h>
#include <boost/utility.hpp>

#include "HelperFactory.h"
#include "HelperImpl.h"
#include "glite/wms/thirdparty/loki/Factory.h"

using namespace std;
using namespace classad;

namespace glite {
namespace wms {
namespace helper {

HelperFactory* HelperFactory::s_instance = 0;

class HelperFactory::Impl: boost::noncopyable
{
  typedef Loki::Factory<product_type, std::string, product_creator_type> factory_type;
  typedef Loki::DefaultFactoryError<std::string, product_type> error_type;
  factory_type m_factory;

public:

  bool register_helper(std::string const& id, product_creator_type creator);
  bool unregister_helper(std::string const& id);
  product_type* create_helper(std::string const& id);
  std::vector<std::string> list() const;
};

bool
HelperFactory::Impl::register_helper(std::string const& id, product_creator_type creator)
{
  return m_factory.Register(id, creator);
}

bool
HelperFactory::Impl::unregister_helper(std::string const& id)
{
  return m_factory.Unregister(id);
}

HelperFactory::product_type*
HelperFactory::Impl::create_helper(std::string const& id)
try {
  return m_factory.CreateObject(id);
} catch (error_type const&) {
  return 0;
}

std::vector<std::string>
HelperFactory::Impl::list() const
{
  std::vector<std::string> result;

  factory_type::IdToProductMap m = m_factory.List();
  for (factory_type::IdToProductMap::iterator it = m.begin(); it != m.end(); ++it) {
    result.push_back(it->first);
  }

  return result;
}

HelperFactory*
HelperFactory::instance()
{
  if (s_instance == 0) {
    s_instance = new HelperFactory;
  }

  return s_instance;
}

HelperFactory::HelperFactory()
  : m_impl(new Impl)
{
}

HelperFactory::~HelperFactory()
{
  delete m_impl;
}

bool
HelperFactory::register_helper(std::string const& id, product_creator_type creator)
{
  return m_impl->register_helper(id, creator);
}

bool
HelperFactory::unregister_helper(std::string const& id)
{
  return m_impl->unregister_helper(id);
}

HelperFactory::product_type*
HelperFactory::create_helper(std::string const& id)
{
  return m_impl->create_helper(id);
}

std::vector<std::string>
HelperFactory::list() const
{
  return m_impl->list();
}


} // namespace helper
} // namespace wms
} // namespace glite

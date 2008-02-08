// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "Helper.h"
#include "glite/wms/helper/HelperImpl.h"
#include "glite/wms/helper/HelperFactory.h"
#include "glite/wms/helper/exceptions.h"

namespace glite {
namespace wms {
namespace helper {

Helper::Helper(std::string const& id)
{
  try {
    m_impl = HelperFactory::instance()->create_helper(id);
  } catch (...) {
    throw NoSuchHelper(id);
  }

  if (!m_impl) {
    throw NoSuchHelper(id);
  }
}

Helper::~Helper()
{
  delete m_impl;
}

std::string
Helper::id() const
{
  return m_impl->id();
}

std::string
Helper::resolve(std::string const& input_file) const
{
  return m_impl->resolve(input_file);
};

classad::ClassAd*
Helper::resolve(
  classad::ClassAd const* input_ad,
  boost::shared_ptr<std::string> jw_template
) const
{
  return m_impl->resolve(input_ad, jw_template);
}

}}} // glite::wms::helper

// File: exceptions.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "exceptions.h"

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

class CannotCreateJobWrapper::Impl
{
public:
  std::string m_what;
  std::string m_path;
};

CannotCreateJobWrapper::CannotCreateJobWrapper(std::string const& path)
try
  : HelperError("JobAdapterHelper")
{
  m_impl.reset(new Impl);
  m_impl->m_path = path;
} catch (...) {
  m_impl.reset();
}

CannotCreateJobWrapper::~CannotCreateJobWrapper() throw()
{
}

std::string
CannotCreateJobWrapper::path() const
{
  return m_impl ? m_impl->m_path : "";
}

char const*
CannotCreateJobWrapper::what() const throw()
{
  if (m_impl) {
    if (m_impl->m_what.empty()) {
      m_impl->m_what = helper() + ": cannot create job wrapper at " + path();
    }
    return m_impl->m_what.c_str();
  } else {
    return "JobAdapter error: CannotCreateJobWrapper";
  }
}

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite


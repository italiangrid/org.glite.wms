// File: exceptions.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_JOBADAPTER_EXCEPTIONS_H
#define GLITE_WMS_HELPER_JOBADAPTER_EXCEPTIONS_H

#include "glite/wms/helper/exceptions.h"

#include <string>
#include <boost/shared_ptr.hpp>

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

class CannotCreateJobWrapper: public helper::HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  CannotCreateJobWrapper(std::string const& path);
  ~CannotCreateJobWrapper() throw();
  std::string path() const;
  char const* what() const throw();
};

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif

// Local Variables:
// mode: c++
// End:

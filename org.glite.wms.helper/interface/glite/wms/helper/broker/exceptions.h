// File: exceptions.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_BROKER_EXCEPTIONS_H
#define GLITE_WMS_HELPER_BROKER_EXCEPTIONS_H

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "glite/wms/helper/exceptions.h"

namespace glite {
namespace wms {
namespace helper {
namespace broker {

class CannotCreateBrokerinfo: public helper::HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  explicit CannotCreateBrokerinfo(boost::filesystem::path const& p);
  ~CannotCreateBrokerinfo() throw();
  boost::filesystem::path path() const;
  char const* what() const throw();
};

class NoAvailableCEs: public helper::HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  explicit NoAvailableCEs(std::string const& reason);
  ~NoAvailableCEs() throw();
  std::string reason() const;
  char const* what() const throw();
};

class NoCompatibleCEs: public helper::HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  typedef enum { FALSE_REQS, INVALID_REQS, ERROR_REQS, DONTKNOW } reason_type;
  NoCompatibleCEs();
  NoCompatibleCEs(std::string const& is_hostname, reason_type reason);
  ~NoCompatibleCEs() throw();
  std::string is_hostname() const;
  reason_type reason() const;
  char const* what() const throw();
};

}}}} // glite::wms::helper::broker

#endif

// Local Variables:
// mode: c++
// End:

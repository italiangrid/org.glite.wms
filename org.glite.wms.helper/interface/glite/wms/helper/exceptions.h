// File: exceptions.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_EXCEPTIONS_H
#define GLITE_WMS_HELPER_EXCEPTIONS_H

#include <exception>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/exception.hpp>

namespace glite {
namespace jdl {
class CannotGetAttribute;
class CannotSetAttribute;
}

namespace wms {

namespace helper {

class HelperError: public std::exception
{
  std::string m_helper;

public:
  explicit HelperError(std::string const& helper);
  ~HelperError() throw();
  char const* what() const throw()
  {
    return "HelperError";
  }
  std::string helper() const;
};

class NoSuchHelper: public HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  NoSuchHelper(std::string const& helper);
  ~NoSuchHelper() throw();
  char const* what() const throw();
};

class CannotGetAttribute: public HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  CannotGetAttribute(std::string const& attribute,
                     std::string const& attribute_type,
                     std::string const& helper);
  CannotGetAttribute(glite::jdl::CannotGetAttribute const& e,
                     std::string const& helper);
  ~CannotGetAttribute() throw();
  char const* what() const throw();
  std::string attribute() const;
  std::string attribute_type() const;
};

class InvalidAttributeValue: public HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  InvalidAttributeValue(std::string const& attribute,
                        std::string const& value,
                        std::string const& expected,
                        std::string const& helper);
  ~InvalidAttributeValue() throw();
  char const* what() const throw();
  std::string attribute() const;
  std::string value() const;
  std::string expected() const;
};

class CannotSetAttribute: public HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  CannotSetAttribute(std::string const& attribute, std::string const& helper);
  CannotSetAttribute(glite::jdl::CannotSetAttribute const& e,
                     std::string const& helper);
  ~CannotSetAttribute() throw();
  char const* what() const throw();
  std::string attribute() const;
};

class FileSystemError: public HelperError
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  FileSystemError(std::string const& helper,
                  boost::filesystem::filesystem_error const& e);
  ~FileSystemError() throw();
  boost::filesystem::filesystem_error error() const;
  char const* what() const throw();
};

} // namespace helper
} // namespace wms
} // namespace glite

#endif

// Local Variables:
// mode: c++
// End:

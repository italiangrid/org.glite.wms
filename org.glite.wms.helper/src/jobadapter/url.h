//**************************************************************************
//  Filename  : url.h
//  Authors   : Elisabetta Ronchieri
//              Francesco Giacomini
//              Marco Cecchi
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//**************************************************************************

#ifndef GLITE_WMS_HELPER_JOBADAPTER_URL_H
#define GLITE_WMS_HELPER_JOBADAPTER_URL_H

#include <string>

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

class InvalidURL: public std::exception
{
public:
  InvalidURL(std::string const& url);
  ~InvalidURL() throw();
  char const* what() const throw();

private:
  std::string m_message;
};

class URL
{
public:
  URL(std::string const& url);

  std::string protocol() const;
  std::string host() const;
  std::string port() const;
  std::string path() const;
  std::string as_string() const;

private:
  std::string m_protocol;
  std::string m_host;
  std::string m_path;
  std::string m_port;
};

}}}} // glite::wms::helper::jobadapter

#endif // GLITE_WMS_HELPER_JOBADAPTER_URL_H

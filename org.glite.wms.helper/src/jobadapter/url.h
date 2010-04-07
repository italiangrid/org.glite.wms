/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://project.eu-egee.org/partners for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

//**************************************************************************
//  Filename  : url.h
//  Authors   : Elisabetta Ronchieri
//              Francesco Giacomini
//              Marco Cecchi
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

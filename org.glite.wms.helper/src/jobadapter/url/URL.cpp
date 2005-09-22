/***************************************************************************
 *  filename  : URL.cpp
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *  copyright : (C) 2001 by INFN
 ***************************************************************************/

#include <iostream>
#include <string>

#include <cctype>
#include <cstdlib>

#include "URL.h"

using namespace std;

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace url {

ExInvalidURL::ExInvalidURL(const char *par)
  : m_invalidurl_parameter(par)
{
}
	
const string& ExInvalidURL::parameter(void) const
{ 
  return m_invalidurl_parameter; 
}
    
URL::URL(void)
  : m_is_empty(true)
{
}

URL::URL(string url)
  : m_is_empty(false)
{
  parse(url);
}

URL::~URL(void)
{
}

void
URL::parse(string url)
{
  string::size_type pos = url.find("://");
  string ur(url);

  string msg("Could not find ");
  if (pos == string::npos) {
    msg.append("\'://\' in URL ");
    msg.append(ur);
    throw ExInvalidURL(msg.c_str());
  }
  string protocol = url.substr(0, pos);
  url      = url.substr(pos+3);

  pos      = url.find("/");
  if (pos == string::npos) {
    msg.append("\'/\' in URL ");
    msg.append(ur);
    throw ExInvalidURL(msg.c_str());
  }
  string hostpart = url.substr(0, pos);

  string host;
  string port;
  string::size_type host_end = hostpart.find(":");
  if (host_end == string::npos) {  // no port
    host = hostpart;
    port = "";
  } else {                             // port
    host = hostpart.substr(0, host_end);
    port     = hostpart.substr(host_end + 1, pos);

    // -*- Check port value -*-
    for(string::const_iterator it = port.begin();
	it != port.end(); it++){
      if (!isdigit(*it)) {
        msg.append("\'correct port number\' in URL ");
        msg.append(ur);
	throw ExInvalidURL(msg.c_str());
      }
    }
  }

  string path = url.substr(pos);

  m_protocol = protocol;
  m_host     = host;
  m_port     = port;
  m_path     = path;
}

bool
URL::is_empty(void) const
{
  return m_is_empty;
}

string
URL::protocol(void) const
{
  return m_protocol;
}

string
URL::host(void) const
{
  return m_host;
}

string
URL::port(void) const
{
  return m_port;
}

string
URL::path(void) const
{
  return m_path;
}

string
URL::as_string(void) const
{
  return m_protocol + "://" + m_host + ((m_port != "") ? ":" + m_port : "")+ m_path;
}

} // namespace url
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

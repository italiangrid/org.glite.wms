/***************************************************************************
 *  Filename  : URL.cpp
 *  Authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
                Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright : (C) 2006 by INFN
 ***************************************************************************/

//Formalization of URI in BNF according to RFC #3986 Jan 2005
// (restricted to our own needs)
//
//URI           = scheme ":" hier-part
//hier-part     = "//" authority path-abempty
//               / path-absolute
//               / path-rootless
//               / path-empty
//scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
//authority     = host [ ":" port ]
//host          = IP-literal / IPv4address / reg-name
//port          = *DIGIT
//path-abempty  = *( "/" segment )
//path-absolute = "/" [ segment-nz *( "/" segment ) ]
//path-noscheme = segment-nz-nc *( "/" segment )
//path-rootless = segment-nz *( "/" segment )
//path-empty    = 0<pchar>
//segment       = *pchar
//segment-nz    = 1*pchar
//segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//; non-zero-length segment without any colon ":"
//pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
//query         = *( pchar / "/" / "?" )
//fragment      = *( pchar / "/" / "?" )
//pct-encoded   = "%" HEXDIG HEXDIG
//unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
//sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
//
//Some example:
//"http://www.cnaf.infn.it"                   = Valid
//"http://www.cnaf.infn.it:"                  = Valid (port = *DIGIT)
//"h ttp://www.cnaf.infn.it"                  = Not valid
//"http://www.c naf.infn.it"                  = Not valid
//"http://www.ics.uci.edu:8080/pub/ietf/uri/" = Valid
//"http://www.ics.uci.edu:8080/pub/ietf/uri"  = Valid
//"http://www.ics.uci.edu:8080"               = Valid
//"http://www.ics.uci.edu:8080:16"            = Not valid
//"http://www.ics.uci.edu:8080/pub/i:etf/uri" = Not valid
//"http://www.ics.uci.edu:8080/pub/i%20f/uri" = Valid
//"http://www.ics.uci.edu:8080/pub/i%2tf/uri" = Not valid
//""                                          = Not valid

#include <string>
#include <boost/regex.hpp>

#include "URL.h"

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace url {

ExInvalidURL::ExInvalidURL(std::string const& par)
  : m_invalidurl_parameter(par)
{
}
	
const std::string& ExInvalidURL::parameter(void) const
{ 
  return m_invalidurl_parameter; 
}
    
URL::URL(std::string url)
{
  parse(url);
}

URL::~URL(void)
{
}

void
URL::parse(std::string url)
{
  static const boost::regex valid_url("^([a-z,A-Z,0-9,-,.,_,~]+)://([^:/\
?#@ ]+):?(([0-9]*)?)((/([a-z,A-Z,0-9,-,.,_,~,!,$,&,',(,)]|%[a-e,A-E,0-9]\
[a-e,A-E,0-9])+)*/?)$");

  bool is_matching = false;
  try {
    is_matching = boost::regex_match(url, valid_url);
  } catch(std::runtime_error ex) {
    throw ExInvalidURL("Cannot parse URL:" + url);
  }
  if (is_matching)
  {
    try {
      boost::sregex_iterator m(url.begin(), url.end(), valid_url);

      m_protocol = (*m)[1].str();
      m_host = (*m)[2].str();
      m_port = (*m)[3].str();
      m_path = (*m)[5].str();
    } catch(std::runtime_error ex) {
      throw ExInvalidURL("Cannot parse URL:" + url);
    }
  } else {
    throw ExInvalidURL(url);
  }
}

std::string
URL::protocol(void) const
{
  return m_protocol;
}

std::string
URL::host(void) const
{
  return m_host;
}

std::string
URL::port(void) const
{
  return m_port;
}

std::string
URL::path(void) const
{
  return m_path;
}

std::string
URL::as_string(void) const
{
  return m_protocol
    + "://" 
    + m_host 
    + (!m_port.empty() ? ":" + m_port : "")
    + m_path;
}

}}}}} // namespace glite::wms::helper::jobadapter::url

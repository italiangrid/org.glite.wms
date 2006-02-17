//**************************************************************************
//  Filename  : url.cpp
//  Authors   : Elisabetta Ronchieri
//              Francesco Giacomini
//              Marco Cecchi
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//**************************************************************************

//Formalization of URI in BNF according to RFC #3986 Jan 2005
// (restricted to our own needs)

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
//Some examples:
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

#include "url.h"

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

InvalidURL::InvalidURL(std::string const& msg)
  : m_message(msg)
{
}
	
std::string const& InvalidURL::message() const
{ 
  return m_message; 
}
    
URL::URL(std::string const& url)
{

  static const boost::regex valid_url("\
([[:alpha:]][[:alpha:],[:digit:],+,.,-]*)\
://\
(([[:alpha:],[:digit:],_,.,~,!,$,&,',(,),-]|%[[:xdigit:]][[:xdigit:]])+)\
(:([0-9]*))?\
((/([[:alpha:],[:digit:],_,.,~,!,$,&,',(,),-]|%[[:xdigit:]][[:xdigit:]])+)*)/?\
");


  bool is_matching = false;
  try {
    is_matching = boost::regex_match(url, valid_url);
  } catch(std::runtime_error& ex) {
    throw InvalidURL("Cannot parse URL:" + url);
  }
  if (is_matching)
  {
    try {
      boost::sregex_iterator m(url.begin(), url.end(), valid_url);

      m_protocol = (*m)[1].str();
      m_host = (*m)[2].str();
      m_port = (*m)[5].str();
      m_path = (*m)[6].str();
    } catch(std::runtime_error& ex) {
      throw InvalidURL("Cannot parse URL:" + url);
    }
  } else {
    throw InvalidURL("Cannot parse URL:" + url);
  }
}

std::string
URL::protocol() const
{
  return m_protocol;
}

std::string
URL::host() const
{
  return m_host;
}

std::string
URL::port() const
{
  return m_port;
}

std::string
URL::path() const
{
  return m_path;
}

std::string
URL::as_string() const
{
  return m_protocol
    + "://" 
    + m_host 
    + (!m_port.empty() ? ":" + m_port : "")
    + m_path;
}

}}}} // namespace glite::wms::helper::jobadapter

/***************************************************************************
 *  filename  : LDAPFilterParser.h
 *  authors   : Salvatore Monforte <salvatore.monforte@ct.infn.it>
 *  copyright : (C) 2001, 2002 by INFN
 ***************************************************************************/

/**
 * @file LDAPFilterParser.h
 * @brief 
 * @author Salvatore Monforte salvatore.monforte@ct.infn.it
 */

// $Id$
// $Date$

#ifndef _LDAPFILTERPARSER_
#define _LDAPFILTERPARSER_

#include <string>
#include <stack>
#include <vector>

namespace glite {
namespace wms {
namespace common {
namespace ldif2classad {

/**
 * Token definition
 */
struct token_t
{
  enum type_t { none, 
		filtercomp, 
		attributetype, 
		attributevalue, 
		filtertype, 
		bracket_open, 
		bracket_close,
		simple_string,
		extended_string
  };
  token_t::token_t() { type = none; }
  token_t::token_t(token_t::type_t t, const std::string& v) { 
    type = t; 
    value = v; 
  }
 
  type_t type;
  std::string value;
};

/**
 * This class models a basic tokenizer object
 * for LDAP filters.
 */
class LDAPFilterTokenizer
{
public:
 
  enum filtercomp_t { AND = '&', OR = '|', NOT = '!' };
  enum filtertype_t { LIKE = '~', EQUAL = '=', LESS = '<', GREATER = '>'};
  
  LDAPFilterTokenizer(const std::string& s);
  bool get_token(token_t& type);
  bool eof() const { return ( ipos == std::string::npos || ipos >= ifilter.length() ); }
  void break_on_reserved(bool v) { BORflag = v;  }

private:
  std::string purify(const std::string& s) const;

private:
  std::string ifilter;
  size_t ipos;
  bool BORflag;
};

/**
 * This class models a basic parser object
 * for LDAP filters.
 */
class LDAPFilterParser
{
public:
  LDAPFilterParser() { }
  bool parse(const std::string& filter, std::string& p, std::vector<std::string>* = NULL);
private:
  std::string make_expression( std::vector<std::string>* = NULL ); 
  bool parse_expression( std::vector<std::string>* = NULL );

private:
  std::stack<token_t> filtercomps;
  std::stack<std::string> parsedterms;
};

} // namespace ldif2classad
} // namespace common
} // namespace wms
} // namespace glite


#endif

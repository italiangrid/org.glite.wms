// File: ldap-dn-utils.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

void tokenize_ldap_dn(std::string const& s, std::vector<std::string> &v)
{
  boost::escaped_list_separator<char> ldap_dn_sep("",",","");
  boost::tokenizer<boost::escaped_list_separator<char> > 
    ldap_dn_tok(s,ldap_dn_sep);

  boost::tokenizer< boost::escaped_list_separator<char> >::iterator
    ldap_dn_tok_it(
      ldap_dn_tok.begin()
    );
  boost::tokenizer< boost::escaped_list_separator<char> >::iterator const 
    ldap_dn_tok_end(
       ldap_dn_tok.end()
    );

  for( ; ldap_dn_tok_it != ldap_dn_tok_end; ++ldap_dn_tok_it) 
    v.push_back(
      boost::algorithm::trim_copy(*ldap_dn_tok_it)
    );
  
}

bool is_gluecluster_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 1 && 
   boost::algorithm::istarts_with(dn[0], "GlueClusterUniqueID") &&
   boost::algorithm::istarts_with(dn[1],"mds-vo-name");
}

bool is_gluece_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 1 && 
   boost::algorithm::istarts_with(dn[0],"GlueCEUniqueID") &&
   boost::algorithm::istarts_with(dn[1],"mds-vo-name"); 
}

bool is_gluesubcluster_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   boost::algorithm::istarts_with(dn[0],"GlueSubClusterUniqueID") && 
   boost::algorithm::istarts_with(dn[1],"GlueClusterUniqueID") &&
   boost::algorithm::istarts_with(dn[2],"mds-vo-name"); 
}

bool is_gluecesebind_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   boost::algorithm::istarts_with(dn[0],"GlueCESEBindSEUniqueID") && 
   boost::algorithm::istarts_with(dn[1],"GlueCESEBindGroupCEUniqueID") && 
   boost::algorithm::istarts_with(dn[2],"mds-vo-name");
}

bool is_gluevoview_info_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GlueVOViewLocalID") &&
    boost::algorithm::istarts_with(dn[1],"GlueCEUniqueID") &&
    boost::algorithm::istarts_with(dn[2],"mds-vo-name");
}

bool is_gluese_info_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 1 &&
    boost::algorithm::istarts_with(dn[0],"GlueSEUniqueID") &&
    boost::algorithm::istarts_with(dn[1],"mds-vo-name");
}

bool is_gluesa_info_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GlueSALocalID") &&
    boost::algorithm::istarts_with(dn[1],"GlueSEUniqueID") &&
    boost::algorithm::istarts_with(dn[2],"mds-vo-name");
}

bool is_gluese_access_protocol_info_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GlueSEAccessProtocolLocalID") &&
    boost::algorithm::istarts_with(dn[1],"GlueSEUniqueID") &&
    boost::algorithm::istarts_with(dn[2],"mds-vo-name");
}

bool is_gluese_control_protocol_info_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GlueSEControlProtocolLocalID") &&
    boost::algorithm::istarts_with(dn[1],"GlueSEUniqueID") &&
    boost::algorithm::istarts_with(dn[2],"mds-vo-name");
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

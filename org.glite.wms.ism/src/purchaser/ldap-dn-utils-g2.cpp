/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners for details on the
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

// File: ldap-dn-utils.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.

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

bool is_glue2_service_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 1 && 
   boost::algorithm::istarts_with(dn[0], "GLUE2ServiceID") &&
   boost::algorithm::istarts_with(dn[1],"GLUE2GroupID");
}

bool is_glue2_manager_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   boost::algorithm::istarts_with(dn[0],"GLUE2ManagerID") &&
   boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") &&
   boost::algorithm::istarts_with(dn[2],"GLUE2GroupID");; 
}

bool is_glue2_share_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   boost::algorithm::istarts_with(dn[0],"GLUE2ShareID") && 
   boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") &&
   boost::algorithm::istarts_with(dn[2],"GLUE2GroupID"); 
}

bool is_glue2_endpoint_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   boost::algorithm::istarts_with(dn[0],"GLUE2EndpointID") && 
   boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") && 
   boost::algorithm::istarts_with(dn[2],"GLUE2GroupID");
}

bool is_glue2_to_storage_service_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2ToStorageServiceID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2GroupID");
}

bool is_glue2_mapping_policy_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 3 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2PolicyID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ShareID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[3],"GLUE2GroupID");
}
bool is_glue2_access_policy_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 3 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2PolicyID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2EndpointID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[3],"GLUE2GroupID");
}

bool is_glue2_resource_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2ResourceID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2GroupID");
}

bool is_glue2_benchmark_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 3 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2BenchmarkID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ResourceID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[3],"GLUE2GroupID");
}

bool is_glue2_application_env_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 3 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2ApplicationEnvironmentID") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ResourceID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[3],"GLUE2GroupID");
}

bool is_glue2_service_capacity_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 2 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2StorageServiceCapacity") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2GroupID");
}

bool is_glue2_share_capacity_dn(std::vector<std::string> const& dn)
{
  return dn.size() > 3 &&
    boost::algorithm::istarts_with(dn[0],"GLUE2StorageShareCapacity") &&
    boost::algorithm::istarts_with(dn[1],"GLUE2ShareID") &&
    boost::algorithm::istarts_with(dn[2],"GLUE2ServiceID") &&
    boost::algorithm::istarts_with(dn[3],"GLUE2GroupID");
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

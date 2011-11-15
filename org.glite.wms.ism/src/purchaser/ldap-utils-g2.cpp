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

// File: ldap-utils.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.

// $Id$
#include <sys/time.h>
#include <ldap.h>
#include <lber.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/tokenizer.hpp>
#include <boost/progress.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/classads/classad_utils.h"

#include "ldap-dn-utils-g2.h"
#include "schema_utils-g2.h"

namespace cu = glite::wmsutils::classads;
namespace ba = boost::algorithm;
namespace ism = glite::wms::ism;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

typedef boost::shared_ptr<classad::ClassAd> ClassAdPtr;

class LDAPException: public std::exception
{
  std::string m_error;
public:
  LDAPException(std::string const& error)
    : m_error(error)
  {
  }
  ~LDAPException() throw()
  {
  }
  virtual char const* what() const throw()
  {
    return m_error.c_str();
  }
};

namespace {

struct ManagerInfo
{
  ClassAdPtr ad;
};
typedef std::map<std::string, ManagerInfo> ManagerInfoMap;


struct EndpointInfo
{
  ClassAdPtr ad;
};
typedef std::map<std::string, EndpointInfo> EndpointInfoMap;

struct ExecEnvInfo
{
  ClassAdPtr ad;
};
typedef std::map<std::string, ExecEnvInfo> ExecEnvInfoMap;


struct ServiceInfo
{
  ClassAdPtr ad;
  ManagerInfoMap::iterator manager_lnk;
  EndpointInfoMap::iterator endpoint_lnk;
};

typedef std::map<std::string, ServiceInfo> ServiceInfoMap;

struct ShareInfo
{
  ClassAdPtr ad;
  std::vector<classad::ExprTree*> policy_rules;
  ServiceInfoMap::iterator service_lnk;
  ExecEnvInfoMap::iterator execenv_lnk; // We are assuming that a share is bound 
					// to one execution environment
					// This limitation should be removed asap
  //std::vector<ClassAdPtr> ses_binds;
};

typedef std::map<std::string, ShareInfo> ShareInfoMap;

//typedef std::map<std::string, ClassAdPtr> SubClusterInfoMap;
/*
struct ClusterInfo {
  std::string site_id;
  std::vector<CEInfoMap::iterator> ces_links;
  std::vector<SubClusterInfoMap::iterator> subclusters_links;
};
typedef std::map<std::string, ClusterInfo> ClusterInfoMap;

struct VoViewInfo
{
  VoViewInfo(std::string const& i, ClassAdPtr a) 
    : id(i), ad(a)
  {
  }
  std::string id;
  ClassAdPtr ad; 
};

typedef std::map<
  std::string, // ce id
  std::vector<VoViewInfo> // list of ce views per vo
> VoViewInfoMap;

struct SEInfo
{
  ClassAdPtr ad;
  std::vector<classad::ExprTree*> storage_areas;
  std::vector<classad::ExprTree*> control_protocols;
  std::vector<classad::ExprTree*> access_protocols;
  std::vector<ClassAdPtr> ces_binds;
};

typedef std::map<std::string, SEInfo> SEInfoMap;

*/

struct BDIICEInfo
{
  ShareInfoMap shares;
  ServiceInfoMap services;
  ManagerInfoMap managers;
  EndpointInfoMap endpoints;
  ExecEnvInfoMap execenvs;
};

void process_glue2_service_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const service_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  ServiceInfoMap::iterator it;
  bool insert;
  boost::tie(it, insert) = bdii_info.services.insert(
    std::make_pair(service_id, ServiceInfo())
  );
  it->second.ad.reset(new classad::ClassAd()); 
  it->second.ad->Insert(
    "Service",
    ad->Copy()
  );
}


void process_glue2_manager_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const manager_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const service_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );
  ManagerInfoMap::iterator manager_it;
  bool insert;
  boost::tie(manager_it, insert) = bdii_info.managers.insert(
    std::make_pair(manager_id, ManagerInfo())
  );
  manager_it->second.ad.reset(new classad::ClassAd());
  manager_it->second.ad->Insert(
    "Manager",
    ad->Copy()
  );
   
  ServiceInfoMap::iterator service_it;
  boost::tie(service_it, insert) = bdii_info.services.insert(
    std::make_pair(service_id, ServiceInfo())
  );
  service_it->second.manager_lnk = manager_it;
}

struct is_a_literal_node_starting_with
{
  is_a_literal_node_starting_with(std::string const& p) : prefix(p) {}
  bool operator()(classad::ExprTree* e) const {
    classad::Value v;
    std::string s;
    return (
      cu::is_literal(e) && 
      e->Evaluate(v) && v.IsStringValue(s) && ba::starts_with(s,prefix)
    );
  }
  std::string prefix;
};

void extract_glue2_info(ClassAdPtr ad, std::string const& from, std::string const& what)
{
  classad::ExprList* el = dynamic_cast<classad::ExprList*>(
    ad->Lookup(from)
  );
  std::vector<classad::ExprTree*>::const_iterator it(
    std::find_if(
      el->begin(), el->end(),
      is_a_literal_node_starting_with(what+"=")
    )
  );
  if (it != el->end()) {
    classad::Value v;
    std::string s;
    (*it)->Evaluate(v) && v.IsStringValue(s);
    ad->InsertAttr(what,s.substr(s.find("=")+1));
  }
}

void process_glue2_share_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const share_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const service_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );
  ShareInfoMap::iterator share_it;
  bool insert;
  boost::tie(share_it, insert) = bdii_info.shares.insert(
    std::make_pair(share_id, ShareInfo())
  );

  extract_glue2_info(ad, "OtherInfo", "CREAMCEId");
 
  share_it->second.ad.reset(new classad::ClassAd());
  share_it->second.ad->Insert(
    "Share",
    ad->Copy()
  );
  ServiceInfoMap::iterator service_it;
  boost::tie(service_it, insert) = bdii_info.services.insert(
    std::make_pair(service_id, ServiceInfo())
  );
  share_it->second.service_lnk = service_it;

  classad::ExprList* el = dynamic_cast<classad::ExprList*>(
    ad->Lookup("ResourceForeignKey")
  );
  if (el && el->size()) {
    classad::Value v;
    std::string s;
    (*el->begin())->Evaluate(v) && v.IsStringValue(s);

    bool insert;
    ExecEnvInfoMap::iterator execenv_it;
    boost::tie(execenv_it, insert) = bdii_info.execenvs.insert(
      std::make_pair(s, ExecEnvInfo())
    );
    share_it->second.execenv_lnk = execenv_it;
  }
}

void process_glue2_endpoint_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const endpoint_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const service_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );
  EndpointInfoMap::iterator endpoint_it;
  bool insert;
  boost::tie(endpoint_it, insert) = bdii_info.endpoints.insert(
    std::make_pair(endpoint_id, EndpointInfo())
  );

  endpoint_it->second.ad.reset(new classad::ClassAd());
  endpoint_it->second.ad->Insert(
    "Endpoint",
    ad->Copy()
  );
  ServiceInfoMap::iterator service_it;
  boost::tie(service_it, insert) = bdii_info.services.insert(
    std::make_pair(service_id, ServiceInfo())
  );
  service_it->second.endpoint_lnk = endpoint_it;
}

void process_glue2_resource_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const resource_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const service_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );

  bool insert;
  ExecEnvInfoMap::iterator execenv_it;
  boost::tie(execenv_it, insert) = bdii_info.execenvs.insert(
    std::make_pair(resource_id, ExecEnvInfo())
  );

  execenv_it->second.ad.reset(new classad::ClassAd());
  execenv_it->second.ad->Insert(
    "ExecutionEnvironment",
    ad->Copy()
  );
}

void process_glue2_policy_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const share_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );
  // Policy rules are inserted directly into the relevant ShareInfo
  bool insert;
  ShareInfoMap::iterator share_it;
  boost::tie(share_it, insert) = bdii_info.shares.insert(
    std::make_pair(share_id, ShareInfo())
  );
  classad::ExprList* el = dynamic_cast<classad::ExprList*>(
    ad->Lookup("Rule")
  );
  std::transform(
    el->begin(), el->end(),
    std::inserter(
      share_it->second.policy_rules, 
      share_it->second.policy_rules.end()
    ),
    boost::bind(&classad::ExprTree::Copy, _1)
  );
}

inline bool iequals(std::string const& a, std::string const& b)
{
 return ba::iequals(a,b);
}

inline bool istarts_with(std::string const& a, std::string const& b)
{
 return ba::istarts_with(a,b);
}

inline std::string strip_prefix(std::string const& prefix, std::string const& s)
{
  return boost::algorithm::istarts_with(s,prefix) ?
    boost::algorithm::erase_head_copy(s, prefix.length()) : s;
}

void insert_values(
  std::string const& name,
  boost::shared_array<struct berval*> values,
  std::list<std::string> const& prefix,
  classad::ClassAd& ad
) {
  std::vector<std::string>::const_iterator list_b, number_b;
  std::vector<std::string>::const_iterator list_e, number_e;
    
  boost::tie(list_b, list_e) = bdii_schema_info_g2().multi_valued();
  boost::tie(number_b, number_e) = bdii_schema_info_g2().number_valued();

  bool is_list = ba::iequals(name,"objectclass") || std::find_if(
    list_b, list_e, boost::bind(iequals,_1, name)
  ) != list_e;

  bool is_number = std::find_if(
    number_b, number_e, boost::bind(iequals, _1, name)
  ) != number_e; 

  std::string result;
  for (size_t i=0; values[i] != 0; ++i) {

    if (i) result.append(",");
    if (is_number || ba::iequals("undefined", values[i]->bv_val) || 
      ba::iequals("false", values[i]->bv_val) || ba::iequals("true", values[i]->bv_val)) {  
    
      result.append(values[i]->bv_val);
    }
    else {
      result.append("\"").append(values[i]->bv_val).append("\"");
    }
  }
  if (is_list) {
    std::string s("{");
    s.append(result).append("}");
    result.swap(s);
  }
  classad::ExprTree* e = 0;
  classad::ClassAdParser parser;
  parser.ParseExpression(result, e);

  if (e) {
    std::list<std::string>::const_iterator it =  std::find_if(
      prefix.begin(), prefix.end(), 
      boost::bind(istarts_with, name, _1)
    );
    ad.Insert(
      ( it != prefix.end() ) ? strip_prefix(*it, name) : name,
      e
      );
  }
}
classad::ClassAd*
create_classad_from_ldap_entry(
  LDAP* ld, LDAPMessage* lde,
  std::list<std::string> prefix
) {
  classad::ClassAd* result = new classad::ClassAd;
  BerElement* ber = 0;
  for(
    char* attr = ldap_first_attribute(ld, lde, &ber);
    attr; attr = ldap_next_attribute(ld, lde, ber)
  ) {
    boost::shared_ptr<void> attr_guard(
      attr, ber_memfree
    );
    boost::shared_array<struct berval*> values(
      ldap_get_values_len(ld, lde, attr),
      ldap_value_free_len
    );

    if (!values) continue;
    insert_values( 
      attr, values, prefix, *result
    );
  }
  boost::shared_ptr<void> ber_guard(
    static_cast<void*>(0),boost::bind(ber_free, ber, 0)
  );
  return result;
}
} // anonymous namespace

void 
fetch_bdii_se_info_g2(
  std::string const& host, size_t port, std::string const& dn, 
  time_t timeout, ism::purchaser::PurchaserInfoContainer& se_info_container) 
{
}

void 
fetch_bdii_ce_info_g2(
  std::string const& host, size_t port, std::string const& dn, 
  time_t timeout, std::string const& ldap_ce_filter_ext,
  ism::purchaser::PurchaserInfoContainer& ce_info_container) 
{
  std::string filter("(|"
    "(&(objectclass=GLUE2ComputingService)(|(GLUE2ServiceType=org.glite.ce.ARC)(GLUE2ServiceType=org.glite.ce.CREAM)))(|"
    "(objectclass=GLUE2ComputingManager)(|"
    "(objectclass=GLUE2ComputingShare)(|"
    "(&(objectclass=GLUE2ComputingEndPoint)(GLUE2EndpointInterfaceName=org.glite.ce.CREAM))(|"
    "(objectclass=GLUE2ToStorageService)(|"
    "(&(objectclass=GLUE2MappingPolicy)(GLUE2PolicyScheme=org.glite.standard))(|"
    "(objectclass=GLUE2ExecutionEnvironment)(|"
    "(objectclass=GLUE2ApplicationEnvironment)(|"
    "(objectclass=GLUE2Benchmark)))))))))"
    ")"
  );

  LDAP* ld = 0;
  int result = ldap_initialize(
    &ld, 
    std::string("ldap://" + host + ":" + boost::lexical_cast<std::string>(port)).c_str()
  );
  boost::shared_ptr<void> ld_guard(
    static_cast<void*>(0), 
    boost::bind(ldap_unbind_ext, ld, (LDAPControl**)(0), (LDAPControl**)(0))
  );
  if (result != LDAP_SUCCESS ) {

    throw LDAPException(
      std::string("ldap_simple_bind_s error: ").append(
        ldap_err2string(result)
      )
    );
  }

  struct timeval to;
  to.tv_sec = timeout;
  to.tv_usec = 0L;

  ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &to);

  LDAPMessage *ldresult = 0;
  if ( (result = ldap_search_ext_s(
    ld,
    dn.c_str(),
    LDAP_SCOPE_SUBTREE,
    filter.c_str(),
    0,
    false,
    0,
    0,
    &to,
    LDAP_NO_LIMIT,
    &ldresult) != LDAP_SUCCESS)) {

    throw LDAPException(
      std::string("ldap_search error: ").append(
        ldap_err2string(result)
      )
    );
  }

  boost::shared_ptr<void> ldresult_guard(
    ldresult, ldap_msgfree
  );

  BDIICEInfo bdii_info;

  time_t const t0 = std::time(0);
  size_t n_entries = 0;
  for (
    LDAPMessage* lde = ldap_first_entry(ld, ldresult);
    lde != 0; lde = ldap_next_entry(ld, lde)
  ) {

    boost::shared_ptr<char> dn_str(
      ldap_get_dn( ld, lde ),
      ber_memfree
    );
    ++n_entries;
    std::vector<std::string> ldap_dn_tokens;
    tokenize_ldap_dn(dn_str.get(), ldap_dn_tokens);
    
    if (is_glue2_service_dn(ldap_dn_tokens)) {
        
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
          ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Service")
      ));
      process_glue2_service_info(ldap_dn_tokens, ad, bdii_info);
    } 
    else if (is_glue2_manager_dn(ldap_dn_tokens)) {
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
          ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Manager")("GLUE2ComputingManager")
      ));
      process_glue2_manager_info(ldap_dn_tokens, ad, bdii_info);
    }
    else if (is_glue2_share_dn(ldap_dn_tokens)) {
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
          ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Share")("GLUE2ComputingShare")
      ));
      process_glue2_share_info(ldap_dn_tokens, ad, bdii_info);
    }
    else if (is_glue2_endpoint_dn(ldap_dn_tokens)) {
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
          ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Endpoint")("GLUE2ComputingEndpoint")
      ));
      process_glue2_endpoint_info(ldap_dn_tokens, ad, bdii_info);
    }
    else if (is_glue2_resource_dn(ldap_dn_tokens)) {
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
          ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Resource")("GLUE2ExecutionEnvironment")
      ));
      process_glue2_resource_info(ldap_dn_tokens, ad, bdii_info);
    }
    else if (is_glue2_policy_dn(ldap_dn_tokens)) {
      ClassAdPtr ad(
        create_classad_from_ldap_entry(
        ld, lde, boost::assign::list_of("GLUE2Entity")("GLUE2Policy")("GLUE2MappingPolicy")
      ));
      process_glue2_policy_info(ldap_dn_tokens, ad, bdii_info);
    }


  }
  Debug("#" << n_entries << " LDAP entries received in " << std::time(0) - t0 << " seconds");

  ShareInfoMap::const_iterator sh_it(bdii_info.shares.begin());
  ShareInfoMap::const_iterator const sh_e(bdii_info.shares.end());

  time_t const t1 = std::time(0);
  for( ; sh_it != sh_e; ++sh_it) { 

    classad::ClassAd* computingAd = new classad::ClassAd;

    computingAd->Update(*sh_it->second.ad);
    computingAd->Update(*sh_it->second.service_lnk->second.ad); //service
    computingAd->Update(*sh_it->second.service_lnk->second.manager_lnk->second.ad); // manager
    computingAd->Update(*sh_it->second.service_lnk->second.endpoint_lnk->second.ad); // endpoint
    computingAd->DeepInsert(
       computingAd->Lookup("Share"),
      "Policy",
      classad::ExprList::MakeExprList(sh_it->second.policy_rules) // policies for the share
    );
/*
    std::vector<classad::ExprTree*> el;
    
    std::vector<EndpointInfoMap::iterator>::const_iterator ep_it(
      sh_it->second.service_lnk->second.endpoints_lnk.begin()
    );
    std::vector<EndpointInfoMap::iterator>::const_iterator const ep_e(
      sh_it->second.service_lnk->second.endpoints_lnk.end()
    );
    
    for( ; ep_it != ep_e; ++ep_it) {
       el.push_back((*ep_it)->second.ad->Copy());
    }
     computingAd->Insert("Endpoint", classad::ExprList::MakeExprList(el)); 
*/
    ClassAdPtr glue2Ad(new classad::ClassAd);
    glue2Ad->Insert("Computing", computingAd);
    glue2Ad->Update(*sh_it->second.execenv_lnk->second.ad);
    
   Debug(">" << *glue2Ad); 
    
  }
}

void fetch_bdii_info_g2(
  std::string const& hostname,
  size_t port,
  std::string const& dn,
  time_t timeout,
  std::string const& ldap_ce_filter_ext,
  ism::purchaser::PurchaserInfoContainer& ce_info_container,
  ism::purchaser::PurchaserInfoContainer& se_info_container)
{
  fetch_bdii_ce_info_g2(hostname, port, dn, timeout, ldap_ce_filter_ext, ce_info_container);
  fetch_bdii_se_info_g2(hostname, port, dn, timeout, se_info_container);
}
}}}}


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
#include <boost/any.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_inserter.hpp>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/classads/classad_utils.h"

#include "ldap-dn-utils-g2.h"
#include "schema_utils-g2.h"

namespace cu  = glite::wmsutils::classads;
namespace ba  = boost::algorithm;
namespace bas = boost::assign;
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

struct ServiceInfo;
typedef std::map<std::string, ServiceInfo> ServiceInfoMap;

struct ShareInfo;
typedef std::map<std::string, ShareInfo> ShareInfoMap;

struct AccessPolicyInfo {
  ClassAdPtr ad;
};
typedef std::map<std::string, AccessPolicyInfo> AccessPolicyInfoMap;

struct EndpointInfo
{
  ClassAdPtr ad;
  std::vector<classad::ExprTree*> policy_rules;
  ServiceInfoMap::iterator service_lnk;
  AccessPolicyInfoMap::iterator policy_lnk;
  std::vector<ShareInfoMap::iterator> shares_lnk;
};
typedef std::map<std::string, EndpointInfo> EndpointInfoMap;

struct BenchMarkInfo
{
  ClassAdPtr ad;
};
typedef std::map<std::string, BenchMarkInfo> BenchMarkInfoMap;

struct ExecEnvInfo
{
  ClassAdPtr ad;
  std::set<std::string> applications;
  std::vector<BenchMarkInfoMap::iterator> bmarks_lnk;
};

typedef std::map<std::string, ExecEnvInfo> ExecEnvInfoMap;

struct ServiceInfo
{
  ClassAdPtr ad;
  ManagerInfoMap::iterator manager_lnk;
};

struct ShareInfo
{
  ClassAdPtr ad;
  std::vector<classad::ExprTree*> policy_rules;
  ExecEnvInfoMap::iterator execenv_lnk; // We are assuming that a share is bound 
					// to one execution environment
					// This limitation should be removed asap
};

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
  AccessPolicyInfoMap policies;
  ExecEnvInfoMap execenvs;
  BenchMarkInfoMap benchmarks;
};

inline void cleanup_glue2_info(ClassAdPtr ad, std::list<std::string> a)
{
  a.push_back("objectClass");
  std::for_each(
   a.begin(), a.end(),
   boost::bind(&classad::ClassAd::Delete, ad, _1)
  );
}

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
  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("AdminDomainForeignKey")
      ("OtherInfo")
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
  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("ComputingServiceForeignKey")
      ("ServiceForeignKey")
      ("OtherInfo")
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
template<typename T>
void extract_glue2_info_value_in(ClassAdPtr ad, std::string const& from, std::string const& what, classad::ClassAd* in)
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
    in->InsertAttr(what,boost::lexical_cast<T>(s.substr(s.find("=")+1)));
  }
}
template<typename T>
void extract_glue2_info_value(ClassAdPtr ad, std::string const& from, std::string const& what)
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
    ad->InsertAttr(what,boost::lexical_cast<T>(s.substr(s.find("=")+1)));
  }
}


void process_glue2_resource_fk(ShareInfoMap::iterator& share_it, ClassAdPtr ad, BDIICEInfo& bdii_info)
{
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

EndpointInfoMap::iterator 
process_glue2_endpoint_fk_entry(BDIICEInfo& bdii_info, classad::ExprTree* e)
{
  classad::Value v;
  std::string s;
  EndpointInfoMap::iterator endpoint_it(
    bdii_info.endpoints.end()
  );
  if (e->Evaluate(v) && v.IsStringValue(s)) {
    bool insert;
    boost::tie(endpoint_it, insert) = bdii_info.endpoints.insert(
      std::make_pair(s, EndpointInfo())
    );
  }
  return endpoint_it;
}

void process_glue2_endpoint_fk(ShareInfoMap::iterator& share_it, ClassAdPtr ad, BDIICEInfo& bdii_info)
{
  classad::ExprList* el = dynamic_cast<classad::ExprList*>(
    ad->Lookup("ComputingEndpointForeignKey")
  );
  if (el && el->size()) {
    for( // link each endpoint to the share it binds to 
      classad::ExprList::iterator el_it = el->begin() ;
      el_it != el->end(); ++el_it 
    ) {
      EndpointInfoMap::iterator endpoint_it(
        process_glue2_endpoint_fk_entry(bdii_info, *el_it)
      );
      if (endpoint_it != bdii_info.endpoints.end()) {
         endpoint_it->second.shares_lnk.push_back(share_it);
      }
    }
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
  process_glue2_resource_fk(share_it, ad, bdii_info);
  process_glue2_endpoint_fk(share_it, ad, bdii_info);
  
  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("ComputingEndpointForeignKey")
      ("ComputingServiceForeignKey")
      ("EndpointForeignKey")
      ("ResourceForeignKey")
      ("ServiceForeignKey")
      ("ExecutionEnvironmentForeignKey")
  );
 
  share_it->second.ad.reset(new classad::ClassAd());
  share_it->second.ad->Insert(
    "Share",
    ad->Copy()
  );
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
  classad::ClassAd* oi = new classad::ClassAd;
  extract_glue2_info_value_in<std::string>(ad, "OtherInfo", "HostDN", oi);
  extract_glue2_info_value_in<std::string>(ad, "OtherInfo", "MiddlewareName", oi);
  extract_glue2_info_value_in<std::string>(ad, "OtherInfo", "MiddlewareVersion", oi);
  ad->Insert("OtherInfo", oi);

  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("ComputingServiceForeignKey")
      ("ServiceForeignKey")
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
  endpoint_it->second.service_lnk = service_it;
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
  classad::ClassAd* oi = new classad::ClassAd;
  extract_glue2_info_value_in<short>(ad, "OtherInfo", "SmpSize", oi);
  extract_glue2_info_value_in<short>(ad, "OtherInfo", "Cores", oi);
  ad->Insert("OtherInfo", oi);

  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("ComputingManagerForeignKey")
      ("ManagerForeignKey")
  );
 
  execenv_it->second.ad.reset(new classad::ClassAd());
  execenv_it->second.ad->Insert(
    "ExecutionEnvironment",
    ad->Copy()
  );
}

void process_glue2_application_env_info(
  std::vector<std::string> const& ldap_dn_tokens,
  ClassAdPtr ad,
  BDIICEInfo& bdii_info
)
{
  std::string const appenv_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const resource_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );

  std::string application =
    cu::evaluate_attribute(*ad,"AppName");

  bool insert;
  ExecEnvInfoMap::iterator execenv_it;
  boost::tie(execenv_it, insert) = bdii_info.execenvs.insert(
    std::make_pair(resource_id, ExecEnvInfo())
  );

  execenv_it->second.applications.insert(application);
}

void process_glue2_benchmark_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const bmark_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const resource_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );

  bool insert;
  BenchMarkInfoMap::iterator bmark_it;
  boost::tie(bmark_it, insert) = bdii_info.benchmarks.insert(
    std::make_pair(bmark_id, BenchMarkInfo())
  );

  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("ComputingManagerForeignKey")
      ("ExecutionEnvironmentForeignKey")
      ("OtherInfo")
  );
  // The schema defines the BenchmarkValue to be a 32-bit float
  // However LDAP is rather deficient in data types and the only 
  // numeric type is integer, so it has to be stored as a string. 
  try {
    std::string v = cu::evaluate_attribute(*ad, "Value");
    ad->InsertAttr("Value", boost::lexical_cast<float>(v));
  } catch(boost::bad_lexical_cast &) {}
  
  bmark_it->second.ad = ad;

  ExecEnvInfoMap::iterator execenv_it;
  boost::tie(execenv_it, insert) = bdii_info.execenvs.insert(
    std::make_pair(resource_id, ExecEnvInfo())
  );
  execenv_it->second.bmarks_lnk.push_back(bmark_it);
}


void process_glue2_mapping_policy_info(
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

void process_glue2_access_policy_info(
  std::vector<std::string> const& ldap_dn_tokens, 
  ClassAdPtr ad, 
  BDIICEInfo& bdii_info
) 
{
  std::string const policy_id(
   ldap_dn_tokens[0].substr(ldap_dn_tokens[0].find("=")+1)
  );
  std::string const endpoint_id(
   ldap_dn_tokens[1].substr(ldap_dn_tokens[1].find("=")+1)
  );
  bool insert;
  AccessPolicyInfoMap::iterator policy_it;
  boost::tie(policy_it, insert) = bdii_info.policies.insert(
    std::make_pair(policy_id, AccessPolicyInfo())
  );

  cleanup_glue2_info(ad,
    bas::list_of("CreationTime")
      ("UserDomainForeignKey")
      ("EndpointForeignKey")
      ("OtherInfo")
  );

  policy_it->second.ad = ad;

  EndpointInfoMap::iterator endpoint_it;
  boost::tie(endpoint_it, insert) = bdii_info.endpoints.insert(
    std::make_pair(endpoint_id, EndpointInfo())
  );
  endpoint_it->second.policy_lnk = policy_it;
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

inline classad::ExprTree* make_literal(std::string const& s)
{
  classad::Value v;
  v.SetStringValue(s);
  return classad::Literal::MakeLiteral(v);
}

inline classad::ExprTree* parse_expression(std::string const& s)
{
  classad::ClassAdParser parser;
  return parser.ParseExpression(s);
}
inline classad::ExprTree* copy_benchmark_info(BenchMarkInfoMap::const_iterator bi)
{
  return dynamic_cast<classad::ExprTree*>(bi->second.ad->Copy());
}
 
typedef std::list<std::string> glue2_stripping_prefix;
typedef boost::function<
  bool(std::vector<std::string> const&)
> glue2_dn_test_predicate;

typedef boost::function<void(
  std::vector<std::string> const&, 
  ClassAdPtr,
  BDIICEInfo&
)> glue2_info_processing_fn;

typedef boost::tuple<
  glue2_dn_test_predicate,
  glue2_stripping_prefix,
  glue2_info_processing_fn
> glue2_info_processor_tuple;

const glue2_stripping_prefix 
  service_pfx  = bas::list_of("GLUE2Entity")("GLUE2Service"),
  manager_pfx  = bas::list_of("GLUE2Entity")("GLUE2Manager")("GLUE2ComputingManager"),
  share_pfx    = bas::list_of("GLUE2Entity")("GLUE2Share")("GLUE2ComputingShare"),
  endpoint_pfx = bas::list_of("GLUE2Entity")("GLUE2Endpoint")("GLUE2ComputingEndpoint"),
  resource_pfx = bas::list_of("GLUE2Entity")("GLUE2Resource")("GLUE2ExecutionEnvironment"),
  mpolicy_pfx  = bas::list_of("GLUE2Entity")("GLUE2Policy")("GLUE2MappingPolicy"),
  apolicy_pfx  = bas::list_of("GLUE2Entity")("GLUE2Policy")("GLUE2AccessPolicy"),
  appenv_pfx   = bas::list_of("GLUE2Entity")("GLUE2ApplicationEnvironment"),
  bmark_pfx    = bas::list_of("GLUE2Entity")("GLUE2Benchmark");

std::list<glue2_info_processor_tuple> glue2_info_processors =
  bas::tuple_list_of(
    is_glue2_service_dn, service_pfx,
    process_glue2_service_info
  )(
    is_glue2_manager_dn, manager_pfx,
    process_glue2_manager_info
  )(
    is_glue2_share_dn, share_pfx,
    process_glue2_share_info
  )(
    is_glue2_endpoint_dn, endpoint_pfx,
    process_glue2_endpoint_info
  )(
    is_glue2_resource_dn, resource_pfx,
    process_glue2_resource_info
  )(
    is_glue2_mapping_policy_dn, mpolicy_pfx,
    process_glue2_mapping_policy_info
  )(
    is_glue2_access_policy_dn, apolicy_pfx,
    process_glue2_access_policy_info
  )(
    is_glue2_application_env_dn, appenv_pfx,
    process_glue2_application_env_info
  )(
    is_glue2_benchmark_dn, bmark_pfx,
    process_glue2_benchmark_info
  );

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
    "(&(objectclass=GLUE2AccessPolicy)(GLUE2PolicyScheme=org.glite.standard))(|"
    "(objectclass=GLUE2ExecutionEnvironment)(|"
    "(objectclass=GLUE2ApplicationEnvironment)(|"
    "(objectclass=GLUE2Benchmark))))))))))"
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
    for (
      std::list<glue2_info_processor_tuple>::const_iterator p_it = glue2_info_processors.begin() ;
      p_it != glue2_info_processors.end() ; ++p_it ) {
    
      glue2_dn_test_predicate check = p_it->get<0>();
      
      if (check(ldap_dn_tokens)) {
        
        ClassAdPtr ad(
          create_classad_from_ldap_entry(
            ld, lde, p_it->get<1>()
        ));
        glue2_info_processing_fn process = p_it->get<2>();
        process(ldap_dn_tokens, ad, bdii_info);
        break ; 
      } 
    }
  }
  Debug("#" << n_entries << " LDAP entries received in " << std::time(0) - t0 << " seconds");

  EndpointInfoMap::const_iterator ep_it(bdii_info.endpoints.begin());
  EndpointInfoMap::const_iterator const ep_e(bdii_info.endpoints.end());
  Debug(bdii_info.endpoints.size());
  time_t const t1 = std::time(0);
  size_t n_shares = 0;
  for( ; ep_it != ep_e; ++ep_it) { 

    if ( ep_it->second.shares_lnk.empty() ) { // Not an Endpoint bound to Shares
       continue;
    }
    ClassAdPtr computingAd(new classad::ClassAd);

    computingAd->Update(*ep_it->second.ad); 
    computingAd->Update(*ep_it->second.service_lnk->second.ad); 
    computingAd->Update(*ep_it->second.service_lnk->second.manager_lnk->second.ad);

    computingAd->DeepInsert(
      computingAd->Lookup("Endpoint"),
      "Policy",
       ep_it->second.policy_lnk->second.ad->Lookup("Rule")->Copy()
    ); 

   std::vector<ShareInfoMap::iterator>::const_iterator sh_it(
      ep_it->second.shares_lnk.begin()
    );
    std::vector<ShareInfoMap::iterator>::const_iterator const sh_e(
      ep_it->second.shares_lnk.end()
    );

    for( ; sh_it != sh_e; ++sh_it) {

      classad::ClassAd* computingAd_copy(
        dynamic_cast<classad::ClassAd*>(computingAd->Copy())
      );
      computingAd_copy->Update(*(*sh_it)->second.ad);

      std::vector<classad::ExprTree*> apps_el;
      std::transform(
        (*sh_it)->second.execenv_lnk->second.applications.begin(),
        (*sh_it)->second.execenv_lnk->second.applications.end(),
        std::back_inserter(apps_el),
        make_literal
      ); 
      classad::ClassAd* appenvAd = new classad::ClassAd();
      appenvAd->Insert("AppName", classad::ExprList::MakeExprList(apps_el));

      std::vector<classad::ExprTree*> bmark_ets;
      std::transform(
        (*sh_it)->second.execenv_lnk->second.bmarks_lnk.begin(),
        (*sh_it)->second.execenv_lnk->second.bmarks_lnk.end(),
        std::back_inserter(bmark_ets),
        copy_benchmark_info
       ); 

      classad::ClassAd* g2Ad = new classad::ClassAd;

      g2Ad->Insert("Benchmark", classad::ExprList::MakeExprList(bmark_ets));
      g2Ad->Insert("ApplicationEnvironment", appenvAd);
      g2Ad->Update(*(*sh_it)->second.execenv_lnk->second.ad);
      g2Ad->Insert("Computing", computingAd_copy);
      g2Ad->DeepInsert(
        computingAd_copy->Lookup("Share"),
        "Policy", 
        classad::ExprList::MakeExprList((*sh_it)->second.policy_rules)
      );
      std::string interface_name = cu::evaluate_expression(
        *ep_it->second.ad, "EndPoint.InterfaceName"
      );
      
      classad::ExprList* el;
      dynamic_cast<classad::ClassAd*>(computingAd_copy->Lookup("Share"))->
        EvaluateAttrList("OtherInfo", el);

      std::vector<classad::ExprTree*>::const_iterator oi_it(
        std::find_if(el->begin(), el->end(),
          is_a_literal_node_starting_with(
            "CREAMCEId=" + ba::erase_first_copy(ep_it->first, "_"+interface_name)
          )
        )
      );
      if (oi_it != el->end()) {
        classad::Value v;
        std::string s;
        (*oi_it)->Evaluate(v) && v.IsStringValue(s);
        g2Ad->DeepInsertAttr(
          computingAd_copy->Lookup("Share"),
          "CREAMCEId",
          s.substr(s.find("=")+1)
        );
      }
      ClassAdPtr result( new classad::ClassAd );
      result->Insert("GLUE2", g2Ad);
      result->Insert("AuthorizationCheck", parse_expression(
        "("
        "  member(other.CertificateSubject,GLUE2.Computing.Endpoint.Policy) ||  "
        "  member(strcat(\"VO:\",other.VirtualOrganisation),GLUE2.Computing.Endpoint.Policy) || "
        "  FQANmember(strcat(\"VOMS:\",other.VOMS_FQAN ),GLUE2.Computing.Endpoint.Policy) "
        ") && "
        "! FQANmember(strcat(\"DENY:\",other.VOMS_FQAN),GLUE2.Computing.Endpoint.Policy)"
      ));
      result->Insert("requirements", parse_expression(
        "AuthorizationCheck"
      ));
      n_shares++;
      std::string id = cu::evaluate_expression(*result,
       "IsUndefined(GLUE2.Computing.Share.CREAMCEId) ?"
        "    GLUE2.Computing.Share.ID :  GLUE2.Computing.Share.CREAMCEId"
      );
      std::string policy = cu::evaluate_expression(*result,"GLUE2.Computing.Share.Policy[0]");
      std::string glue13Id = id+"/"+policy.substr(policy.find(":")+1);

      // Required for backward compatibility by Helper
      result->Insert(
        "QueueName", 
        parse_expression("GLUE2.Computing.Share.MappingQueue")
      );
      result->Insert(
        "LRMSType",
        parse_expression("GLUE2.Computing.Manager.ProductName")
      );
      result->InsertAttr(
        "CEid", glue13Id
      );
      ce_info_container.insert(std::make_pair(glue13Id, result));
    }   
  }
  Debug("#" << n_shares << " GLUE2 shares ClassAd generated in " << std::time(0) - t1 << " seconds");
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


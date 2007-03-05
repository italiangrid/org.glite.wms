// File: gpbox_utils.cpp
// Author: Marco Cecchi
//         Francesco Giacomini
//         Francesco Grimaldi
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

#ifndef GLITE_WMS_DONT_HAVE_GPBOX
#include <map>
#include <unistd.h>
#include <sys/types.h>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/progress.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <openssl/pem.h>
#include <openssl/x509.h>

#include "glite/gpbox/pep_connection.h"

#include "glite/lb/producer.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/matchmaking/matchmaker.h"

#include "glite/wmsutils/classads/classad_utils.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/security/proxyrenewal/renewal.h"
#include "glite/security/voms/voms_api.h"

#include "gpbox_utils.h"
#include "globus_gss_assist.h"

#include "glite/lb/context.h"
#include "glite/lb/consumer.h"
#include "classad_distribution.h"

namespace jobid = glite::wmsutils::jobid;
namespace configuration = glite::wms::common::configuration;
namespace classadutils = glite::wmsutils::classads;
namespace pep = glite::gpbox::pep;

namespace glite {
namespace wms {
namespace helper {
namespace broker {
namespace gpbox {

namespace {

static std::string const service_class_tag = "SC:";
static std::string const not_a_service_class_tag = "NOT_A_SC";

std::string
get_user_x509_proxy(jobid::JobId const& jobid,
  configuration::Configuration const& config)
{
  static std::string const null_string;
  char* c_x509_proxy = NULL;

  int err_code = glite_renewal_GetProxy(jobid.toString().c_str(), &c_x509_proxy);
  if (!err_code) {
    return null_string;
  } else {
    // currently no proxy is registered if renewal is not requested
    // try to get the original user proxy from the input sandbox
    boost::shared_ptr<char> _c_x509_proxy(c_x509_proxy, ::free);

    const configuration::NSConfiguration* ns_config = config.ns();
    assert(ns_config);

    std::string x509_proxy(ns_config->sandbox_staging_path());
    x509_proxy += "/"
    + jobid::get_reduced_part(jobid)
    + "/"
    + jobid::to_filename(jobid)
    + "/user.proxy";

    return x509_proxy;
  }
}

std::string
get_proxy_distinguished_name(std::string const& proxy_file)
{
  static std::string const null_string;

  std::FILE* rfd = std::fopen(proxy_file.c_str(), "r");
  if (!rfd) {
    return null_string;
  }
  boost::shared_ptr<std::FILE> fd(rfd, std::fclose);

  ::X509* rcert = ::PEM_read_X509(rfd, 0, 0, 0);
  if (!rcert) {
    return null_string;
  }
  boost::shared_ptr<X509> cert(rcert, ::X509_free);

  ::X509_NAME* name = ::X509_get_subject_name(rcert);
  if (!name) {
    return null_string;
  }

  char* cp = ::X509_NAME_oneline(name, NULL, 0);
  if (!cp) {
    return null_string;
  }
  boost::shared_ptr<char> cp_(cp, ::free);

  return std::string(cp);
}

STACK_OF(X509) *
load_chain(const char *certfile)
{
  STACK_OF(X509_INFO) *sk = NULL;
  STACK_OF(X509) *stack = NULL;
  BIO *in = NULL;
  X509_INFO *xi;
  int first = 1;

  if (!(stack = sk_X509_new_null())) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  if (!(in = BIO_new_file(certfile, "r"))) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  boost::shared_ptr<BIO> in_(in, ::BIO_free);

  // This loads from a file, a stack of x509/crl/pkey sets
  if (!(sk = ::PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  // scan over it and pull out the certs
  while (sk_X509_INFO_num(sk)) {
    // skip first cert
    if (first) {
      first = 0;
      continue;
    }
    xi = sk_X509_INFO_shift(sk);
    boost::shared_ptr<X509_INFO> _xi(xi, ::X509_INFO_free);
    if (xi->x509 != NULL) {
      sk_X509_push(stack,xi->x509);
      xi->x509 = NULL;
    }
  }
  if (!sk_X509_num(stack)) {
    Debug("no certificates in file");
    sk_X509_free(stack);
    sk_X509_INFO_free(sk);

    return NULL;
  }
  return stack;
}

bool
VOMS_proxy_init(
  const std::string& user_cert_file_name, 
  pep::Attributes& USER_attribs)
{
  vomsdata v;
  X509 *x = NULL;
  BIO *in = NULL;
  STACK_OF(X509) *chain = NULL;

  in = BIO_new(BIO_s_file());
  boost::shared_ptr<BIO> in_(in, ::BIO_free);

  if (in) {
    if (BIO_read_filename(in, user_cert_file_name.c_str()) > 0) {
      x = PEM_read_bio_X509(in, NULL, 0, NULL);
      chain = load_chain(user_cert_file_name.c_str());
      if (x && chain) {
        if (v.Retrieve(x, chain, RECURSE_CHAIN)) {
          voms vomsdefault;
          v.DefaultData(vomsdefault);
          bool marked_first_user_attrib = false;
          USER_attribs.push_back(pep::Attribute("voname", vomsdefault.voname));
          for (std::vector<voms>::iterator i = v.data.begin(); i != v.data.end(); i++) {
            for(std::vector<data>::iterator j = (*i).std.begin(); j != (*i).std.end(); j++) {
              std::string name = (*j).group;
              if ((*j).role != std::string("NULL")) {
                name += "/Role=" + (*j).role;
              }
              if (!marked_first_user_attrib) {
                USER_attribs.push_back(pep::Attribute("primary_group", name));
                marked_first_user_attrib = true;
              }
              USER_attribs.push_back(pep::Attribute("group", name));
            }
          }
        }
        else {
          Debug("VOMS error: " + v.ErrorMessage());
          return false;
        }
      }
      else {
        Debug("VOMS error: " + v.ErrorMessage());
        return false;
      }
    }

    if (x) {
      X509_free(x);
    }
    if (chain) {
      sk_X509_free(chain);
    }

    return true;
  }
  else {
    return false;
  }
}

bool
is_service_class(std::string attribute_value)
{
  boost::regex const service_class(service_class_tag + ".+");
  return boost::regex_match(attribute_value, service_class);
}

std::string
get_tag(matchmaking::matchinfo const& info)
{
  static std::string const null_string;

  classad::ClassAd const* ad = matchmaking::getAd(info).get();
  std::vector<std::string> acbr_vector;
  classadutils::EvaluateAttrList(*ad, "GlueCEAccessControlBaseRule", acbr_vector);

  //TODO: by now we simply look for the 'SC:' tag indicating  by convention a
  //service class. Afterwards, the value has to be passed 'as is' each time
  //it's not a grouping tag (by now VO: always by convention)
  std::vector<std::string>::iterator const it_end = acbr_vector.end();
  std::vector<std::string>::iterator const it = std::find_if(
    acbr_vector.begin(),
    it_end,
    is_service_class
  );

  if (it != it_end)
  { 
    return it->substr(service_class_tag.size());
  } else {
    return not_a_service_class_tag;
  }
}

//std::string
//get_CE_unique_id(matchmaking::match_info const& info)
//{
//  classad::ClassAd const* ad = info.getAd();
//  classad::Value value;
//
//  ad->EvaluateExpr("GlueCEUniqueID", value);
//  std::string result;
//  value.IsStringValue(result);
//
//  return result;
//}

bool
filter_gpbox_authorizations(
  matchmaking::matchtable& suitable_CEs,
  Connection& PEP_connection,
  std::string const& user_cert_file_name
)
{
  if (user_cert_file_name.empty()) {
    Debug("gpbox: cannot find user certificate");
    return false;
  }

  const std::string user_subject(
    get_proxy_distinguished_name(user_cert_file_name)
  );

  if(user_subject.empty()) {
    Debug("gpbox: empty user subject");
    return false;
  }

  pep::Attributes CE_attributes;
  std::string ce_names;
  std::string ce_tags;
  matchmaking::matchtable::iterator it = suitable_CEs.begin();
  matchmaking::matchtable::iterator const end = suitable_CEs.end();
  for (;
       it != end;
       ++it) {
    ce_names += it->first + '#';
    std::string tag(get_tag(it->second));
    ce_tags += tag.empty() ? "error" : tag + '#';
  }
  ce_names.erase(ce_names.size() - 1);
  ce_tags.erase(ce_tags.size() - 1);

  Info(ce_names);
  Info(ce_tags);

  CE_attributes.push_back(pep::Attribute("aggregation-tag", ce_tags));

  try {

    pep::Attributes CE_attributes;
    pep::Attributes SUB_attributes;

    pep::Action const action("job-submission");

    pep::Request request(ce_names, action, user_subject);
    request.attributes(CE_attributes, pep::Request::RESOURCE);

    if(VOMS_proxy_init(user_cert_file_name,USER_attribs)) {
      request.attributes(SUB_attributes, pep::Request::SUBJECT);
      //PEP_request.SetAttr(USER_attribs, SUBJ);

      pep::Responses resp = PEP_connection.query(request);
      for (Responses::const_iterator it = resp.begin(); it != resp.end(); ++it)
      {
        Info(it->resource());
        if (
          it->decision() == pep::Answer::DENY ||
          it->decision() == pep::Answer::NOT_APPLICABLE ||
          it->decision() == pep::Answer::INDETERMINATE
        ) {
          suitable_CEs.erase(it->resource());
          Info("!!!erased CE");
        }
      }
    }
    else {
      Debug("VOMS_proxy_init returned false");
      return false;
    }
  } catch(...) {
    Debug("filter_gbox_authorizations: PEP Send returned false");
    return false;
  }

  return true;
}

} //empty  namespace

bool
interact(
  configuration::Configuration const& config,
  jobid::JobId const& jobid,
  std::string const& PBOX_host_name,
  matchmaking::matchtable& suitable_CEs   
  )
{
  return interact(config,
    get_user_x509_proxy(jobid, config),
    PBOX_host_name,
    suitable_CEs
  );
}

bool
interact(
  configuration::Configuration const& config,
  std::string const& x509_user_proxy,
  std::string const& PBOX_host_name,
  matchmaking::matchtable& suitable_CEs)
{
  boost::timer perf_timer;

  const configuration::CommonConfiguration* common_conf = config.common();
  assert(common_conf);

  const std::string broker_subject(
    get_proxy_distinguished_name(common_conf->host_proxy_file())
  );

  const configuration::WMConfiguration* wm_conf = config.wm();
  assert(wm_conf);

  if (!broker_subject.empty()) {
    try {
      Connection PEP_connection(
                                PBOX_host_name,
                                wm_conf->pbox_port_num(),
                                broker_subject,
                                wm_conf->pbox_safe_mode()
                               );

      if (!filter_gpbox_authorizations(suitable_CEs,
                                       PEP_connection,
                                       x509_user_proxy)) {
        return false;
      }
    }
    catch (...) { // exception no_conn from API
                  // PEP_connection not properly propagated
      Debug("gpbox: exception caught during interaction");
      return false;
    };
 
    std::string end_msg(
      "gpbox interaction ended. Elapsed: "
      +
      boost::lexical_cast<std::string>(perf_timer.elapsed())
    );
    Info(end_msg);

    return true;
  }
  else {
    Debug("gpbox: unable to find the broker proxy certificate");
    return false;
  }
}

}}}}} //glite::wms::helper::broker::gpbox
#endif

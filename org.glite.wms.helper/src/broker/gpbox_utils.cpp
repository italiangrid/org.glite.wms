// File: gpbox_utils.cpp
// Author: Marco Cecchi
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

#ifndef GLITE_WMS_DONT_HAVE_GPBOX
#include <sys/types.h>
#include <unistd.h>
#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/lb/producer.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "glite/wms/matchmaking/matchmaker.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"

#include "glite/wms/common/logger/logger_utils.h"
#include "gpbox_utils.h"
#include "globus_gss_assist.h"
#include "glite/gpbox/Clientcc.h"
#include "glite/security/proxyrenewal/renewal.h"
#include "glite/security/voms/voms_api.h"

#include "glite/lb/context.h"
#include "glite/lb/consumer.h"
#include "classad_distribution.h"

namespace jobid = glite::wmsutils::jobid;
namespace configuration = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace helper {
namespace broker {
namespace gpbox {

namespace {

std::string
get_user_x509_proxy(jobid::JobId const& jobid)
{
  static std::string const null_string;
  char* c_x509_proxy = NULL;

  int err_code = glite_renewal_GetProxy(jobid.toString().c_str(), &c_x509_proxy);
  if (!err_code) {

    return null_string;
  }
  else {
    // currently no proxy is registered if renewal is not requested
    // try to get the original user proxy from the input sandbox
    boost::shared_ptr<char> _c_x509_proxy(c_x509_proxy, ::free);

    configuration::Configuration const* const config
      = configuration::Configuration::instance();
    assert(config);

    configuration::NSConfiguration const* const ns_config = config->ns();
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

  if(!(stack = sk_X509_new_null())) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  if(!(in=BIO_new_file(certfile, "r"))) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  boost::shared_ptr<BIO> _in(in, ::BIO_free);

  // This loads from a file, a stack of x509/crl/pkey sets
  if(!(sk = ::PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  // scan over it and pull out the certs
  while (sk_X509_INFO_num(sk)) {
    /* skip first cert */
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
  if(!sk_X509_num(stack)) {
    Info("no certificates in file");
    sk_X509_free(stack);
    sk_X509_INFO_free(sk);
    return NULL;
  }
  return stack;
}

bool
VOMS_proxy_init(
  const std::string& user_cert_file_name, 
  Attributes& USER_attribs)
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
          USER_attribs.push_back(Attribute("voname", vomsdefault.voname, STRING));
          for (std::vector<voms>::iterator i = v.data.begin(); i != v.data.end(); i++)
          {
            for(std::vector<data>::iterator j = (*i).std.begin(); j != (*i).std.end(); ++j)
            {
              std::string name = (*j).group;
              if ((*j).role != std::string("NULL")) {
                     name += "/Role=" + (*j).role;
              }
              USER_attribs.push_back(Attribute("group", name, STRING));
            }
          }
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }

    if (x)
      X509_free(x);
    if (chain)
      sk_X509_free(chain);

    return true;
  }
  else {
    return false;
  }
}

std::string
get_tag(matchmaking::match_info const& info)
{
  static std::string const null_string;

  classad::ClassAd const* ad = info.getAd();
  classad::Value value;

  ad->EvaluateExpr("GlueCEPolicyPriority", value);
  std::string result;
  value.IsStringValue(result);
  if (result.empty()) {
    // sometimes this attribute is published as string (as it should),
    // sometimes as int, so we need to handle this.

    int int_result;
    value.IsIntegerValue(int_result);
    result = boost::lexical_cast<std::string>(int_result);
  }
  return result;
}

bool
filter_gpbox_authorizations(
  matchmaking::match_table_t& suitable_CEs,
  Connection& PEP_connection,
  std::string const& user_cert_file_name
)
{
  if(user_cert_file_name.empty())
    return false;

  const std::string user_subject(
    get_proxy_distinguished_name(user_cert_file_name)
  );
  if(user_subject.empty())
    return false;

  Attributes CE_attributes;
  std::string ce_names;
  std::string ce_tags;

  for (matchmaking::match_table_t::iterator it = suitable_CEs.begin();
       it != suitable_CEs.end();
       ++it) {
    ce_names += it->first + '#';
    std::string tag(get_tag(it->second));
    ce_tags += tag.empty() ? "-1" : tag + '#';
  }
  if (!ce_names.empty())
    ce_names.erase(ce_names.size() - 1);
  if (!ce_tags.empty())
    ce_tags.erase(ce_tags.size() - 1);

  Info(ce_names);
  Info(ce_tags);

  CE_attributes.push_back(Attribute("aggregation-tag", ce_tags, STRING));

  try {

    PEPClient PEP_request(ce_names, "job-submission", user_subject);
    PEP_request.Attach(&PEP_connection);
    PEP_request.SetAttr(CE_attributes, RES);
    Attributes USER_attribs;
    PEP_request.SetAttr(USER_attribs, SUBJ);

    if(VOMS_proxy_init(user_cert_file_name,USER_attribs)) {
      PEP_request.SetAttr(USER_attribs, SUBJ);

      EvalResults evaluation_of_results;
      static std::string const null_string;

      if( PEP_request.Send(null_string, 0, 0, 0, evaluation_of_results) ) {
        Info("filter_gbox_authorizations: PEP Send returned true");
        for (EvalResults::iterator iter = evaluation_of_results.begin();
          iter != evaluation_of_results.end();
          ++iter) {

          answer PEP_request_answer = iter->GetResult();
          // INDET may even be returned from an exception coming to the API
          // Send, so we don't even check it (since the policy (permit) is
          // according)), its content being untrustable

          Info(iter->GetId());
          // NOTE: borderline cases are filtered off without questioning
          // because of the resubmission costs
          if( PEP_request_answer == DENY
              or
              PEP_request_answer == NOTA
              or 
              PEP_request_answer == INDET ) {
            suitable_CEs.erase(iter->GetId());
            Info("!!!erased CE");
          }
        }
      }
      else {
        return false;
      }
    }
    else {
      Info("VOMS_proxy_init returned false");
      return false;
    }
  }
  catch(...) {
    Info("filter_gbox_authorizations: PEP Send returned false");
    return false;
  }

  return true;
}
} //empty  namespace

bool
interact(
  configuration::Configuration const& config,
  jobid::JobId const& jobid,
  matchmaking::match_table_t& suitable_CEs   
  )
{
  return interact(config, get_user_x509_proxy(jobid), suitable_CEs);
}

bool
interact(
  configuration::Configuration const& config,
  std::string const& x509_user_proxy,
  matchmaking::match_table_t& suitable_CEs)
{
  boost::timer perf_timer;

  const configuration::CommonConfiguration* common_conf = config.common();
  assert(common_conf);

  const std::string broker_subject(
    get_proxy_distinguished_name(common_conf->host_proxy_file())
  );

  const configuration::WMConfiguration* WM_conf = config.wm();
  assert(WM_conf);

  std::string Pbox_host_name(WM_conf->pbox_host_name());
  if( !broker_subject.empty() and !Pbox_host_name.empty() ) {
    try {

      Connection PEP_connection(
                                Pbox_host_name,
                                WM_conf->pbox_port_num(),
                                broker_subject,
                                WM_conf->pbox_safe_mode()
                               );

      if (!filter_gpbox_authorizations(suitable_CEs,
                                       PEP_connection,
                                       x509_user_proxy)) {
        return false;
      }
    }
    catch (...) { // exception no_conn from API
                  // PEP_connection not properly propagated
      return false;
    };
  }
  else {
    Info("gpbox: unable to find the broker proxy certificate or gpbox host name not specified");
  }

  Info("END G-Pbox. Elapsed:");
  Info(perf_timer.elapsed());
  return true;
}

}}}}} //glite::wms::helper::broker::gpbox
#endif

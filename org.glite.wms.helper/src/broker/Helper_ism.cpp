// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <stdexcept>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>

#include <classad_distribution.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/bio.h>

#include "Helper.h"
#include "globus_gss_assist.h"

#include "glite/gpbox/Clientcc.h"

#include "glite/security/proxyrenewal/renewal.h"
#include "glite/security/voms/voms_api.h"

#include "glite/wms/broker/RBSimpleISMImpl.h"
#include "glite/wms/broker/RBMaximizeFilesISMImpl.h"

#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

#include "glite/wms/classad_plugin/classad_plugin_loader.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/helper/HelperFactory.h"
#include "glite/wms/helper/broker/exceptions.h"
#include "glite/wms/helper/exceptions.h"

#include "glite/wms/ism/ism.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/matchmaking/exceptions.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

namespace fs            = boost::filesystem;
namespace jobid         = glite::wmsutils::jobid;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace requestad     = glite::wms::jdl;
namespace utilities     = glite::wms::common::utilities;
namespace matchmaking   = glite::wms::matchmaking;

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

//#define GPBOXDEBUG 1
#ifndef GPBOXDEBUG
  #define print(msg) Info(msg)
#else
  #define print(msg) std::cout << msg << '\n'
#endif

namespace glite {
namespace wms {
namespace helper {
namespace broker {

namespace {

typedef glite::wms::brokerinfo::BrokerInfo<glite::wms::brokerinfo::brokerinfoGlueImpl> BrokerInfo;

std::string const helper_id("BrokerHelper");

void dump_suitable_CEs( boost::scoped_ptr<matchmaking::match_table_t> const& suitable_CEs ) {
  #ifdef GPBOXDEBUG
    for (matchmaking::match_table_t::const_iterator it = suitable_CEs->begin();
         it != suitable_CEs->end();
         ++it) {
      print(it->first);
    }
  #endif
}

std::string
get_user_x509_proxy(jobid::JobId const& jobid)
{
  static std::string const null_string;
  char* c_x509_proxy = NULL;

  int err_code = edg_wlpr_GetProxy(jobid.getId(), &c_x509_proxy);

  if (err_code == 0) {

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
    try {
      result = boost::lexical_cast<std::string>(int_result);
    }
    catch (boost::bad_lexical_cast const&) {
      return(null_string);
    }
  }
  return result;
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
    xi=sk_X509_INFO_shift(sk);
    boost::shared_ptr<X509_INFO> _xi(xi, ::X509_INFO_free);
    if (xi->x509 != NULL) {
      sk_X509_push(stack,xi->x509);
      xi->x509 = NULL;
    }
  }
  if(!sk_X509_num(stack)) {
print("no certificates in file");
    sk_X509_free(stack);
    sk_X509_INFO_free(sk);
    return NULL;
  }

  return stack;
}

bool
VOMS_proxy_init(const std::string& user_cert_file_name, Attributes& USER_attribs)
{ 
  vomsdata v;
  X509 *x = NULL;
  BIO *in = NULL;
  STACK_OF(X509) *chain = NULL;

  in = BIO_new(BIO_s_file());
  boost::shared_ptr<BIO> _in(in, ::BIO_free);

  if (in) {
    if (BIO_read_filename(in, user_cert_file_name.c_str()) > 0) {
      x = PEM_read_bio_X509(in, NULL, 0, NULL);
      chain = load_chain(user_cert_file_name.c_str());
      if (x && chain) {
        if (v.Retrieve(x, chain, RECURSE_CHAIN)) {
          voms vomsdefault;
          v.DefaultData(vomsdefault);
          USER_attribs.push_back(Attribute("voname", vomsdefault.voname, STRING));
          for (std::vector<voms>::iterator i = v.data.begin(); i != v.data.end(); i++) {
	         for(std::vector<data>::iterator j = (*i).std.begin(); j != (*i).std.end(); j++) {
	           std::string name = (*j).group;
	           if ((*j).role != std::string("NULL"))
	             name += "/Role=" + (*j).role;
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

//FIXME: should be moved back to ckassad_plugin if/when only this DL is used
//FIXME: for matchmaking.
glite::wms::classad_plugin::classad_plugin_loader init;

helper::HelperImpl* create_helper()
{
  return new Helper;
}

struct Register
{
  Register()
  {
    helper::HelperFactory::instance()->register_helper(helper_id, create_helper);
  }
  ~Register()
  {
    helper::HelperFactory::instance()->unregister_helper(helper_id);
  }
};

Register const r;

std::string const f_output_file_suffix(".rbh");

std::auto_ptr<classad::ClassAd>
f_resolve_simple(classad::ClassAd const& input_ad, std::string const& ce_id)
{
  std::auto_ptr<classad::ClassAd> result;

  static boost::regex  expression( "(.+/[^\\-]+-([^\\-]+))-(.+)" );
  boost::smatch        pieces;
  std::string gcrs, type, name;
  if (boost::regex_match(ce_id, pieces, expression)) {
    gcrs.assign(pieces[1].first, pieces[1].second);
    type.assign(pieces[2].first, pieces[2].second);
    name.assign(pieces[3].first, pieces[3].second);

    result.reset(new classad::ClassAd(input_ad));
    requestad::set_globus_resource_contact_string(*result, gcrs);
    requestad::set_queue_name(*result, name);
    try {
      std::string junk = requestad::get_lrms_type(*result);
    } catch ( glite::wms::jdl::CannotGetAttribute const& e) {
      requestad::set_lrms_type(*result, type);
    }
   
    requestad::set_ce_id(*result, ce_id);

    // TODO catch requestad::CannotSetAttribute
  } else {
    throw helper::InvalidAttributeValue(requestad::JDL::SUBMIT_TO,
                                        ce_id,
                                        "match (.+/[^\\-]+-([^\\-]+))-(.+)",
                                        helper_id);
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

print(user_subject);

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

print(ce_names);
print(ce_tags);

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
print("filter_gbox_authorizations: PEP Send returned true");
 	     for (EvalResults::iterator iter = evaluation_of_results.begin(); 
             iter != evaluation_of_results.end(); 
             ++iter) {

          answer PEP_request_answer = iter->GetResult();
          // INDET may even be returned from an exception coming to the API 
          // Send, so we don't even check it (since the policy (permit) is
          // according)), its content being untrustable

print(iter->GetId());
          // IMPORTANT: NOTA means that G-Pbox cannot match the request with any found policy
          // hence this will result in a DENY whilst PERMIT and UNDET are passed on 
          if(PEP_request_answer == DENY or PEP_request_answer == NOTA) {
            suitable_CEs.erase(iter->GetId());
print("!!!erased CE");
          }
        }
      }
      else {
        return false;
      }
    }
    else {
print("VOMS_proxy_init returned false");
      return false;
    }
  }
  catch(...) {
print("filter_gbox_authorizations: PEP Send returned false");
    return false;
  }

  return true;
} 

std::auto_ptr<classad::ClassAd>
f_resolve_mm(classad::ClassAd const& input_ad)
try {
  std::auto_ptr<classad::ClassAd> result;
  std::auto_ptr<glite::wms::broker::ResourceBrokerImpl> rb_impl;

  std::string vo(requestad::get_virtual_organisation(input_ad));

  boost::scoped_ptr<BrokerInfo> BI(new BrokerInfo);

  bool input_data_exists = false;
  std::vector<std::string> input_data;
  requestad::get_input_data(input_ad, input_data, input_data_exists);
  if (input_data_exists) {
    rb_impl.reset(new glite::wms::broker::RBMaximizeFilesISMImpl(BI.get()));
  } 
  else {
    rb_impl.reset(new glite::wms::broker::RBSimpleISMImpl());
  }

  glite::wms::broker::ResourceBroker rb(rb_impl.release());

  // If fuzzy_rank is true in the request ad we have
  // to use the stochastic selector...
  bool use_fuzzy_rank = false;
  if (requestad::get_fuzzy_rank(input_ad, use_fuzzy_rank) && use_fuzzy_rank) {
    rb.changeSelector("stochasticRankSelector");
  }

  boost::scoped_ptr<matchmaking::match_table_t> suitable_CEs(rb.findSuitableCEs(&input_ad));
  if (suitable_CEs->empty()) {
    throw NoCompatibleCEs();
  }

  std::string dg_jobid_str(requestad::get_edg_jobid(input_ad));
  jobid::JobId dg_jobid(dg_jobid_str);

  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  // Start interaction with G-Pbox
  boost::timer perf_timer;
dump_suitable_CEs(suitable_CEs);
print("Start G-Pbox interaction");
perf_timer.restart();

  const configuration::CommonConfiguration* common_conf = config->common();
  assert(common_conf);

  const std::string broker_subject(
    get_proxy_distinguished_name(common_conf->host_proxy_file())
  );

  const configuration::WMConfiguration* WM_conf = config->wm();
  assert(WM_conf);

  std::string Pbox_host_name(WM_conf->pbox_host_name());
  if( !broker_subject.empty() and !Pbox_host_name.empty() ) {
    try {

print(Pbox_host_name);
print(WM_conf->pbox_port_num());
print(WM_conf->pbox_safe_mode());

      Connection PEP_connection(
                                Pbox_host_name,
                                WM_conf->pbox_port_num(),
                                broker_subject,
                                WM_conf->pbox_safe_mode()
                               );

print("gpbox: connection open");

      if (!filter_gpbox_authorizations(*suitable_CEs, 
                                       PEP_connection, 
                                       get_user_x509_proxy(dg_jobid))) {
        //TODO
      }
    }
    catch (...) { // exception no_conn from API 
                  // PEP_connection not properly propagated
print("gpbox: no connection!!!");
      // no connection to the Pbox server, the RB goes on 
      // without screening the list of suitable CEs
    }; //try
  }
  else {
    print("gpbox: unable to find the broker proxy certificate or gpbox host name not specified");
  }

print("END G-Pbox:");
print(perf_timer.elapsed());
dump_suitable_CEs(suitable_CEs);
  if (suitable_CEs->empty()) {
print("Empty CE list after G-Pbox screening");
    throw NoCompatibleCEs();
  }
  // End of G-Pbox interaction

  matchmaking::match_const_iterator ce_it = rb.selectBestCE(*suitable_CEs);
  
  // update the brokerinfo
  BI->retrieveCloseSEsInfo(ce_it->first); // CE id
  BI->retrieveCloseSAsInfo(vo); // Retrieve only GlueSAAvailableVOSpace

  configuration::NSConfiguration const* const ns_config = config->ns();
  assert(ns_config);

  fs::path p(ns_config->sandbox_staging_path(), fs::native);

  p /= jobid::get_reduced_part(dg_jobid);
  p /= jobid::to_filename(dg_jobid);

  fs::path BIfile(
                  (p / "input/.BrokerInfo").native_file_string(), 
                   fs::portable_posix_name
                  );
  std::ofstream BIfilestream(BIfile.native_file_string().c_str());

  if (!BIfilestream) {
    throw CannotCreateBrokerinfo(BIfile);
  }

  boost::scoped_ptr<classad::ClassAd> biAd(BI->asClassAd());
  classad::ExprTree const* DACexpr = input_ad.Lookup("DataAccessProtocol");
  if (DACexpr) {
    biAd->Insert("DataAccessProtocol", DACexpr->Copy());
  }
  BIfilestream << *biAd << std::endl;

  if (!BIfilestream) {
    throw CannotCreateBrokerinfo(BIfile);
  }

  result.reset(new classad::ClassAd(input_ad));

  // Add the .Brokerinfo files to the InputSandbox
  bool input_sandbox_exists = false;
  bool wmpinput_sandbox_base_uri_exists = false;

  std::vector<std::string> ISB;

  requestad::get_input_sandbox(input_ad, ISB, input_sandbox_exists);
  std::string WMPInputSandboxBaseURI(requestad::get_wmpinput_sandbox_base_uri(
                                             input_ad, 
                                             wmpinput_sandbox_base_uri_exists)
                                    );

  if (wmpinput_sandbox_base_uri_exists) 
    ISB.push_back(WMPInputSandboxBaseURI+"/input/.BrokerInfo");
  else 
    ISB.push_back(".BrokerInfo");
  
  requestad::set_input_sandbox(*result, ISB);

  requestad::set_ce_id(*result, ce_it->first);
  matchmaking::match_info const& ce_info = ce_it->second;
  classad::ClassAd const* ce_ad = ce_info.getAd();

  try {

    requestad::set_globus_resource_contact_string(
      *result,
      utilities::evaluate_attribute(*ce_ad, "GlobusResourceContactString")
    );
    requestad::set_queue_name(
      *result,
      utilities::evaluate_attribute(*ce_ad, "QueueName")
    );
    requestad::set_lrms_type(
      *result,
      utilities::evaluate_attribute(*ce_ad, "LRMSType")
    );
    requestad::set_ce_id(
      *result,
      utilities::evaluate_attribute(*ce_ad, "CEid")
    );

  } catch (utilities::InvalidValue const& e) {

    edglog(error) << e.what() << " for CE id " << ce_it->first << std::endl;

    throw helper::HelperError("BrokerHelper");

  }

  return result;

} catch (matchmaking::ISNoResultError const& e) {

  throw NoAvailableCEs(
    "The user is not authorized on any resource currently registered in "
    + e.host()
  );

} catch (matchmaking::InformationServiceError const& e) {

  throw NoAvailableCEs(e.what());
  
} catch (matchmaking::RankingError const& e) {
  
  throw NoAvailableCEs(e.what());
	
} catch (requestad::CannotGetAttribute const& e) {

  throw helper::CannotGetAttribute(e, helper_id);

} catch (requestad::CannotSetAttribute const& e) {

  throw helper::CannotSetAttribute(e, helper_id);

} catch( jobid::JobIdException& jide ) {

  edglog( error ) << jide.what() << std::endl;
  throw helper::InvalidAttributeValue(requestad::JDL::JOBID,
                                      "unknown",
                                      "valid jobid",
                                      helper_id);

} catch( fs::filesystem_error& fse ) {

    edglog( error ) << fse.what() << std::endl;
    throw helper::FileSystemError(helper_id, fse);

}

} // {anonymous}

std::string
Helper::id() const
{
  return helper_id;
}

std::string
Helper::output_file_suffix() const
{
  return f_output_file_suffix;
}

classad::ClassAd*
Helper::resolve(classad::ClassAd const* input_ad) const
{
  bool submit_to_exists = false;
  std::string ce_id = requestad::get_submit_to(*input_ad, submit_to_exists);

  std::auto_ptr<classad::ClassAd> result
    = submit_to_exists ? f_resolve_simple(*input_ad, ce_id) 
                       : f_resolve_mm(*input_ad);

  return result.release();
}

}}}} // edg::workload::planning::broker

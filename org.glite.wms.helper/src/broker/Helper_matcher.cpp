// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/regex.hpp>

#include <classad_distribution.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/bio.h>

#include "Helper_matcher.h"
#include "globus_gss_assist.h"

#include "glite/gpbox/Clientcc.h"

#include "glite/wms/helper/broker/exceptions.h"
#include "glite/wms/helper/exceptions.h"
#include "glite/wms/helper/HelperFactory.h"

#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"
#include "glite/wms/brokerinfo/brokerinfo.h"

#ifdef MATCHER_HELPER_USE_ISM
#include "glite/wms/broker/RBSimpleISMImpl.h"
#include "glite/wms/broker/RBMaximizeFilesISMImpl.h"
#else
#include "glite/wms/broker/RBSimpleImpl.h"
#include "glite/wms/broker/RBMaximizeFilesImpl.h"
#include "glite/wms/broker/RBMinimizeAccessCostImpl.h"
#endif

#include "glite/wms/matchmaking/exceptions.h"
#include "glite/wms/matchmaking/utility.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/security/proxyrenewal/renewal.h"
#include "glite/security/voms/voms_api.h"

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
namespace matcher {

namespace {

typedef glite::wms::brokerinfo::BrokerInfo<glite::wms::brokerinfo::brokerinfoGlueImpl> BrokerInfo;

std::string const helper_id("MatcherHelper");

void
dump_suitable_CEs( boost::scoped_ptr<matchmaking::match_table_t> const& suitable_CEs ) {
  #ifdef GPBOXDEBUG
    for (matchmaking::match_table_t::const_iterator it = suitable_CEs->begin();
         it != suitable_CEs->end();
         ++it) {
      print(it->first);
    }
  #endif
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
    return false;
  }

  return true;
} 

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

/* Answer format:
 * 1) if no error occurs
 * [
 *   reason =ok;
 *   match_result={
 *                  [
 *                    ce_id=...;
 *                    rank=...;
 *                    brokerinfo=...;
 *                  ],
 *                  [...],
 *                  ...
 *                }
 * ]
 * 2) if error occurs
 * [
 *   reason = error reason;
 *   match_result={}
 * ]
 */

std::auto_ptr<classad::ClassAd>
f_resolve_do_match(classad::ClassAd const& input_ad)
{
  std::auto_ptr<classad::ClassAd> result;
  result.reset(new classad::ClassAd);

  result->InsertAttr("reason", std::string("no matching resources found"));
  result->Insert("match_result", new classad::ExprList);
    
#ifdef MATCHER_HELPER_USE_ISM
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleISMImpl());
#elif MATCHER_HELPER_PREFETCH_INFO_FROM_II
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleImpl(true));
#else
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleImpl());
#endif

  bool input_data_exists = false;
  std::vector<std::string> input_data;
  requestad::get_input_data(input_ad, input_data, input_data_exists);

  boost::scoped_ptr<BrokerInfo> BI(new BrokerInfo);
  
  if (input_data_exists) {
    // Here we have to check if the rank expression in the request
    // is rank = other.dataAccessCost and change the implementation
    // of the broker (RBMinimizeAccessCost)
    classad::ExprTree* rank_expr = input_ad.Lookup("rank");
    if (rank_expr) {
      std::vector<std::string> rankAttributes;
      utilities::insertAttributeInVector(&rankAttributes, rank_expr, utilities::is_reference_to("other"));
      if (rankAttributes.size() == 1 
          && rankAttributes.front() == "DataAccessCost") {
      // RBMinimizeAccessCostImpl doesn't rank based on the
      // Info Service. So the use_cached_info_for_gris 
      // flag isn't needed.
#ifndef MATCHER_HELPER_USE_ISM
        rb.changeImplementation(new glite::wms::broker::RBMinimizeAccessCostImpl(BI.get()));
#endif
      } else {
#ifdef MATCHER_HELPER_USE_ISM
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesISMImpl(BI.get(),true));
#elif MATCHER_HELPER_PREFETCH_INFO_FROM_II
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesImpl(BI.get(),true));
#else
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesImpl(BI.get()));
#endif
      }
    }
  }

  boost::scoped_ptr<matchmaking::match_table_t> suitableCEs;
  std::string mm_error;
print("Joblistmatch");
  
  try {
    suitableCEs.reset(rb.findSuitableCEs(&input_ad));

    std::string vo(requestad::get_virtual_organisation(input_ad));    
    std::vector<classad::ExprTree*> hosts;
    std::string x509_user_proxy_file_name(requestad::get_x509_user_proxy(input_ad)); 
print(x509_user_proxy_file_name);

    // Start interaction with G-Pbox
    if ( !suitableCEs->empty() ) {
      configuration::Configuration const* const config
        = configuration::Configuration::instance();
      assert(config);

      boost::timer perf_timer;
dump_suitable_CEs(suitableCEs);
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

          if (!filter_gpbox_authorizations(*suitableCEs, 
                                           PEP_connection, 
                                           x509_user_proxy_file_name)) {
            //TODO
          }
        }
        catch (...) { // exception no_conn from API 
                      // PEP_connection not properly propagated
print("gpbox: no connection!!!");
          // no connection to the Pbox server, the WM goes on 
          // without screening the list of suitable CEs
        }; //try
      }
      else {
        print("gpbox: unable to find the broker proxy certificate or gpbox host name not specified");
      }

print("END G-Pbox");
print(perf_timer.elapsed());
dump_suitable_CEs(suitableCEs);
//END G-Pbox interaction
    } // if ( !suitableCEs->empty() )

    if (!suitableCEs->empty() ) {
      matchmaking::match_vector_t suitableCEs_vector(suitableCEs->begin(), suitableCEs->end());
      
      std::stable_sort(suitableCEs_vector.begin(), suitableCEs_vector.end(), matchmaking::rank_greater_than_comparator());
      
      for (matchmaking::match_vector_t::const_iterator it = suitableCEs_vector.begin(); 
           it != suitableCEs_vector.end(); 
           ++it) {
 
        BI->retrieveCloseSEsInfo(it->first);
        BI->retrieveCloseSAsInfo(vo); // Retrieve only GlueSAAvailableVOSpace
   
        std::auto_ptr<classad::ClassAd> ceinfo(new classad::ClassAd);
        ceinfo->InsertAttr("ce_id", it->first);
        ceinfo->InsertAttr("rank", it->second.getRank());
        ceinfo->Insert("brokerinfo", BI->asClassAd());
      
        hosts.push_back(ceinfo.release());
      }
      
      result->InsertAttr("reason", std::string("ok"));    
      result->Insert("match_result", classad::ExprList::MakeExprList(hosts));           
    }
  } catch (matchmaking::ISNoResultError const& e) {
    result->InsertAttr(
      "reason",
      "The user is not authorized on any resource currently registered in "
      + e.host()
    );

  } catch (matchmaking::InformationServiceError const& e) {
    result->InsertAttr("reason", std::string(e.what()));  

  } catch (matchmaking::RankingError const& e) {
    result->InsertAttr("reason", std::string(e.what()));  
 
  } catch (requestad::CannotGetAttribute const& e) {
    result->InsertAttr(
      "reason", 
      "Attribute " + e.parameter() + " does not exist or has the wrong type"
    ); 

  } catch (requestad::CannotSetAttribute const& e) {
    result->InsertAttr("reason", "Cannot set attribute " + e.parameter());

  } catch (jobid::JobIdException const& e) {
    result->InsertAttr("reason", std::string(e.what()));  

  } catch (boost::filesystem::filesystem_error const& e) {
    result->InsertAttr("reason", std::string(e.what()));  
  }
  return result;
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
  std::auto_ptr<classad::ClassAd> result = f_resolve_do_match(*input_ad);

  return result.release();
}

}}}} // glite::wms::helper::matcher

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>

#include "globus_gss_assist.h"

#include "glite/gpbox/Clientcc.h"
#include "glite/wms/ism/ism.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/matchmaking/exceptions.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
#include "glite/security/proxyrenewal/renewal.h"
#include "glite/security/voms/voms_api.h"


namespace jobid         = glite::wmsutils::jobid;
namespace requestad     = glite::wms::jdl;
namespace configuration = glite::wms::common::configuration;
namespace matchmaking   = glite::wms::matchmaking;

namespace glite {
namespace wms {
namespace helper {
namespace gpbox_utils {


static int
X509_NAME_cmp_no_set( X509_NAME *a, X509_NAME *b)
{
    int             i,j;
    X509_NAME_ENTRY *na,*nb;

    if (sk_X509_NAME_ENTRY_num(a->entries) !=
        sk_X509_NAME_ENTRY_num(b->entries))
    {
        return(sk_X509_NAME_ENTRY_num(a->entries) -
               sk_X509_NAME_ENTRY_num(b->entries));
    }

    for (i=sk_X509_NAME_ENTRY_num(a->entries)-1; i>=0; i--)
    {
        na = sk_X509_NAME_ENTRY_value(a->entries,i);
        nb = sk_X509_NAME_ENTRY_value(b->entries,i);
        j = na->value->length-nb->value->length;

        if (j)
        {
            return(j);
        }

        j = memcmp(na->value->data,
                   nb->value->data,
                   na->value->length);
        if (j)
        {
            return(j);
        }

        /*j=na->set-nb->set; */
        /* if (j) return(j); */
    }

    for (i=sk_X509_NAME_ENTRY_num(a->entries)-1; i>=0; i--)
    {
        na = sk_X509_NAME_ENTRY_value(a->entries,i);
        nb = sk_X509_NAME_ENTRY_value(b->entries,i);
        j = OBJ_cmp(na->object,nb->object);

        if (j)
        {
            return(j);
        }
    }
    return(0);
}

static int 
proxy_check_proxy_name(X509 * cert)
{
    int               ret = 0;
    X509_NAME *       subject;
    X509_NAME *       name = NULL;
    X509_NAME_ENTRY * ne = NULL;
    ASN1_STRING *     data;


    subject = ::X509_get_subject_name(cert);
    ne = ::X509_NAME_get_entry(subject, X509_NAME_entry_count(subject)-1);
    if ( !OBJ_cmp(ne->object,OBJ_nid2obj(NID_commonName)) )
    {
        data = ::X509_NAME_ENTRY_get_data(ne);
        if ((data->length == 5 &&
             !memcmp(data->data,"proxy",5)) ||
            (data->length == 13 &&
             !memcmp(data->data,"limited proxy",13)))
        {

            if (data->length == 13)
            {
                ret = 2; /* its a limited proxy */
            }
            else
            {
                ret = 1; /* its a proxy */
            }

            name = ::X509_NAME_dup(X509_get_issuer_name(cert));
            ne = ::X509_NAME_ENTRY_create_by_NID(NULL,
                                               NID_commonName,
                                               V_ASN1_APP_CHOOSE,
                                               (ret == 2) ?
                                               (unsigned char *)
                                               "limited proxy" :
                                               (unsigned char *)"proxy",
                                               -1);

            ::X509_NAME_add_entry(name,ne,X509_NAME_entry_count(name),0);
            ::X509_NAME_ENTRY_free(ne);
            ne = NULL;

            if ( X509_NAME_cmp_no_set(name, subject) )
            {
                ret = -1;
            }
            ::X509_NAME_free(name);
        }
    }
    return ret;
}


X509 *
get_real_cert(X509 *base, STACK_OF(X509) *stk)
{
  X509 *cert = NULL;
  int i;

  if (!proxy_check_proxy_name(base))
    return base;

  /* Determine id data */
  for (i = 0; i < sk_X509_num(stk); i++) {
    cert = sk_X509_value(stk, i);
    if (!proxy_check_proxy_name(cert)) {
      return (X509 *)ASN1_dup((int (*)())i2d_X509, 
            (char * (*)())d2i_X509, (char *)cert);
    }
  }
  return NULL;
}

std::string
get_user_x509_proxy(jobid::JobId const& jobid)
{
  static std::string const null_string;
  char* c_x509_proxy = NULL;

  int err_code = edg_wlpr_GetProxy(jobid.getId(), &c_x509_proxy);

  if ( err_code == 0 ) {
    return null_string;
  } 
  else {
    // currently no proxy is registered if renewal is not requested
    // try to get the original user proxy from the input sandbox
    boost::shared_ptr<char> c_x509_proxy_(c_x509_proxy, ::free);

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

static std::string 
get_proxy_distinguished_name_from_cert(X509* cert)
{
  static std::string const null_string;
  boost::shared_ptr<X509> cert_(cert, ::X509_free);

  ::X509_NAME* name = ::X509_get_subject_name(cert);
  if ( !name ) {
    return null_string;
  }

  char* cp = ::X509_NAME_oneline(name, NULL, 0);
  if ( !cp ) {
    return null_string;
  }
  boost::shared_ptr<char> cp_(cp, ::free);

  return std::string(cp);
}

std::string 
get_proxy_distinguished_name(std::string const& proxy_file)
{
  static std::string const null_string;
 
  std::FILE* rfd = std::fopen(proxy_file.c_str(), "r");
  if ( !rfd ) {
    return null_string;
  }
  boost::shared_ptr<std::FILE> rfd_(rfd, std::fclose);

  ::X509* rcert = ::PEM_read_X509(rfd, 0, 0, 0);
  if ( !rcert ) {
    return null_string;
  }
  boost::shared_ptr<X509> cert_(rcert, ::X509_free);

  ::X509_NAME* name = ::X509_get_subject_name(rcert);
  if ( !name ) {
    return null_string;
  }

  char* cp = ::X509_NAME_oneline(name, NULL, 0);
  if ( !cp ) {
    return null_string;
  }
  boost::shared_ptr<char> cp_(cp, ::free);

  return std::string(cp);
}

static std::string
get_tag(matchmaking::match_info const& info)
{
  static std::string const null_string;

  classad::ClassAd const* ad = info.getAd();
  classad::Value value;

  ad->EvaluateExpr("GlueCEPolicyPriority", value);
  std::string result;
  value.IsStringValue(result);
  if( result.empty() ) {
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

static STACK_OF(X509) *
load_chain(const char *certfile)
{
  STACK_OF(X509_INFO) *sk = NULL;
  STACK_OF(X509) *stack = NULL;
  BIO *in = NULL;
  X509_INFO *xi;
  int first = 1;

  if( !(stack = sk_X509_new_null()) ) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  if( !(in=BIO_new_file(certfile, "r")) ) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  boost::shared_ptr<BIO> in_(in, ::BIO_free);

  // This loads from a file, a stack of x509/crl/pkey sets
  if( !(sk = ::PEM_X509_INFO_read_bio(in,NULL,NULL,NULL)) ) {
    sk_X509_INFO_free(sk);
    return NULL;
  }
  // scan over it and pull out the certs
  while ( sk_X509_INFO_num(sk) ) {
    /* skip first cert */
    if (first) {
      first = 0;
      continue;
    }
    xi=sk_X509_INFO_shift(sk);
    boost::shared_ptr<X509_INFO> xi_(xi, ::X509_INFO_free);
    if ( xi->x509 != NULL ) {
      sk_X509_push(stack,xi->x509);
      xi->x509 = NULL;
    }
  }
  if( !sk_X509_num(stack) ) {
    Info("no certificates in file");
    sk_X509_free(stack);
    sk_X509_INFO_free(sk);
    return NULL;
  }

  return stack;
}

static bool
VOMS_proxy_init(const std::string& user_cert_file_name, Attributes& USER_attribs, X509 **x_real)
{ 
  vomsdata v;
  X509 *x = NULL;
  BIO *in = NULL;
  STACK_OF(X509) *chain = NULL;

  in = BIO_new(BIO_s_file());
  boost::shared_ptr<BIO> in_(in, ::BIO_free);

  if ( in ) {
    if ( BIO_read_filename(in, user_cert_file_name.c_str()) > 0 ) {
      x = PEM_read_bio_X509(in, NULL, 0, NULL);
      chain = load_chain(user_cert_file_name.c_str());
      if ( x && chain ) {
        if ( v.Retrieve(x, chain, RECURSE_CHAIN) ) {
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
        
          *x_real = get_real_cert(x, chain);
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }

    if ( x )
      X509_free(x);
    if ( chain )
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
  if( user_cert_file_name.empty() ) {
    return false;
  }

  Attributes CE_attributes;
  std::string ce_names;
  std::string ce_tags;

  for( matchmaking::match_table_t::iterator it = suitable_CEs.begin();
       it != suitable_CEs.end();
       ++it ) {
    ce_names += it->first + '#';
    std::string tag(get_tag(it->second)); 
    ce_tags += tag.empty() ? "-1" : tag + '#';
  }
  if ( !ce_names.empty() ) {
    ce_names.erase(ce_names.size() - 1);
  }
  if ( !ce_tags.empty() ) {
    ce_tags.erase(ce_tags.size() - 1);
  }

  Info(ce_names);
  Info(ce_tags);

  CE_attributes.push_back(Attribute("aggregation-tag", ce_tags, STRING));

  Attributes USER_attribs;
  X509 *user_real_cert;

  try {
  
    if( VOMS_proxy_init(user_cert_file_name,USER_attribs, &user_real_cert) ) {

      Info(user_real_cert);

      const std::string user_subject(
        get_proxy_distinguished_name_from_cert(user_real_cert)
      );

      if( user_subject.empty() ) {
        return false;
      }

      Info(user_subject);

      PEPClient PEP_request(ce_names, "job-submission", user_subject);
      PEP_request.Attach(&PEP_connection);
      PEP_request.SetAttr(CE_attributes, RES);
      PEP_request.SetAttr(USER_attribs, SUBJ);

      EvalResults evaluation_of_results;
      static std::string const null_string;

      if( PEP_request.Send(null_string, 0, 0, 0, evaluation_of_results) ) { 

        Info("filter_gbox_authorizations: PEP Send returned true");

        EvalResults::iterator const end_it = evaluation_of_results.end();
 	      for ( EvalResults::iterator iter = evaluation_of_results.begin(); 
        iter != end_it; 
        ++iter) {

          answer PEP_request_answer = iter->GetResult();
          // INDET may even be returned from an exception coming to the API 
          // Send, so we don't even check it (since the policy (permit) is
          // according)), its content being untrustable

          Info(iter->GetId());
          // IMPORTANT: NOTA means that G-Pbox cannot match the request with any found policy
          // hence this will result in a DENY whilst PERMIT and UNDET are passed on 
          if( PEP_request_answer == DENY || PEP_request_answer == NOTA ) {
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
  } catch(...) {
    Info("filter_gbox_authorizations: PEP Send returned false");
    return false;
  }

  return true;
} 

}}}}

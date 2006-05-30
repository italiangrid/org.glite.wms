//
// G-PBox test drive program
//

#include <map>
#include <iostream>
#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/timer.hpp>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/bio.h>

#include <classad_distribution.h>

#include "glite/gpbox/Clientcc.h"
#include "glite/wms/matchmaking/matchmaker.h"

#include "glite/security/voms/voms_api.h"
#include "gssapi.h"
#include "globus_gss_assist.h"

#include "glite/jdl/PrivateAdManipulation.h"

#define print(tag, val) std::cout << tag << ": " << val << '\n';

namespace matchmaking = glite::wms::matchmaking;

class NoCompatibleCEs {};

namespace {

static std::string const service_class_tag = "SC:";
static std::string const not_a_service_class_tag = "NOT_A_SC";
static std::string ACBR_tag = "GlueCEAccessControlBaseRule";

inline void setValue(classad::Value& value, const std::string& s) { value.SetStringValue(s); }
inline void setValue(classad::Value& value, double d) { value.SetRealValue(d); }
inline void setValue(classad::Value& value, bool b) { value.SetBooleanValue(b); }
inline void setValue(classad::Value& value, int i) { value.SetIntegerValue(i); }

template<class T>
classad::ExprList* asExprList(const std::vector<T>& v)
{
  std::vector< classad::ExprTree* > list;
  typename std::vector<T>::const_iterator it;

  for(it = v.begin(); it != v.end(); it++) {
    classad::Value value;
    setValue(value, (*it));
    list.push_back(classad::Literal::MakeLiteral(value));
  }

  classad::ExprList* result = classad::ExprList::MakeExprList(list);

  return result;
}

template<class T>
bool InsertAttrList(classad::ClassAd& ad, const std::string& what, const std::vector<T>&l)
{
  classad::ExprList* expr_list = asExprList(l);

  if (!expr_list)
    return false;
  else
    return ad.Insert(what,expr_list);
}

inline bool is_literal(classad::ExprTree const* t) {
  return t && t->GetKind() == classad::ExprTree::LITERAL_NODE;
}

template<typename _InputIterator, typename _Tp>
_Tp
accumulate(_InputIterator __first, _InputIterator __last, _Tp __init)
{
    for ( ; __first != __last; ++__first)
      __init = __init + *__first;
    return __init;
}

template<typename _InputIterator, typename _Tp, typename _BinaryOperation>
_Tp
accumulate(_InputIterator __first, _InputIterator __last, _Tp __init, _BinaryOperation __binary_op)
{ 
  for ( ; __first != __last; ++__first)
    __init = __binary_op(__init, *__first);
  return __init;
}

inline bool getValue(const classad::Value& value, std::string& s) { return value.IsStringValue(s); }
inline bool getValue(const classad::Value& value, double& d) { return value.IsRealValue(d); }
inline bool getValue(const classad::Value& value, bool& b) { return value.IsBooleanValue(b); }
inline bool getValue(const classad::Value& value, int& i) { return value.IsIntegerValue(i); }

template<class T>
struct InsertExprInVector : public std::binary_function<std::vector<T>*, classad::ExprTree*, std::vector<T>* >
{
  std::vector<T>* operator()(std::vector<T>* v, classad::ExprTree* e)
  {
    if (is_literal(e)) {
      classad::Value value;
      static_cast<classad::Literal*>(e)->GetValue(value);
      T s;
      if (getValue(value, s)) {
        v->push_back(s);
      }
    }
    return v;
  }
};

template<class T>
bool EvaluateAttrList(
  const classad::ClassAd& ad,
  const std::string& what, std::vector<T>&l
)
{
  bool res = false;
  std::string where;
  classad::Value list_value;
  const classad::ExprList *expr_list;

  if( ad.EvaluateAttr(what, list_value) == true &&
      list_value.IsListValue( expr_list ) == true ) {
    accumulate(expr_list -> begin(), expr_list -> end(), &l, InsertExprInVector<T>());
    res = true;
  }

  return res;
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

bool
is_service_class(std::string attribute_value)
{
  boost::regex const service_class(service_class_tag + ".+");
  return boost::regex_match(attribute_value, service_class);
}

std::string
get_tag(matchmaking::match_info const& info)
{
  static std::string const null_string;

  classad::ClassAd const* ad = info.getAd();
  std::vector<std::string> acbr_vector;
  EvaluateAttrList(*ad, ACBR_tag, acbr_vector);

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
    print("", "no certificates in file");
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
          for (std::vector<voms>::iterator i = v.data.begin(); i != v.data.end(); i++) {
            for(std::vector<data>::iterator j = (*i).std.begin(); j != (*i).std.end(); j++) {
              std::string name = (*j).group;
              if ((*j).role != std::string("NULL")) {
                name += "/Role=" + (*j).role;
              }
              USER_attribs.push_back(Attribute("group", name, STRING));
            }
          }
        }
        else {
          print ("VOMS error", v.ErrorMessage());
          return false;
        }
      } else {
        print ("VOMS error", v.ErrorMessage());
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
  } else {
    return false;
  }
}

std::string
get_CE_unique_id(matchmaking::match_info const& info)
{
  classad::ClassAd const* ad = info.getAd();
  classad::Value value;

  ad->EvaluateExpr("GlueCEUniqueID", value);
  std::string result;
  value.IsStringValue(result);

  return result;
}

bool
filter_gpbox_authorizations(
  matchmaking::match_table_t& suitable_CEs,
  Connection& PEP_connection,
  std::string const& user_cert_file_name
)
{
  if (user_cert_file_name.empty()) {
    return false;
  }

  const std::string user_subject(
    get_proxy_distinguished_name(user_cert_file_name)
  );

  print("", user_subject);

  if(user_subject.empty()) {
    return false;
  }

  Attributes CE_attributes;
  std::string ce_names;
  std::string ce_tags;

  for (matchmaking::match_table_t::iterator it = suitable_CEs.begin();
       it != suitable_CEs.end();
       ++it) {
    ce_names += it->first + '#';
    std::string tag(get_tag(it->second));
    ce_tags += tag.empty() ? "error" : tag + '#';
  }
  ce_names.erase(ce_names.size() - 1);
  ce_tags.erase(ce_tags.size() - 1);

  print("CEs", ce_names);
  print("aggregation tag", ce_tags);

  CE_attributes.push_back(Attribute("aggregation-tag", ce_tags, STRING));

  try {

    PEPClient PEP_request(ce_names, "job-submission", user_subject);
    PEP_request.Attach(&PEP_connection);
    PEP_request.SetAttr(CE_attributes, RES);
    Attributes USER_attribs;
    PEP_request.SetAttr(USER_attribs, SUBJ);

    if(VOMS_proxy_init(user_cert_file_name, USER_attribs)) {
      PEP_request.SetAttr(USER_attribs, SUBJ);

      EvalResults evaluation_of_results;
      static std::string const null_string;

      if( PEP_request.Send(null_string, 0, 0, 0, evaluation_of_results) ) {
        print("", "filter_gbox_authorizations: PEP Send returned true");
        for (EvalResults::iterator iter = evaluation_of_results.begin();
          iter != evaluation_of_results.end();
          ++iter)
      {
          answer PEP_request_answer = iter->GetResult();
          // INDET may even be returned from an exception coming to the API
          // Send, so we don't even check it (since the policy (permit) is
          // according)), its content being untrustable

          std::string answer_id = iter->GetId();
          print("", answer_id);

          // NOTE: borderline cases are filtered off without questioning
          // because of the resubmission costs
          if( PEP_request_answer == DENY
              or
              PEP_request_answer == NOTA
              or 
              PEP_request_answer == INDET ) {

            suitable_CEs.erase(answer_id);
            print("", "!!!erased CE");
          }
          else {

            //at this point we've got the certainty that suitableCEs
            //will be a list of unique CE identifiers (if the related info 
            //is correctly published, if not the first added applies) so we can replace
            //the former unique key (CEID/VoViewID) with the real name for the CE
            matchmaking::match_table_t::iterator it = suitable_CEs.find(answer_id);
            if (it != suitable_CEs.end()) {
              std::string CE_id = get_CE_unique_id(it->second);
              matchmaking::match_info CE_ad(it->second);
              suitable_CEs.erase(answer_id);
              suitable_CEs[CE_id] = CE_ad;
              //or else (without defining CE_ad)
              //  suitable_CEs[CE_id] = suitable_CEs[answer_id];
            } else {
              print("", "Mismatching CE id got from gpbox answer\n");
            }
            //or else (one for the two branches)
            //suitable_CEs.erase(answer_id);
          }
        }
      }
      else {
        return false;
      }
    }
    else {
      print("", "VOMS_proxy_init returned false");
      return false;
    }
  } catch(...) {
    print("", "filter_gbox_authorizations: PEP Send returned false");
    return false;
  }

  return true;
}

bool
interact(
  std::string const& broker_subject,
  std::string const& x509_user_proxy,
  std::string const& PBOX_host_name,
  matchmaking::match_table_t& suitable_CEs)
{
  boost::timer perf_timer;

  const bool safe_mode = false;
  const int port_num = 6699;

  print("hostname", PBOX_host_name);
  print("port_num", port_num);
  print("safe mode", safe_mode);
  print("Connection done", "");

  if (!broker_subject.empty()) {
    try {

      Connection PEP_connection(
                                PBOX_host_name,
                                port_num,
                                broker_subject,
                                safe_mode
                               );

      if (!filter_gpbox_authorizations(suitable_CEs,
                                       PEP_connection,
                                       x509_user_proxy)) {
        return false;
      }
    }
    catch (...) { // exception no_conn from API
                  // PEP_connection not properly propagated
      print("", "gpbox: exception caught during interaction");
      return false;
    };
    std::string report(
      "gpbox interaction ended. Elapsed: "
      +
      boost::lexical_cast<std::string>(perf_timer.elapsed())
    );
    print("", report);
    return true;
  }
  else {
    print("", "gpbox: unable to find the broker proxy certificate");
    return false;
  }
}

} // empty namespace

int
main(int argc, char *argv[])
{
  if(argc == 4) // 1 = broker_proxy_file, 2 = user_proxy_file, 3 = pbox_host_name
  {

    const std::string broker_subject(
      get_proxy_distinguished_name(argv[1])
    );

    print("broker subj",broker_subject);

    matchmaking::match_table_t suitable_CEs;
    std::vector<std::string> VOViewsVector;

    VOViewsVector.push_back("VO:aaa");
    VOViewsVector.push_back("SC:GOLD");
    VOViewsVector.push_back("SC:SILVER");

    classad::ClassAd* ad1 = new classad::ClassAd;
    ad1->InsertAttr("GlueCEUniqueID", "ce01-lcg.cr.cnaf.infn.it:2119/aaa");
    InsertAttrList(*ad1, ACBR_tag, VOViewsVector);
    boost::shared_ptr<classad::ClassAd> _ad1(ad1);
    matchmaking::match_info __ad1(_ad1);
    pair<std::string, matchmaking::match_info> element1(
      std::string("ce01-lcg.cr.cnaf.infn.it:2119/blablabla"),
      __ad1
    );
    suitable_CEs.insert(element1);

    VOViewsVector.clear();
    VOViewsVector.push_back("VO:bbb");
    VOViewsVector.push_back("SC:SILVER");

    classad::ClassAd* ad2 = new classad::ClassAd;
    ad2->InsertAttr("GlueCEUniqueID", "gridit-ce-001.cnaf.infn.it:2119/bbb");
    InsertAttrList(*ad2, ACBR_tag, VOViewsVector);
    boost::shared_ptr<classad::ClassAd> _ad2(ad2);
    matchmaking::match_info __ad2(_ad2);
    pair<std::string, matchmaking::match_info> element2(
      std::string("gridit-ce-001.cnaf.infn.it:2119/blablabla"),
      __ad2
    );
    suitable_CEs.insert(element2);

    VOViewsVector.clear();
    VOViewsVector.push_back("VO:bbb");
    VOViewsVector.push_back("SC:GOLD");

    classad::ClassAd* ad3 = new classad::ClassAd;
    ad3->InsertAttr("GlueCEUniqueID", "pre-ce-01.cnaf.infn.it:2119/jobmanager-lcgpbs-infngrid_low02");
    InsertAttrList(*ad3, ACBR_tag, VOViewsVector);
    boost::shared_ptr<classad::ClassAd> _ad3(ad3);
    matchmaking::match_info __ad3(_ad3);
    pair<std::string, matchmaking::match_info> element3(
      std::string("pre-ce-01.cnaf.infn.it:2119/vkevnewlkvm/VO"),
      __ad3
    );
    suitable_CEs.insert(element3);

    VOViewsVector.clear();

    classad::ClassAd* ad4 = new classad::ClassAd;
    ad4->InsertAttr("GlueCEUniqueID", "pre-ce-01.cnaf.infn.it:2119/jobmanager-lcgpbs-infngrid_low03");
    InsertAttrList(*ad4, ACBR_tag, VOViewsVector);
    boost::shared_ptr<classad::ClassAd> _ad4(ad4);
    matchmaking::match_info __ad4(_ad4);
    pair<std::string, matchmaking::match_info> element4(
      std::string("pre-ce-01.cnaf.infn.it:2119/zzzzdfbvfdlk"),
      __ad4
    );
    suitable_CEs.insert(element4);

    VOViewsVector.clear();
    VOViewsVector.push_back("VO:bbb");
    VOViewsVector.push_back("SC:BRONZE");

    classad::ClassAd* ad5 = new classad::ClassAd;
    ad5->InsertAttr("GlueCEUniqueID", "pre-ce-01.cnaf.infn.it:2119/jobmanager-lcgpbs-infngrid_low03");
    InsertAttrList(*ad5, ACBR_tag, VOViewsVector);
    boost::shared_ptr<classad::ClassAd> _ad5(ad5);
    matchmaking::match_info __ad5(_ad5);
    pair<std::string, matchmaking::match_info> element5(
      std::string("pre-ce-01.cnaf.infn.it:2119/yyyyyyfbvfdlk"),
      __ad5
    );
    suitable_CEs.insert(element5);

    const std::string PBOX_host_name(argv[3]);
    if (!PBOX_host_name.empty())
    {
      if (!interact(
        broker_subject,
        argv[2],
        PBOX_host_name,
        suitable_CEs
      ))
        print("", "Error during gpbox interaction");
    }
  }
  else {
    std::cout << "Usage:\n";
    std::cout << "\tgpbox_test broker_proxy_file user_proxy_file pbox_host_name\n";
  }

  return 0;
}

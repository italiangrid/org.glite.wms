/*
 * File: retrieveCloseSEInfo.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <classad_distribution.h>
#include <fnCall.h>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include <regex.h>
#include <glite/wms/ism/ism.h>

using namespace std;
#ifdef WANT_NAMESPACES
using namespace classad;
#endif

namespace glite {
namespace wms {
namespace classad_plugin {
namespace gangmatch {

namespace ism = glite::wms::ism;

namespace {

char const* sa_attributes[] = {
"GlueSAPath", "GlueSchemaVersionMajor", "GlueSAStateAvailableSpace",
"GlueSARoot", "GlueSAStateUsedSpace", "GlueSAPolicyMaxFileSize",
"GlueSchemaVersionMinor", "GlueSAPolicyMinFileSize", "GlueSALocalID",
"GlueSAPolicyMaxPinDuration", "GlueSAType", "GlueSAPolicyMaxData",
"GlueSAPolicyMaxNumFiles","GlueSAPolicyQuota", "GlueSAPolicyFileLifeTime",
"GlueSAAccessControlBaseRule",
0
};

char const* se_attributes[] = {
"GlueSEName", "GlueSEUniqueID", "GlueSEPort"
"GlueSESizeTotal","GlueSEArchitecture", "GlueSESizeFree"
"GlueInformationServiceURL",
0
};

class gluesa_local_id_matches
{
  std::string m_id;
public:
  gluesa_local_id_matches(std::string const& id) 
    : m_id(id) {}
  bool operator()(classad::ExprTree* e) {
    std::string gluesa_local_id;
    static_cast<classad::ClassAd*>(e)->EvaluateAttrString(
      "GlueSALocalID", gluesa_local_id
    );
    return gluesa_local_id == m_id;
  }
};

bool evaluate(
  classad::ClassAd const& ad,
  std::string const& name,
  vector<classad::ExprTree*>& v
) {
  bool result = false;
  classad::Value value;
  const classad::ExprList *l;
  if (ad.EvaluateAttr(name, value) &&
    value.IsListValue(l)) {
      l->GetComponents(v);
      result = true;
  }
  return result;
}

}

bool retrieveCloseSEsInfo(
  const char         *name,
  const ArgumentList &arguments,
  EvalState          &state,
  Value              &result)
{
  bool  eval_successful = false;
  result.SetErrorValue();
  // We check to make sure that we passed exactly three arguments...
  if (arguments.size() == 1) {

    // ...the first and second arguments should evaluate to a string...
    Value       arg1;
    std::string vo;
    if (arguments[0] -> Evaluate(state, arg1) &&
      arg1.IsStringValue(vo)) {
	
      eval_successful = true;
      classad::ClassAd const* thiscead = 
        static_cast<const classad::ClassAd*>(arguments[0] -> GetParentScope());

      vector<classad::ExprTree*> ads;
      if (evaluate(*thiscead, "CloseStorageElements", ads)) {
          
        vector<classad::ExprTree*>::const_iterator it = ads.begin();
        vector<classad::ExprTree*>::const_iterator const e = ads.end();
        
        ism::ism_mutex_type::scoped_lock l(ism::get_ism_mutex(ism::se));
        ism::ism_type::const_iterator const ism_end(
          ism::get_ism(ism::se).end()
        );

        vector<classad::ExprTree*> sesa_ads;
        for( ; it != e; ++it ) {

          std::string name;
          static_cast<classad::ClassAd*>(*it)->EvaluateAttrString(
            "name",name
          );
          ism::ism_type::const_iterator se_it(
            ism::get_ism(ism::se).find(name)
          );
          if (se_it != ism_end) {
            boost::shared_ptr<classad::ClassAd> se_ad_ptr(
              boost::tuples::get<2>(se_it->second)
            );
            classad::ClassAd sesa_ad;
            for(int i=0; se_attributes[i]; ++i) {
              classad::ExprTree* e = se_ad_ptr->Lookup(se_attributes[i]);
              if(e) { 
                sesa_ad.Insert(
                  se_attributes[i],
                  e->Copy()
                );
              }
            }
            vector<classad::ExprTree*> ads_sa;
            if (evaluate(*se_ad_ptr, "GlueSA", ads_sa)) {

              vector<classad::ExprTree*>::const_iterator it_sa(
                std::find_if(
                  ads_sa.begin(), ads_sa.end(),
                  gluesa_local_id_matches(vo)
                )
              );
              if (it_sa != ads_sa.end()) {
                for(int i=0; sa_attributes[i]; ++i) {
                  classad::ExprTree* e = static_cast<classad::ClassAd*>(*it_sa)->Lookup(sa_attributes[i]);
                  if (e) {
                    sesa_ad.Insert(
                      sa_attributes[i],
                      e->Copy()
                    );
                  }
                }
              }
            }
            sesa_ads.push_back(sesa_ad.Copy()); 
          }

        }
        result.SetListValue(ExprList::MakeExprList(sesa_ads));
        eval_successful=true;
      }
      else {
        result.SetUndefinedValue();
      }
    }
  }
  return eval_successful;
}

} // namespace gangmatch
} // namespace clasad_plugin
} // namespace wms
} // namespace glite

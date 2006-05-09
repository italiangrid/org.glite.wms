// File: ldap-utils.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/mem_fn.hpp>
#include <boost/tokenizer.hpp>
#include <boost/progress.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>

#include "ldap-dn-utils.h"
#include "glite/wms/ism/purchaser/common.h"

#include "glite/wms/common/ldif2classad/LDAPQuery.h"
#include "glite/wms/common/ldif2classad/LDAPSynchConnection.h"
#include "glite/wms/common/ldif2classad/LDAPForwardIterator.h"
#include "glite/wms/common/ldif2classad/exceptions.h"

#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wmsutils/classads/classad_utils.h"

using namespace std;
namespace classadutils = glite::wmsutils::classads;

namespace glite {
namespace wms {

namespace ldif2classad	= common::ldif2classad;
namespace utilities     = common::utilities;

namespace ism {
namespace purchaser {

typedef boost::shared_ptr<classad::ClassAd> classad_shared_ptr;
typedef std::vector<classad_shared_ptr> vector_classad_shared_ptr;
typedef std::pair<std::string, classad_shared_ptr> pair_string_classad_shared_ptr;

typedef std::map<
  std::string, // ce id
  std::pair<classad_shared_ptr, vector_classad_shared_ptr> // cead, closes bind ad
> gluece_info_map_type;

typedef std::map<
  std::string,       // subcluster id
  classad_shared_ptr // subcluster ad
> gluesubcluster_info_map_type;

typedef std::map<
  std::string, // cluster id
  boost::tuple<
    string, // site name
    vector<gluece_info_map_type::iterator>,         // list of ce in cluster
    vector<gluesubcluster_info_map_type::iterator> // list of subcluster in cluster
  >
> gluecluster_info_map_type;

typedef std::map<
  std::string, // ce id
  std::vector<pair_string_classad_shared_ptr> // list of ce views per vo
> gluece_voview_info_map_type;

typedef std::map<
  std::string, // se id
  boost::tuple<
    classad_shared_ptr, // sead
    std::vector<classad::ExprTree*>, // list of sa info
    std::vector<classad::ExprTree*>, // list of control protocol info
    std::vector<classad::ExprTree*>  // list of access  protocol info
  >
> gluese_info_map_type;

bool is_access_control_vo_rule(const string& r)
{
 return boost::algorithm::istarts_with(r, "VO:");
}

string get_cluster_name(ldif2classad::LDIFObject& ldif_CE)
{
  std::string cluster;
  std::string reg_string("GlueClusterUniqueID\\s*=\\s*([^\\s]+)");
  ldif_CE.EvaluateAttribute("GlueCEInfoHostName", cluster);
  try {
    std::vector<std::string> foreignKeys;
    ldif_CE.EvaluateAttribute("GlueForeignKey", foreignKeys);
    static boost::regex get_cluid(reg_string);
    boost::smatch result_cluid;
    bool found = false;

    // Looking for one GlueForeignKey (it is possibly multi-valued)
    // With the specified attribute.
    for( std::vector<std::string>::const_iterator key = foreignKeys.begin();
	 key != foreignKeys.end(); key++) {

      if( boost::regex_match(*key, result_cluid, get_cluid) ) {

	cluster.assign(result_cluid[1].first,result_cluid[1].second);
	found = true;
	break;
      }
    }
    if (!found) {

      Warning("Cannot find GlueClusterUniqueID assignment. Using "
	      << cluster << ".");
    }
  } catch( ldif2classad::LDAPNoEntryEx& ) {

    // The GlueForeignKey was not found.
    // We keep the gatekeeper name.
  } catch( boost::bad_expression& e ){

    Error("Bad regular expression " << reg_string
	  << ". Cannot parse GlueForeignKey. Using " << cluster << ".");
  }
  return cluster;
}

string get_site_name(ldif2classad::LDIFObject& ldif_CE)
{
  std::string site;
  std::string reg_string("GlueSiteUniqueID\\s*=\\s*([^\\s]+)");
  try {
    std::vector<std::string> foreignKeys;
    ldif_CE.EvaluateAttribute("GlueForeignKey", foreignKeys);
    static boost::regex get_gsuid(reg_string);
    boost::smatch result_gsuid;
    bool found = false;
    std::vector<std::string>::const_iterator key_it = foreignKeys.begin();
    std::vector<std::string>::const_iterator const key_e = foreignKeys.end();

    // Looking for one GlueForeignKey (it is possibly multi-valued)
    // With the specified attribute.
    for( ;
         key_it != key_e; ++key_it) {

      if( boost::regex_match(*key_it, result_gsuid, get_gsuid) ) {

        site.assign(result_gsuid[1].first,result_gsuid[1].second);
        found = true;
        break;
      }
    }
    if (!found) {

      Warning("Cannot find GlueSiteUniqueID assignment.");
    }
  } catch( ldif2classad::LDAPNoEntryEx& ) {

    // The GlueForeignKey was not found.
    // We keep the gatekeeper name.
  } catch( boost::bad_expression& e ){

    Error("Bad regular expression " << reg_string
          << ". Cannot parse GlueForeignKey.");
  }
  return site;
}

void 
fetch_bdii_se_info(boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection, 
  gluese_info_container_type& gluese_info_container) 
{
  string filter("(|(objectclass=gluecesebind)(|(objectclass=gluese)(|(objectclass=gluesa)"
    "(|(objectclass=glueseaccessprotocol)(objectclass=gluesecontrolprotocol)))))");
  
  ldif2classad::LDAPQuery query(IIconnection.get(), filter, vector<string>());

  try {

    IIconnection->open();
    query.execute();
    if (!query.tuples()->empty()) {

      utilities::ii_attributes::const_iterator multi_attrs_begin;
      utilities::ii_attributes::const_iterator multi_attrs_end;
      boost::tie(multi_attrs_begin,multi_attrs_end)
	= utilities::ii_attributes::multiValued();

      ldif2classad::LDAPForwardIterator ldap_it( query.tuples() );
      ldap_it.first();
      
      gluese_info_map_type         gluese_info_map;

      while (ldap_it.current()) {

	vector<string> ldap_dn_tokens;
	tokenize_ldap_dn(ldap_it.ldap_dn(), ldap_dn_tokens);
        if (is_gluese_info_dn(ldap_dn_tokens)) {
          ldif2classad::LDIFObject ldif_SE(*ldap_it);
          boost::shared_ptr<classad::ClassAd> seAd(
            ldif_SE.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
          string const gluese_unique_id(
            ldap_dn_tokens[0].substr(ldap_dn_tokens[0].rfind("=")+1)
          );
          gluese_info_map_type::iterator it;
          bool gluese_info_map_insert;
          boost::tie(it, gluese_info_map_insert) = gluese_info_map.insert(
            std::make_pair(
              gluese_unique_id, 
              boost::make_tuple(
                seAd,
                std::vector<classad::ExprTree*>(),
                std::vector<classad::ExprTree*>(),
                std::vector<classad::ExprTree*>()
              )
            )
          );
          if (!gluese_info_map_insert) { 
            // The entry has been previously added (*)
            // only the ad should be updated now                              
            boost::tuples::get<0>(it->second) = seAd;                          
          }                                                                   
        }                                                                     
        else if (
          is_gluesa_info_dn(ldap_dn_tokens) ||
          is_gluese_control_protocol_info_dn(ldap_dn_tokens) ||
          is_gluese_access_protocol_info_dn(ldap_dn_tokens)) {

          ldif2classad::LDIFObject ldif(*ldap_it);
          classad::ExprTree* ad(
            ldif.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
          string const gluese_unique_id(
            ldap_dn_tokens[1].substr(ldap_dn_tokens[1].rfind("=")+1)
          );
          gluese_info_map_type::iterator it(
            gluese_info_map.find(gluese_unique_id)
          );
          if (it == gluese_info_map.end()) {
            bool gluese_info_map_insert; 
            boost::tie(it, gluese_info_map_insert) = gluese_info_map.insert(           // (*)
              std::make_pair(
                gluese_unique_id,
                boost::make_tuple(
                  classad_shared_ptr(),
                  std::vector<classad::ExprTree*>(),
                  std::vector<classad::ExprTree*>(),
                  std::vector<classad::ExprTree*>()
                )
              )
            );
            if(gluese_info_map_insert) {
              Debug("info for " << gluese_unique_id << " inserted");
            }
          }
          if(is_gluesa_info_dn(ldap_dn_tokens)) {
            boost::tuples::get<1>(it->second).push_back(ad);
          }
          else if(is_gluese_control_protocol_info_dn(ldap_dn_tokens)) {
            boost::tuples::get<2>(it->second).push_back(ad);
          }
          else if(is_gluese_access_protocol_info_dn(ldap_dn_tokens)) {
            boost::tuples::get<3>(it->second).push_back(ad);
          }
        }
	ldap_it.next();
      } // while( ldap_it.current() )

      gluese_info_map_type::const_iterator se_it(
        gluese_info_map.begin()
      );
      gluese_info_map_type::const_iterator const se_e(
        gluese_info_map.end()
      );

      for( ; se_it != se_e; ++se_it) {

        if (!boost::tuples::get<0>(se_it->second)) continue;
       
        if (!boost::tuples::get<1>(se_it->second).empty()) {
          boost::tuples::get<0>(se_it->second)->
            Insert("GlueSA", classad::ExprList::MakeExprList(
              boost::tuples::get<1>(se_it->second)
            ));
        }
        if (!boost::tuples::get<2>(se_it->second).empty()) {
          boost::tuples::get<0>(se_it->second)->
            Insert("GlueSEControlProtocol", classad::ExprList::MakeExprList(
              boost::tuples::get<2>(se_it->second)
            ));
        }
        if (!boost::tuples::get<3>(se_it->second).empty()) {
          boost::tuples::get<0>(se_it->second)->
            Insert("GlueSEAccessProtocol", classad::ExprList::MakeExprList(
              boost::tuples::get<3>(se_it->second)
            ));
        }
        gluese_info_container[se_it->first] =
          boost::tuples::get<0>(se_it->second);
      }
    }
  }
  catch (ldif2classad::LDAPNoEntryEx&) {

    Error("Unexpected result while searching the IS...");
  }
  catch( ldif2classad::QueryException& e) {

    Warning(e.what());
  }
  catch( ldif2classad::ConnectionException& e) {

    Warning(e.what());
  }
}
     
void 
fetch_bdii_ce_info(boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection, 
  gluece_info_container_type& gluece_info_container) 
{
  string filter("(|(objectclass=gluevoview)(|(objectclass=gluecesebind)(|(objectclass=gluece)"
    "(|(objectclass=gluecluster)(objectclass=gluesubcluster)))))");
  ldif2classad::LDAPQuery query(IIconnection.get(), filter, vector<string>());

  try {

    IIconnection->open();
    query.execute();
    if (!query.tuples()->empty()) {

      utilities::ii_attributes::const_iterator multi_attrs_begin;
      utilities::ii_attributes::const_iterator multi_attrs_end;
      boost::tie(multi_attrs_begin,multi_attrs_end)
	= utilities::ii_attributes::multiValued();

      ldif2classad::LDAPForwardIterator ldap_it( query.tuples() );
      ldap_it.first();
      
      gluece_info_map_type         gluece_info_map;
      gluesubcluster_info_map_type gluesubcluster_info_map;
      gluecluster_info_map_type    gluecluster_info_map;
      gluece_voview_info_map_type  gluece_voview_info_map;

      while (ldap_it.current()) {

	vector<string> ldap_dn_tokens;
	tokenize_ldap_dn(ldap_it.ldap_dn(), ldap_dn_tokens);
	
	if (is_gluecluster_info_dn(ldap_dn_tokens)) {
	  string glue_cluster_unique_id(
	    ldap_dn_tokens[0].substr(ldap_dn_tokens[0].rfind("=")+1)
          );
          ldif2classad::LDIFObject ldif_CL(*ldap_it);
          string glue_site_unique_id(get_site_name(ldif_CL));
          
          gluecluster_info_map_type::iterator it;
          bool gluecluster_info_map_insert;
          boost::tie(it, gluecluster_info_map_insert) =
            gluecluster_info_map.insert(
              std::make_pair(glue_cluster_unique_id,
                boost::make_tuple(
                  glue_site_unique_id,
                  vector<gluece_info_map_type::iterator>(),
                  vector<gluesubcluster_info_map_type::iterator>()
                )
              )
            );
          if (!gluecluster_info_map_insert) {
            boost::tuples::get<0>(it->second) = glue_site_unique_id;
          }
	}
	else 
        if (is_gluece_info_dn(ldap_dn_tokens)) {
	  ldif2classad::LDIFObject ldif_CE(*ldap_it);
          boost::shared_ptr<classad::ClassAd> ceAd(
            ldif_CE.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
	  string gluece_unique_id(
	    ldap_dn_tokens[0].substr(ldap_dn_tokens[0].rfind("=")+1)
	  );
	  string glue_cluster_unique_id(get_cluster_name(ldif_CE));
	  
          gluece_info_map_type::iterator it;
	  bool gluece_info_map_insert;
	  boost::tie(it, gluece_info_map_insert) =
	    gluece_info_map.insert(
              std::make_pair(gluece_unique_id, 
                std::make_pair(ceAd, vector_classad_shared_ptr())
              )
            );
	  if(gluece_info_map_insert) 
            boost::tuples::get<1>(
              gluecluster_info_map[glue_cluster_unique_id]
            ).push_back(it);
	}
	else if (is_gluesubcluster_info_dn(ldap_dn_tokens)) {
          ldif2classad::LDIFObject ldif_SC(*ldap_it);
	  boost::shared_ptr<classad::ClassAd> scAd(
            ldif_SC.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
          string gluesubcluster_unique_id(
            ldap_dn_tokens[0].substr(ldap_dn_tokens[0].rfind("=")+1)
          );
          string glue_cluster_unique_id(
            ldap_dn_tokens[1].substr(ldap_dn_tokens[1].rfind("=")+1)
          );
          gluesubcluster_info_map_type::iterator it;
          bool gluesubcluster_info_map_insert;
          boost::tie(it, gluesubcluster_info_map_insert) =
            gluesubcluster_info_map.insert(
              std::make_pair(gluesubcluster_unique_id, scAd)
            );
          if (gluesubcluster_info_map_insert) 
            boost::tuples::get<2>(
              gluecluster_info_map[glue_cluster_unique_id]
            ).push_back(it);
        }
	else if (is_gluecesebind_info_dn(ldap_dn_tokens)) {
	  ldif2classad::LDIFObject ldif_BN(*ldap_it);
          boost::shared_ptr<classad::ClassAd> bnAd(
            ldif_BN.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
          string glueceuniqueid(
	    ldap_dn_tokens[1].substr(ldap_dn_tokens[1].rfind("=")+1)
          );
          gluece_info_map[glueceuniqueid].second.push_back(bnAd);
	}
        else if (is_gluevoview_info_dn(ldap_dn_tokens)) {
          ldif2classad::LDIFObject ldif_VO(*ldap_it);
          boost::shared_ptr<classad::ClassAd> voAd(
            ldif_VO.asClassAd(multi_attrs_begin, multi_attrs_end)
          );
          string gluevoviewlocalid(
            ldap_dn_tokens[0].substr(ldap_dn_tokens[0].rfind("=")+1)
          );
          string glueceuniqueid(
            ldap_dn_tokens[1].substr(ldap_dn_tokens[1].rfind("=")+1)
          );
          gluece_voview_info_map[glueceuniqueid].push_back(
            std::make_pair(gluevoviewlocalid,voAd)
          );
        }
	ldap_it.next();
      } // while( ldap_it.current() )
      
      gluecluster_info_map_type::const_iterator cl_it(
        gluecluster_info_map.begin()
      );

      gluecluster_info_map_type::const_iterator const cl_e(
        gluecluster_info_map.end()
      );

      for( ; cl_it != cl_e; ++cl_it) { // for each cluster
        
        if (boost::tuples::get<2>(cl_it->second).empty()) {
          continue; // Addresses bug #15098
        }

        vector<gluesubcluster_info_map_type::iterator>::const_iterator sc_it(
          boost::tuples::get<2>(cl_it->second).begin()
        );

	boost::shared_ptr<classad::ClassAd> sc_ad = (*sc_it)->second;

        string cluster_id(cl_it->first);
        vector<gluece_info_map_type::iterator>::const_iterator ce_it(
          boost::tuples::get<1>(cl_it->second).begin()
        );
        vector<gluece_info_map_type::iterator>::const_iterator const ce_e(
          boost::tuples::get<1>(cl_it->second).end()
        );
        
        for( ; ce_it != ce_e; ++ce_it) {

          (*ce_it)->second.first->Update(*sc_ad.get());
          (*ce_it)->second.first->InsertAttr(
            "GlueSiteUniqueID",
            boost::tuples::get<0>(cl_it->second)
          );
          std::vector<std::string> sebind;
          std::vector<classad::ExprTree*>  exprs;
          vector<classad_shared_ptr>::const_iterator se_it(
            (*ce_it)->second.second.begin()
          );
          vector<classad_shared_ptr>::const_iterator const se_e(
            (*ce_it)->second.second.end()
          );
          
          for( ; se_it != se_e; ++se_it) {
            exprs.push_back(new classad::ClassAd);
	    static_cast<classad::ClassAd*>(
              exprs.back())->Update(*se_it->get()
            );
 
            std::string gluecesebindseuniqueid;
            static_cast<classad::ClassAd*>(exprs.back())->
              EvaluateAttrString(
                "GlueCESEBindSEUniqueID", gluecesebindseuniqueid
            );
            sebind.push_back(gluecesebindseuniqueid);

            static_cast<classad::ClassAd*>(exprs.back())->Insert(
              "name",
              classad::AttributeReference::MakeAttributeReference(
                0, "GlueCESEBindSEUniqueID"
              )
            );
            static_cast<classad::ClassAd*>(exprs.back())->Insert(
              "mount", 
              classad::AttributeReference::MakeAttributeReference(
                0,"GlueCESEBindCEAccesspoint"
              )
            );
          }
          (*ce_it)->second.first->Insert(
            "CloseStorageElements", 
            classad::ExprList::MakeExprList(exprs)
          );
          (*ce_it)->second.first->Insert(
            "GlueCESEBindGroupSEUniqueID",
            classadutils::asExprList(sebind)
          );
          try {  
            expand_glueceid_info((*ce_it)->second.first);
            insert_aux_requirements((*ce_it)->second.first);
            insert_gangmatch_storage_ad((*ce_it)->second.first);
          }
          catch(classadutils::InvalidValue) {
            Error("Cannot extract GlueCEUniqueID from Ad");
            continue;
          }

          gluece_voview_info_map_type::const_iterator const vo_views(
            gluece_voview_info_map.find((*ce_it)->first)
          );

          if (vo_views!=gluece_voview_info_map.end() &&
              !vo_views->second.empty()
             ) {
            
            vector<string> access_control_base_rules;

            classadutils::EvaluateAttrList(
              *((*ce_it)->second.first),
              "GlueCEAccessControlBaseRule",
              access_control_base_rules
            );
            bool access_control_vo_rule_exists = false;
            vector<string>::iterator last_access_control_vo_rule(
              access_control_base_rules.begin()
            );
            // for each access control vo rule...
            do {

              last_access_control_vo_rule = std::find_if(
                last_access_control_vo_rule,
                access_control_base_rules.end(),
                is_access_control_vo_rule
              );
              access_control_vo_rule_exists = 
                last_access_control_vo_rule != access_control_base_rules.end();
              
	      if (access_control_vo_rule_exists) {
                 size_t colon_pos = last_access_control_vo_rule->rfind(":");
                 string vo(
                   last_access_control_vo_rule->substr(colon_pos+1)
                 );

                 vector<pair_string_classad_shared_ptr>::const_iterator 
                   vo_view_it(
                     vo_views->second.begin()
                   );
                 vector<pair_string_classad_shared_ptr>::const_iterator const 
                   vo_view_e(
                     vo_views->second.end()
                   );
                 for ( ; vo_view_it!=vo_view_e; ++vo_view_it) 
                   if(vo_view_it->first==vo) break;
                 
                 
                 //...if exists a voview create the relevant ceAd
                 if(vo_view_it!=vo_view_e) {
                   classad_shared_ptr ceAd(
                     dynamic_cast<classad::ClassAd*>(
                       (*ce_it)->second.first->Copy()
                     )
                   );
                   ceAd->Update(*vo_view_it->second);
                   gluece_info_container.insert(
                     std::make_pair(
                       (*ce_it)->first + "/" + vo_view_it->first,
                       ceAd
                     )
                   );
                   
                   last_access_control_vo_rule =
                     access_control_base_rules.erase(last_access_control_vo_rule);
                 }
                 else {
                   ++last_access_control_vo_rule;
                 }
              }
            } while(access_control_vo_rule_exists);
            
            if (!access_control_base_rules.empty()) {
              (*ce_it)->second.first->Insert(
                "GlueCEAccessControlBaseRule",
                classadutils::asExprList(access_control_base_rules)
              );
              gluece_info_container.insert(
                std::make_pair((*ce_it)->first, (*ce_it)->second.first)
              );  
            }
          }
          else {
            gluece_info_container.insert( 
              std::make_pair((*ce_it)->first, (*ce_it)->second.first)
            );
          }
	}
      }
    }
  }
  catch (ldif2classad::LDAPNoEntryEx&) {

    Error("Unexpected result while searching the IS...");
  }
  catch( ldif2classad::QueryException& e) {

    Warning(e.what());
  }
  catch( ldif2classad::ConnectionException& e) {

    Warning(e.what());
  }
}

void fetch_bdii_info(const std::string& hostname,
			int port,
			const std::string& dn,
			int timeout,
			gluece_info_container_type& gluece_info_container,
                        gluese_info_container_type& gluese_info_container)
{
  boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection(
    new ldif2classad::LDAPSynchConnection(dn, hostname, port, timeout)
  );
  fetch_bdii_ce_info(IIconnection, gluece_info_container);
  fetch_bdii_se_info(IIconnection, gluese_info_container);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

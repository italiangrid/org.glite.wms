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

#include "glite/wms/ism/purchaser/common.h"

#include "glite/wms/common/ldif2classad/LDAPQuery.h"
#include "glite/wms/common/ldif2classad/LDAPSynchConnection.h"
#include "glite/wms/common/ldif2classad/LDAPForwardIterator.h"
#include "glite/wms/common/ldif2classad/exceptions.h"

#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/logger/logger_utils.h"


using namespace std;

namespace glite {
namespace wms {

namespace ldif2classad	= common::ldif2classad;

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
  pair<
    vector<gluece_info_map_type::iterator>,         // list of ce in cluster
    vector<gluesubcluster_info_map_type::iterator> // list of subcluster in cluster
  >
> gluecluster_info_map_type;

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

void tokenize_ldap_dn(std::string const& s, std::vector<std::string> &v)
{
  boost::escaped_list_separator<char> ldap_dn_sep("",",","");
  boost::tokenizer<boost::escaped_list_separator<char> > 
    ldap_dn_tok(s,ldap_dn_sep);

  boost::tokenizer< boost::escaped_list_separator<char> >::iterator
    ldap_dn_tok_it(
      ldap_dn_tok.begin()
    );
  boost::tokenizer< boost::escaped_list_separator<char> >::iterator const 
    ldap_dn_tok_end(
       ldap_dn_tok.end()
    );

  for( ; ldap_dn_tok_it != ldap_dn_tok_end; ++ldap_dn_tok_it) 
    v.push_back(
      boost::algorithm::trim_copy(*ldap_dn_tok_it)
    );
  
}

bool is_gluecluster_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 1 && 
        dn[0].substr(0,string("GlueClusterUniqueID").length()) == 
          string("GlueClusterUniqueID") &&
        dn[1].substr(0,string("mds-vo-name").length()) == 
          string("mds-vo-name");
}

bool is_gluece_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 1 && 
        dn[0].substr(0,string("GlueCEUniqueID").length()) == 
          string("GlueCEUniqueID") &&
        dn[1].substr(0,string("mds-vo-name").length()) == 
          string("mds-vo-name");
}

bool is_gluesubcluster_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
        dn[0].substr(0,string("GlueSubClusterUniqueID").length()) == 
          string("GlueSubClusterUniqueID") &&
        dn[1].substr(0,string("GlueClusterUniqueID").length()) == 
          string("GlueClusterUniqueID") &&
        dn[2].substr(0,string("mds-vo-name").length()) == 
          string("mds-vo-name");
}

bool is_gluecesebind_info_dn(std::vector<std::string> const& dn)
{
 return dn.size() > 2 && 
   dn[0].substr(0,string("GlueCESEBindSEUniqueID").length()) == 
     string("GlueCESEBindSEUniqueID") &&
   dn[1].substr(0,string("GlueCESEBindGroupCEUniqueID").length()) == 
     string("GlueCESEBindGroupCEUniqueID") &&
   dn[2].substr(0,string("mds-vo-name").length()) == 
     string("mds-vo-name");
}

void 
fetch_bdii_info(boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection, 
  gluece_info_container_type& gluece_info_container) 
{
  string filter("(|(objectclass=gluecesebind)(|(objectclass=gluece)"
    "(|(objectclass=gluecluster)(objectclass=gluesubcluster))))");
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

      while (ldap_it.current()) {

	vector<string> ldap_dn_tokens;
	tokenize_ldap_dn(ldap_it.ldap_dn(), ldap_dn_tokens);
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
            gluecluster_info_map[glue_cluster_unique_id].first.push_back(it);
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
            gluecluster_info_map[glue_cluster_unique_id].second.push_back(it);
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
	ldap_it.next();
      } // while( ldap_it.current() )
      
      gluecluster_info_map_type::const_iterator cl_it(
        gluecluster_info_map.begin()
      );

      gluecluster_info_map_type::const_iterator const cl_e(
        gluecluster_info_map.end()
      );
/*
      gluesubcluster_info_map_type::const_iterator sc_it(
        gluesubcluster_info_map.begin()
      );

      gluesubcluster_info_map_type::const_iterator const sc_e(
        gluesubcluster_info_map.end()
      );
*/
      for( ; cl_it != cl_e; ++cl_it) { // for each cluster
        
        vector<gluesubcluster_info_map_type::iterator>::const_iterator sc_it(
          cl_it->second.second.begin()
        );
/*
        vector<gluesubcluster_info_map_type::iterator>::const_iterator sc_e(
          cl_it->second.second.end()
        );
*/

        if (cl_it->second.second.empty()) {
          continue;
        }
	boost::shared_ptr<classad::ClassAd> sc_ad = (*sc_it)->second;

        string cluster_id(cl_it->first);
        vector<gluece_info_map_type::iterator>::const_iterator ce_it(
          cl_it->second.first.begin()
        );
        vector<gluece_info_map_type::iterator>::const_iterator const ce_e(
          cl_it->second.first.end()
        );
        
        for( ; ce_it != ce_e; ++ce_it) {

          (*ce_it)->second.first->Update(*sc_ad.get());
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

          gluece_info_container.insert( 
            std::make_pair((*ce_it)->first, (*ce_it)->second.first)
          );
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
			gluece_info_container_type& gluece_info_container)
{
  boost::shared_ptr<ldif2classad::LDAPConnection> IIconnection(
    new ldif2classad::LDAPSynchConnection(dn, hostname, port, timeout)
  );
  fetch_bdii_info(IIconnection, gluece_info_container);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

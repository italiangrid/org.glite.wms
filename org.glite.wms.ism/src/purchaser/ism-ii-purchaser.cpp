// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "ism-ii-purchaser.h"
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "glite/wms/common/ldif2classad/LDAPQuery.h"
#include "glite/wms/common/ldif2classad/LDAPSynchConnection.h"
#include "glite/wms/common/ldif2classad/LDAPForwardIterator.h"
#include "glite/wms/common/ldif2classad/exceptions.h"

#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/ii_attr_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/ism/ism.h"

using namespace std;

namespace exception = glite::wmsutils::exception;

namespace glite {
namespace wms {

namespace ldif2classad  = common::ldif2classad;
namespace utilities     = common::utilities;
namespace logger        = common::logger;

namespace ism {
namespace purchaser {

namespace {

typedef boost::shared_ptr<classad::ClassAd>      gluece_info_type;
typedef std::map<std::string, gluece_info_type>  gluece_info_container_type;
typedef gluece_info_container_type::const_iterator gluece_info_const_iterator;
typedef gluece_info_container_type::iterator       gluece_info_iterator;


bool mergeInfo(
  string const& filter,
  ldif2classad::LDAPConnection& IIconnection,
  ldif2classad::LDIFObject& o
)
{
  Debug("Filtering and merging: " << filter);
  vector<string> all_attributes;
  ldif2classad::LDAPQuery query(&IIconnection, filter, all_attributes);
  query.execute();
  if (!query.tuples()->empty()) {

    ldif2classad::LDAPForwardIterator ldap_it(query.tuples());
    ldap_it.first();
    if (ldap_it.current()) {

      o.merge(*ldap_it);
      return true;
    } else {
      Error("Bad filter...!");
    }
  }
  return false;
}

string getClusterName(ldif2classad::LDIFObject& ldif_CE)
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

/*
 * Prefetch GlueCEUniqueIDs
 * This method obtains the GlueCEUniqueIDs from the information index
 */
void prefetchGlueCEinfo(const std::string& hostname,
                        int port,
                        const std::string& dn,
                        int timeout,
                        gluece_info_container_type& gluece_info_container)
{
  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection(
    new ldif2classad::LDAPSynchConnection(dn, hostname, port, timeout)
  );

  vector<string> reqAttributes;

  reqAttributes.push_back("GlueCEUniqueID");
  reqAttributes.push_back("GlueSubClusterUniqueID");
  reqAttributes.push_back("GlueCEInfoHostName");
  reqAttributes.push_back("GlueForeignKey");
  reqAttributes.push_back("GlueClusterUniqueID");
  reqAttributes.push_back("GlueInformationServiceURL");

  string filter("objectclass=GlueCE");

  ldif2classad::LDAPQuery query(IIconnection.get(), filter, reqAttributes);

  Debug("Filtering Information Index (GlueCEUniqueIDs): " << filter);

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

      bool all_merge_query_fail = true;

      while (ldap_it.current()) {

        ldif2classad::LDIFObject ldif_CE(*ldap_it);
        string GlueCEUniqueID;
        ldif_CE.EvaluateAttribute("GlueCEUniqueID", GlueCEUniqueID);

        string cluster("(&(objectclass=GlueSubCluster)(GlueSubClusterUniqueID=" + getClusterName(ldif_CE) + "))");
        string closese("(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupCEUniqueID=" + GlueCEUniqueID + "))");

        if (mergeInfo(cluster, *IIconnection, ldif_CE)
            && mergeInfo(closese, *IIconnection, ldif_CE)) {

          all_merge_query_fail = false;
        }

        boost::shared_ptr<classad::ClassAd> ceAd(
          ldif_CE.asClassAd(multi_attrs_begin, multi_attrs_end)
        );
        gluece_info_container[GlueCEUniqueID] = ceAd;
        ldap_it.next();
      } // while( ldap_it.current() )
    }
  }
  catch (ldif2classad::LDAPNoEntryEx&) {

    Error("Unexpected result while searching the IS...");
    //throw matchmaking::ISQueryError(
    //                          NSconf->ii_contact(),
    //                          NSconf->ii_port(),
    //                          NSconf->ii_dn(),
    //                          filter);
  }
  catch( ldif2classad::QueryException& e) {

    Warning(e.what());
    //throw matchmaking::ISQueryError(
    //                          NSconf->ii_contact(),
    //                          NSconf->ii_port(),
    //                          NSconf->ii_dn(),
    //                          filter);
  }
  catch( ldif2classad::ConnectionException& e) {

    Warning(e.what());
    //throw matchmaking::ISConnectionError(
    //                               NSconf->ii_contact(),
    //                               NSconf->ii_port(),
    //                               NSconf->ii_dn());
  }
}

bool expand_information_service_info(gluece_info_type& gluece_info)
{
  string isURL;
  bool result = false;
  try {

    isURL = utilities::evaluate_attribute(*gluece_info, "GlueInformationServiceURL");
    static boost::regex  expression_gisu("\\S.*://(.*):([0-9]+)/(.*)");
    boost::smatch        pieces_gisu;

    if (boost::regex_match(isURL, pieces_gisu, expression_gisu)) {

      string ishost(pieces_gisu[1].first, pieces_gisu[1].second);
      string isport(pieces_gisu[2].first, pieces_gisu[2].second);
      string isbasedn(pieces_gisu[3].first, pieces_gisu[3].second);

      gluece_info->InsertAttr("InformationServiceDN", isbasedn);
      gluece_info->InsertAttr("InformationServiceHost", ishost);
      gluece_info->InsertAttr("InformationServicePort",
                              boost::lexical_cast<int>(isport));
      result = true;
    }
  } catch (utilities::InvalidValue& e) {
    Error("Cannot evaluate GlueInformationServiceURL...");
    result = false;
  }
  return result;
}

bool fetch_gluece_info(gluece_info_type& gluece_info)
{
  std::string is_dn;
  try {
    is_dn = utilities::evaluate_attribute(*gluece_info, "InformationServiceDN");
  } catch (utilities::InvalidValue& e) {
    Warning("Cannot evaluate InformationServiceDN...");
    return false;
  }

  std::string is_host;
  try {
    is_host = utilities::evaluate_attribute(*gluece_info, "InformationServiceHost");
  } catch (utilities::InvalidValue& e) {
    Warning("Cannot evaluate InformationServiceHost...");
    return false;
  }

  int is_port;
  try {
    is_port = utilities::evaluate_attribute(*gluece_info, "InformationServicePort");
  } catch (utilities::InvalidValue& e) {
    Warning("Cannot evaluate InformationServicePort...");
    return false;
  }

  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection(
    new ldif2classad::LDAPSynchConnection(is_dn, is_host, is_port, 20)
  );

  std::vector<std::string> all_attributes;
  std::string filter("(&(objectclass=GlueCE)(GlueCEUniqueID=");
  std::string gluece_id(utilities::evaluate_attribute(*gluece_info, "GlueCEUniqueID"));

  filter += gluece_id + "))";

  ldif2classad::LDAPQuery query(IIconnection.get(), filter, all_attributes);

  try {

    IIconnection->open();
    query.execute();
    if (!query.tuples()->empty())  {

      utilities::ii_attributes::const_iterator multi_attrs_begin;
      utilities::ii_attributes::const_iterator multi_attrs_end;
      boost::tie(multi_attrs_begin, multi_attrs_end)
        = utilities::ii_attributes::multiValued();

      ldif2classad::LDAPForwardIterator ldap_it(query.tuples());
      ldap_it.first();

      boost::scoped_ptr<classad::ClassAd> ceAd(
        ldap_it->asClassAd(multi_attrs_begin, multi_attrs_end)
      );
      gluece_info->Update(*ceAd);
    } // if GRIS query not empty

  } catch (ldif2classad::ConnectionException& e) {
    Warning(e.what());
    return false;
  } catch (ldif2classad::QueryException& e) {
    Warning(e.what());
    return false;
  } catch (ldif2classad::LDAPNoEntryEx&) {
    Error("Unexpected result while searching the IS..." << gluece_id);
    return false;
  }
  return true;
}

boost::scoped_ptr<classad::ClassAd> requirements_ad;

void insert_aux_requirements(gluece_info_type& gluece_info)
{
  if (!requirements_ad) {
    
    std::string requirements_str(
      "[ \
        CloseOutputSECheck = IsUndefined(other.OutputSE) \
          ||   member(other.OutputSE, GlueCESEBindGroupSEUniqueID); \
        AuthorizationCheck = member(other.CertificateSubject, GlueCEAccessControlBaseRule) \
          ||   member(strcat(\"VO:\",other.VirtualOrganisation), GlueCEAccessControlBaseRule); \
      ]"
    );

    try {
      requirements_ad.reset(utilities::parse_classad(requirements_str));
    }
    catch(...) {
      std::cout << "Ops!" << std::endl;
      exit( -1 );
    }
  }
  gluece_info->Update(*requirements_ad);
}

bool expand_glueceid_info(gluece_info_type& gluece_info)
{
  string ce_str;
  ce_str.assign(utilities::evaluate_attribute(*gluece_info, "GlueCEUniqueID"));
  static boost::regex  expression_ceid("(.+/[^\\-]+-(.+))-(.+)");
  boost::smatch  pieces_ceid;
  string gcrs, type, name;
  
  if (boost::regex_match(ce_str, pieces_ceid, expression_ceid)) {
    
    gcrs.assign(pieces_ceid[1].first, pieces_ceid[1].second);
    try {
      type.assign(utilities::evaluate_attribute(*gluece_info, "GlueCEInfoLRMSType"));
    } 
    catch(utilities::InvalidValue& e) {
      // Try to fall softly in case the attribute is missing...
      type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
      Warning("Cannot evaluate GlueCEInfoLRMSType using value from contact string: " << type);
    }
    // ... or in case the attribute is empty.
    if (type.length() == 0) type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
    name.assign(pieces_ceid[3].first, pieces_ceid[3].second);
  }
  else { 
    Warning("Cannot parse CEid=" << ce_str);
    return false;
  }
  gluece_info -> InsertAttr("GlobusResourceContactString", gcrs);
  gluece_info -> InsertAttr("LRMSType", type);
  gluece_info -> InsertAttr("QueueName", name);
  gluece_info -> InsertAttr("CEid", ce_str);
  return true;
}

} // {anonymous}

ism_ii_purchaser::ism_ii_purchaser(
  std::string const& hostname,
  int port,
  std::string const& distinguished_name,
  int timeout,
  exec_mode_t mode,
  size_t interval
)
  : m_hostname(hostname),
    m_port(port),
    m_dn(distinguished_name),
    m_timeout(timeout),
    m_mode(mode),
    m_interval(interval)
{
}

namespace {

timestamp_type get_current_time(void)
{
  timestamp_type current_time;

  boost::xtime_get(&current_time, boost::TIME_UTC);
  
  return current_time;
}

}

void ism_ii_purchaser::operator()()
{
  gluece_info_container_type gluece_info_container;

  prefetchGlueCEinfo(m_hostname, m_port, m_dn, m_timeout, gluece_info_container);

  do {

    for (gluece_info_iterator it = gluece_info_container.begin();
         it != gluece_info_container.end(); ++it) {
      try {
        expand_information_service_info(it->second);
        fetch_gluece_info(it->second);
        insert_aux_requirements(it->second);
	expand_glueceid_info(it->second);
        boost::mutex::scoped_lock l(get_ism_mutex());
        get_ism().insert(
          make_ism_entry(it->first, get_current_time(), it->second)
        );
      } catch(...) {
        Warning("Caught exception while fetching " << it->first
                << " IS information.");
      }
    }

    if (m_mode == loop) {
      sleep(m_interval);
    }

  } while (m_mode);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

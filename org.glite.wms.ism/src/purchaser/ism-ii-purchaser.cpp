// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$
#include <boost/mem_fn.hpp>
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"

#include "glite/wms/common/ldif2classad/LDAPQuery.h"
#include "glite/wms/common/ldif2classad/LDAPSynchConnection.h"
#include "glite/wms/common/ldif2classad/LDAPForwardIterator.h"
#include "glite/wms/common/ldif2classad/exceptions.h"

#include "glite/wms/common/utilities/ii_attr_utils.h"

using namespace std;

namespace glite {
namespace wms {

namespace ldif2classad  = common::ldif2classad;

namespace ism {
namespace purchaser {

namespace {

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
  }
  catch( ldif2classad::QueryException& e) {

    Warning(e.what());
  }
  catch( ldif2classad::ConnectionException& e) {

    Warning(e.what());
  }
}

bool fetch_gluece_info(gluece_info_type& gluece_info)
{
  std::string is_dn;
  std::string is_host;
  std::string is_URL;
  int is_port;
  try {

    is_URL.assign( utilities::evaluate_attribute(*gluece_info, "GlueInformationServiceURL") );
    static boost::regex expression_gisu( "\\S.*://(.*):([0-9]+)/(.*)" );
    boost::smatch pieces_gisu;
    std::string port;

    if (boost::regex_match(is_URL, pieces_gisu, expression_gisu)) {

      is_host.assign (pieces_gisu[1].first, pieces_gisu[1].second);
      port.assign    (pieces_gisu[2].first, pieces_gisu[2].second);
      is_dn.assign   (pieces_gisu[3].first, pieces_gisu[3].second);

      is_port = std::atoi(port.c_str());
    }
  }
  catch (utilities::InvalidValue& e) {
    Error("Cannot evaluate GlueInformationServiceURL...");
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

} // {anonymous}

ism_ii_purchaser::ism_ii_purchaser(
  std::string const& hostname,
  int port,
  std::string const& distinguished_name,
  int timeout,
  exec_mode_t mode,
  size_t interval
) : ism_purchaser(mode, interval),
  m_hostname(hostname),
  m_port(port),
  m_dn(distinguished_name),
  m_timeout(timeout),
  m_mode(mode),
  m_interval(interval)
{
}

void ism_ii_purchaser::operator()()
{
  do_purchase();
}

void ism_ii_purchaser::do_purchase()
{
  // History of glueceid already inserted into the ISM
  //set<std::string> gluece_info_history;
  do {
    try {
      gluece_info_container_type gluece_info_container;
      prefetchGlueCEinfo(m_hostname, m_port, m_dn, m_timeout, gluece_info_container);
      		
      for (gluece_info_iterator it = gluece_info_container.begin();
          it != gluece_info_container.end(); it++) {

        // Check whether the gluece_info has already been inserted into the ISM, on not...
        //if (gluece_info_history.find(it->first) == gluece_info_history.end()) {

          bool purchasing_ok =
            fetch_gluece_info(it->second) &&
            expand_glueceid_info(it->second);

          if (purchasing_ok) {
            insert_aux_requirements(it->second);
            boost::mutex::scoped_lock l(get_ism_mutex());
            get_ism().insert(make_ism_entry(it->first, get_current_time(), it->second));
          //  gluece_info_history.insert(it->first);
          }
          else {

            Warning("Purchasing from " << it->first << " failed...");
          }
        //}
        //else {
        //  Debug(it->first << "...already in ISM");
        //}
 
      }
      if (m_mode) sleep(m_interval);
    }
    catch (...) { // TODO: Check which exception may arrive here... and remove catching all
      Warning("Failed to purchase info from " << m_hostname << ":" << m_port);
    }
  } while (m_mode);
}

namespace ii {
// the class factories

extern "C" ism_ii_purchaser* create(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout = 30,
    exec_mode_t mode = loop,
    size_t interval = 30) 
{
    return new ism_ii_purchaser(hostname, port, distinguished_name, timeout, mode, interval);
}

extern "C" void destroy(ism_ii_purchaser* p) {
    delete p;
}
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

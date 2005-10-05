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

namespace ldif2classad	= common::ldif2classad;

namespace ism {
namespace purchaser {

namespace {

boost::condition f_purchasing_cycle_run_condition;
boost::mutex     f_purchasing_cycle_run_mutex;

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
void prefetchGlueCEinfo(ldif2classad::LDAPConnection* IIconnection, gluece_info_container_type& gluece_info_container) 
{
  vector<string> reqAttributes;

  reqAttributes.push_back("GlueCEUniqueID");
  reqAttributes.push_back("GlueInformationServiceURL");

  string filter("objectclass=GlueCE");

  ldif2classad::LDAPQuery query(IIconnection, filter, reqAttributes);

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

      while (ldap_it.current()) {

	ldif2classad::LDIFObject ldif_CE(*ldap_it);
	string GlueCEUniqueID;
	ldif_CE.EvaluateAttribute("GlueCEUniqueID", GlueCEUniqueID);

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
  prefetchGlueCEinfo(IIconnection.get(), gluece_info_container);
}

bool fetch_gluece_se_bind_info(ldif2classad::LDAPConnection* IIconnection,
                       std::string const& gluece_id,
                       gluece_info_type& gluece_info)
{
  bool result = false;
  string CloseSE_filter("(&(objectclass=GlueCESEBindGroup)(GlueCESEBindGroupCEUniqueID=" + gluece_id + "))");
  ldif2classad::LDAPQuery CloseSE_query(IIconnection, CloseSE_filter, vector<string>());

  try {
    CloseSE_query.execute();
    if (!CloseSE_query.tuples()->empty()) {
      try {
        ldif2classad::LDAPForwardIterator CloseSE_it( CloseSE_query.tuples() );
        CloseSE_it.first();
	std::vector<classad::ExprTree*>  CloseSE_exprs;
        while (CloseSE_it.current()) {
          vector<string> CloseSEs;
          CloseSE_it->EvaluateAttribute("GlueCESEBindGroupSEUniqueID",CloseSEs);
	
          for(vector<string>::const_iterator it=CloseSEs.begin(); it!=CloseSEs.end(); it++) {

            string CloseSEInfo_filter("(&(objectclass=GlueCESEBind)(GlueCESEBindSEUniqueID=" + *it + "))");
            ldif2classad::LDAPQuery CloseSEInfo_query(IIconnection, CloseSEInfo_filter, vector<string>());
            try {
              CloseSEInfo_query.execute();
              if (!CloseSEInfo_query.tuples()->empty()) {

                ldif2classad::LDAPForwardIterator CloseSEInfo_it( CloseSEInfo_query.tuples() );
                CloseSEInfo_it.first();
                CloseSE_exprs.push_back(new classad::ClassAd());
                string name, mount;
                CloseSEInfo_it->EvaluateAttribute("GlueCESEBindSEUniqueID", name);
		CloseSEInfo_it->EvaluateAttribute("GlueCESEBindCEAccesspoint", mount);
                static_cast<classad::ClassAd*>(CloseSE_exprs.back())->InsertAttr("name" , name);
                static_cast<classad::ClassAd*>(CloseSE_exprs.back())->InsertAttr("mount", mount);
              }
              else {
                Warning("No entries while filtering " << CloseSEInfo_filter << endl);
              }
            }
            catch( ldif2classad::QueryException& e) {
              Warning(e.what() << endl);
            }
          }
          CloseSE_it.next();
        }
        gluece_info->Insert("CloseStorageElements", classad::ExprList::MakeExprList(CloseSE_exprs));
	result = true;
      }
      catch( ldif2classad::LDAPNoEntryEx& ) {
        Warning("No entries while filtering " << CloseSE_query.what() << endl);
      }
    }
    else {
      Warning("No tuples executing query " << CloseSE_query.what() << endl);
    }
  }
  catch( ldif2classad::QueryException& e) {
    Warning(e.what() << endl);
  }
  return result;
}

bool fetch_gluece_info(ldif2classad::LDAPConnection* IIconnection,
		       std::string const& gluece_id,
		       gluece_info_type& gluece_info)
{
  std::vector<std::string> all_attributes;
  std::string filter("(&(objectclass=GlueCE)(GlueCEUniqueID=" + gluece_id + "))");

  ldif2classad::LDAPQuery query(IIconnection, filter, all_attributes);

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
      ldif2classad::LDIFObject ldif_CE(*ldap_it);
      
      string cluster("(&(objectclass=GlueSubCluster)(GlueSubClusterUniqueID=" + getClusterName(ldif_CE) + "))");
      string closese("(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupCEUniqueID=" + gluece_id + "))");

      if (mergeInfo(cluster, *IIconnection, ldif_CE) && 
	  mergeInfo(closese, *IIconnection, ldif_CE)) {
	
	boost::scoped_ptr<classad::ClassAd> 
	  ceAd(ldif_CE.asClassAd(multi_attrs_begin, multi_attrs_end));
	
	gluece_info->Update(*ceAd);
	return true;
      } 
    }

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
  return false;
}
} // {anonymous}

bool ism_ii_purchaser_entry_update::operator()(int a,boost::shared_ptr<classad::ClassAd>& ad)
{
  boost::mutex::scoped_lock l(f_purchasing_cycle_run_mutex);
  f_purchasing_cycle_run_condition.notify_one();
}
  
ism_ii_purchaser::ism_ii_purchaser(
  std::string const& hostname,
  int port,
  std::string const& distinguished_name,
  int timeout,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
) : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_hostname(hostname),
  m_port(port),
  m_dn(distinguished_name),
  m_timeout(timeout)
{
}

void ism_ii_purchaser::operator()()
{
  do_purchase();
}

void ism_ii_purchaser::do_purchase()
{
  do {
    try {
      gluece_info_container_type gluece_info_container;
      vector<gluece_info_iterator> gluece_info_container_updated_entries;

      boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection(
        new ldif2classad::LDAPSynchConnection(m_dn, m_hostname, m_port, m_timeout)
      );
      prefetchGlueCEinfo(IIconnection.get(), gluece_info_container);
      for (gluece_info_iterator it = gluece_info_container.begin();
           it != gluece_info_container.end(); ++it) {

       // Skips CE info according to the specified predicate...
       if (m_skip_predicate.empty() || !m_skip_predicate(it->first)) {
         	
         bool purchasing_ok = 
           fetch_gluece_info(IIconnection.get(), it->first, it->second) &&
           fetch_gluece_se_bind_info(IIconnection.get(), it->first, it->second) &&
           expand_glueceid_info(it->second) &&
           insert_aux_requirements(it->second);
         if (purchasing_ok) {
           it->second->InsertAttr("PurchasedBy","ism_ii_purchaser");
           gluece_info_container_updated_entries.push_back(it);
         }
       }
      }
      {
        boost::recursive_mutex::scoped_lock l(get_ism_mutex());	
        while(!gluece_info_container_updated_entries.empty()) {
	  
          ism_type::value_type ism_entry = make_ism_entry(
            gluece_info_container_updated_entries.back()->first, 
            static_cast<int>(get_current_time().sec), 
            gluece_info_container_updated_entries.back()->second, 
            ism_ii_purchaser_entry_update());

	  get_ism().insert(ism_entry);
          gluece_info_container_updated_entries.pop_back();            
        } // while
      } // unlock the mutex
      if (m_mode) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += m_interval;
        boost::mutex::scoped_lock l(f_purchasing_cycle_run_mutex);
        f_purchasing_cycle_run_condition.timed_wait(l, xt);
      }
    }
    catch (...) { // TODO: Check which exception may arrive here... and remove catch all
      Warning("Failed to purchase info from " << m_hostname << ":" << m_port);
    }
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}

// the class factories
extern "C" ism_ii_purchaser* create_ii_purchaser(std::string const& hostname,
    int port,
    std::string const& distinguished_name,
    int timeout,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate) 
{
    return new ism_ii_purchaser(hostname, port, distinguished_name, timeout, mode, interval, exit_predicate, skip_predicate);
}

extern "C" void destroy_ii_purchaser(ism_ii_purchaser* p) {
    delete p;
}

// the entry update function factory
extern "C" boost::function<bool(int&, ad_ptr)> create_ii_entry_update_fn() 
{
  return ism_ii_purchaser_entry_update();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

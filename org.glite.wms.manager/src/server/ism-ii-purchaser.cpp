// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id: 

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

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "ism-ii-purchaser.h"
#include "ism.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

using namespace std;

namespace exception =glite::wmsutils::exception;

namespace glite {
namespace wms {
	
namespace ldif2classad  = common::ldif2classad;
namespace utilities     = common::utilities;
namespace logger        = common::logger;

namespace manager {
namespace server {

namespace 
{
typedef boost::shared_ptr< classad::ClassAd >      gluece_info_type;		
typedef std::map< std::string, gluece_info_type >  gluece_info_container_type;
typedef gluece_info_container_type::const_iterator gluece_info_const_iterator;
typedef gluece_info_container_type::iterator       gluece_info_iterator;


bool mergeInfo(const string& filter,
			   ldif2classad::LDAPConnection& IIconnection,
			   ldif2classad::LDIFObject& o)
{
	edglog( debug ) << "Filtering and merging: " << filter << endl << flush;
	vector<string> all_attributes;
	ldif2classad::LDAPQuery query(&IIconnection, filter, all_attributes);
	query.execute();
	if(!query.tuples()->empty()) {
		
		ldif2classad::LDAPForwardIterator ldap_it(query.tuples());
		ldap_it.first();
		if(ldap_it.current()) {
			
			o.merge( *ldap_it );
			return true;
		}
		else edglog( error )  << "Bad filter...!" << endl <<flush;
	}
	return false;	
}

string getClusterName (ldif2classad::LDIFObject &ldif_CE)
{
	std::string cluster;
	std::vector<std::string> foreignKeys;
	std::string reg_string;
	ldif_CE.EvaluateAttribute("GlueCEInfoHostName", cluster);
    try {
	    reg_string.assign("GlueClusterUniqueID");
	    reg_string.append("\\s*=\\s*([^\\s]+)");
        ldif_CE.EvaluateAttribute("GlueForeignKey", foreignKeys);
        static boost::regex get_cluid( reg_string );
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
			
			edglog( warning ) << "Cannot find " << "GlueClusterUniqueID" << 
					" assignment. Using " << cluster << "." << endl;
		}
	} catch( ldif2classad::LDAPNoEntryEx& ) {
	
		// The GlueForeignKey was not found.
		// We keep the gatekeeper name.
	} catch( boost::bad_expression& e ){
	
		edglog( error ) << "Bad regular expression " << reg_string <<
				". Cannot parse "<< "GlueForeignKey" << ". Using " << cluster << "." << endl;
	}
	return cluster;
}

/*
 * Prefetch GlueCEUniqueIDs
 * This method obtains the GlueCEUniqueIDs from the information index
 */
void prefetchGlueCEinfo(const std::string& hostname, 
				const int port, 
				const std::string& dn, 
				const int timeout, 
				gluece_info_container_type& gluece_info_container)
{
  edglog_fn(prefetchGlueCEUniqueIDs);     	

  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
  
  IIconnection.reset( new ldif2classad::LDAPSynchConnection( dn,
							     hostname,
							     port,
							     timeout));

  vector<string> reqAttributes;
  
  reqAttributes.push_back("GlueCEUniqueID");
  reqAttributes.push_back("GlueSubClusterUniqueID");
  reqAttributes.push_back("GlueCEInfoHostName"); 
  reqAttributes.push_back("GlueForeignKey"); 
  reqAttributes.push_back("GlueClusterUniqueID");
  reqAttributes.push_back("GlueInformationServiceURL");
  
  string filter("objectclass=GlueCE");
  
  ldif2classad::LDAPQuery query(IIconnection.get(), filter, reqAttributes);
  
  edglog( debug ) << "Filtering Information Index (GlueCEUniqueIDs): " << filter << endl;
  
  try { 

    IIconnection -> open();
    query.execute();
    if( !query.tuples() -> empty() ) {
	
	  utilities::ii_attributes::const_iterator multi_attrs_begin, multi_attrs_end;
	  boost::tie(multi_attrs_begin,multi_attrs_end) = utilities::ii_attributes::multiValued();
		
      ldif2classad::LDAPForwardIterator ldap_it( query.tuples() );   
      ldap_it.first();
	  
	  bool all_merge_query_fail = true;
	  
      while( ldap_it.current() ) {		
			  
		ldif2classad::LDIFObject ldif_CE = (*ldap_it);
		string GlueCEUniqueID;
		ldif_CE.EvaluateAttribute("GlueCEUniqueID", GlueCEUniqueID);
		
		string cluster;
		string closese;

		cluster.assign("(&(objectclass=GlueSubCluster)(GlueSubClusterUniqueID=" + getClusterName(ldif_CE) + "))");
		closese.assign("(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupCEUniqueID=" + GlueCEUniqueID + "))");
		
		if( mergeInfo(cluster, *IIconnection, ldif_CE) &&
			mergeInfo(closese, *IIconnection, ldif_CE) ) {
				
				all_merge_query_fail = false;
		}
		
		boost::shared_ptr<classad::ClassAd> ceAd(ldif_CE.asClassAd(multi_attrs_begin, multi_attrs_end));
		gluece_info_container[GlueCEUniqueID] = ceAd;
	  	ldap_it.next();
      } // while( ldap_it.current() )
    }
  }
  catch( ldif2classad::LDAPNoEntryEx& ) {
    
    edglog( severe ) << "Unexpected result while searching the IS..." << endl;
    //throw matchmaking::ISQueryError(
	//			    NSconf->ii_contact(),
	//			    NSconf->ii_port(),
	//			    NSconf->ii_dn(),
	//			    filter);
  }
  catch( ldif2classad::QueryException& e) {
    
    edglog( warning ) << e.what() << endl;
    //throw matchmaking::ISQueryError( 
	//			    NSconf->ii_contact(),
	//			    NSconf->ii_port(),
	//			    NSconf->ii_dn(),
	//			    filter);
  }
  catch( ldif2classad::ConnectionException& e) {
    
    edglog( warning ) << e.what() << endl;
    //throw matchmaking::ISConnectionError(
	//				 NSconf->ii_contact(),
	//				 NSconf->ii_port(),
	//				 NSconf->ii_dn());
  }
}

bool expand_information_service_info(gluece_info_type& gluece_info)
{
	string isURL;
    bool result = false;
	try {
			
		isURL.assign( utilities::evaluate_attribute(*gluece_info, "GlueInformationServiceURL") );
		static boost::regex  expression_gisu( "\\S.*://(.*):([0-9]+)/(.*)" );
		boost::smatch        pieces_gisu;
		string                isbasedn, isport, ishost;

		if( boost::regex_match(isURL, pieces_gisu, expression_gisu) ) {

	   		ishost.assign  (pieces_gisu[1].first, pieces_gisu[1].second);
    		isport.assign  (pieces_gisu[2].first, pieces_gisu[2].second);
        	isbasedn.assign(pieces_gisu[3].first, pieces_gisu[3].second);

        	gluece_info -> InsertAttr( "InformationServiceDN",   isbasedn);
        	gluece_info -> InsertAttr( "InformationServiceHost", ishost);
        	gluece_info -> InsertAttr( "InformationServicePort", std::atoi(isport.c_str()) );
    		result = true;
		}
	}
    catch( utilities::InvalidValue& e ) {

	   edglog( error ) << "Cannot evaluate GlueInformationServiceURL..." << endl;
	   result = false;	
	}
	return result;
}

bool fetch_gluece_info(gluece_info_type& gluece_info)
{
	std::string is_dn;
	std::string is_host;
	int is_port;
	try {
		is_dn.assign( utilities::evaluate_attribute(*gluece_info, "InformationServiceDN") );
	}
	catch( utilities::InvalidValue& e ) {
		edglog( warning ) << "Cannot evaluate InformationServiceDN..." << endl;
		return false;
	}
	try {
	    is_host.assign( utilities::evaluate_attribute(*gluece_info, "InformationServiceHost") );
	}
	catch( utilities::InvalidValue& e ) {
		edglog( warning ) << "Cannot evaluate InformationServiceHost..." << endl;
		return false;
	}
	try {
	    is_port = utilities::evaluate_attribute(*gluece_info, "InformationServicePort");
	}
	catch( utilities::InvalidValue& e ) {
		edglog( warning ) << "Cannot evaluate InformationServicePort..." << endl;
		return false;
	}
  
	boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
	IIconnection.reset( new ldif2classad::LDAPSynchConnection( is_dn,
							is_host,
							is_port,
							20));
	
	std::vector<std::string> all_attributes;
	std::string filter( "(&(objectclass=GlueCE)(GlueCEUniqueID=" );
	std::string gluece_id( utilities::evaluate_attribute(*gluece_info, "GlueCEUniqueID") );
	
	filter.append(gluece_id).append(std::string("))"));
	
	ldif2classad::LDAPQuery query( IIconnection.get(), filter, all_attributes);

	try {

		IIconnection -> open();
		query.execute();
		if( (!query.tuples() -> empty()) )  {
			
			utilities::ii_attributes::const_iterator multi_attrs_begin, multi_attrs_end;
			boost::tie(multi_attrs_begin,multi_attrs_end) = utilities::ii_attributes::multiValued();

		    ldif2classad::LDAPForwardIterator ldap_it(query.tuples());
		    ldap_it.first();

		    boost::scoped_ptr<classad::ClassAd> ceAd((*ldap_it).asClassAd(multi_attrs_begin, multi_attrs_end));
			(*gluece_info).Update(*ceAd);
		} // if GRIS query not empty
	}
	catch( ldif2classad::ConnectionException& e) {
	    edglog( warning ) << e.what() << endl;
		return false;
    }
    catch( ldif2classad::QueryException& e) {
	    edglog( warning ) << e.what() << endl;
		return false;
	}
    catch( ldif2classad::LDAPNoEntryEx& ) {
		edglog( severe ) << "Unexpected result while searching the IS..." << gluece_id << endl;
		return false;
    }
	return true;
}

}; // anonymous namespace closure

ism_ii_purchaser::ism_ii_purchaser(const std::string& hostname, 
					const int port, 
					const std::string& dn, 
					const int timeout)
{
	this->hostname.assign(hostname);
	this->port = port;
	this->dn.assign(dn);
	this->timeout = timeout;
	this->mode = ism_ii_purchaser::_once_;
	this->interval = 30;
}


void ism_ii_purchaser::operator()() 
{
	gluece_info_container_type gluece_info_container;
	
	prefetchGlueCEinfo(hostname, port, dn, timeout, gluece_info_container);
	
	do {
	
		for(gluece_info_iterator it = gluece_info_container.begin(); 
					it != gluece_info_container.end(); it++) {
			try {
				expand_information_service_info(it->second);
				fetch_gluece_info(it->second);
				boost::mutex::scoped_lock l(get_ism_mutex());
				get_ism().insert(make_ism_entry(it->first, get_current_time(), it->second));
			}
			catch(...) {
				edglog( warning ) << "Caught exception while fetching " << it->first << " IS information." << endl;
			}
		}
		if( mode ) sleep( interval );
	} while( mode );							
}

}}}}

// File: brokerinfoGlueImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
//
// Revision history
// 29-11-2004 new catolog interfaces added. Author: Enzo Martelli <enzo.martelli@mi.infn.it>
//
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <dlfcn.h>

#include <DataLocationInterfaceSOAP.h>
#include <StorageIndexCatalogInterface.h>

#include <ServiceDiscovery.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/regex.hpp>

#include <classad_distribution.h>

#include "glite/wms/common/ldif2classad/LDAPQuery.h"
#include "glite/wms/common/ldif2classad/LDAPSynchConnection.h"
#include "glite/wms/common/ldif2classad/LDAPForwardIterator.h"
#include "glite/wms/common/ldif2classad/exceptions.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/wms/common/utilities/ii_attr_utils.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"

#include "glite/wmsutils/exception/Exception.h"

#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

#include "glite/wms/rls/ReplicaServiceReal.h"

#include "glue_attributes.h"
#include "glite/wms/ism/ism.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

using namespace std;

namespace glite {
namespace wms {

namespace configuration = common::configuration;
namespace ldif2classad  = common::ldif2classad;
namespace utilities     = common::utilities;
namespace logger        = common::logger;
namespace requestad     = jdl;
namespace brokerinfo {

brokerinfoGlueImpl::brokerinfoGlueImpl()
{
}

brokerinfoGlueImpl::~brokerinfoGlueImpl()
{
}

void brokerinfoGlueImpl::retrieveCloseSAsInfo(const BrokerInfoData::VO_name_type& VO, 
					      BrokerInfoData& bid,
					      std::vector<std::string>* additional_attrs)
{
  edglog_fn(retrieveCloseSAsInfo);   
  const configuration::NSConfiguration* NSconf = configuration::Configuration::instance() -> ns();
  bid.m_referredVO.assign( VO );
  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
  
  IIconnection.reset( new ldif2classad::LDAPSynchConnection( NSconf -> ii_dn(),
							     NSconf -> ii_contact(),
							     NSconf -> ii_port(),
							     NSconf -> ii_timeout()) );
  try {
    IIconnection -> open();
    std::vector<std::string> CloseSAattr;
    CloseSAattr.push_back( "GlueSAStateAvailableSpace" );
  
    if( additional_attrs && !additional_attrs->empty() ) {
    
      CloseSAattr.insert( CloseSAattr.begin(), 
			  additional_attrs->begin(), 
			  additional_attrs->end() );
    }
    for(BrokerInfoData::CloseSEInfo_iterator it = bid.m_CloseSEInfo_map.begin();
	it != bid.m_CloseSEInfo_map.end(); it++) { 
      const string CloseSE(it->first);
      std::string CloseSAInfo_filter;
      CloseSAInfo_filter.assign("(&(&(objectclass=GlueSA)(GlueChunkKey=GlueSEUniqueID=" + CloseSE + 
				")(GlueSAAccessControlBaseRule=" + VO + ")))");
    
      ldif2classad::LDAPQuery CloseSAInfo_query(IIconnection.get(), CloseSAInfo_filter, CloseSAattr);
      try {
    
	CloseSAInfo_query.execute();
	if( !CloseSAInfo_query.tuples() -> empty() ) {
        
	  ldif2classad::LDAPForwardIterator CloseSAInfo_it( CloseSAInfo_query.tuples() );
	  CloseSAInfo_it.first();
	  boost::scoped_ptr<classad::ClassAd> ad((*CloseSAInfo_it).asClassAd());
	  bid.m_CloseSEInfo_map[CloseSE]->Update(*ad);
	}
	else {
	  edglog(warning) << "InformationIndex search (no tuples): " 
			  <<  CloseSAInfo_query.what() << endl;
	}
      }
      catch( ldif2classad::QueryException& e) {
    
	edglog( warning ) << e.what() << endl;
      }
      catch( ldif2classad::LDAPNoEntryEx&) { 
      
	edglog(warning) << "InformationIndex search (no entry): " << CloseSAInfo_query.what() << endl;    
      }    
    }
  }
  catch( ldif2classad::ConnectionException& e) {
   
    edglog(warning) << e.what() << endl;
  }
}

bool  brokerinfoGlueImpl::retrieveCloseSEsInfoFromISM(const BrokerInfoData::CEid_type& CEid,
						      BrokerInfoData& bid) {
  bool result = true;
  edglog_fn(retrieveCloseSAsInfoFromISM);
  bid.m_CloseSEInfo_map.clear();
  bid.m_referredCEid.assign( CEid );
  ism::ism_mutex_type::scoped_lock l(ism::get_ism_mutex());
  ism::ism_type::const_iterator ce_it = ism::get_ism().find(CEid);
  if (ce_it != ism::get_ism().end()) {
    // Retrieve the CloseStorageElements expression list
    classad::Value value;
    const classad::ExprList *adList;
    if (boost::tuples::get<2>((*ce_it).second)->EvaluateAttr("CloseStorageElements", value) &&
	value.IsListValue(adList)) {
      vector<classad::ExprTree*> ads;
      adList->GetComponents(ads);
      for (vector<classad::ExprTree*>::const_iterator expr_it = ads.begin();
	   expr_it != ads.end(); expr_it++) {
	
	// Check if the expression is a classad...
	if ((*expr_it)->GetKind() == classad::ExprTree::CLASSAD_NODE) {
	  string SEid;
	  string SEmount;
	  static_cast<classad::ClassAd*>(*expr_it)->EvaluateAttrString("name", SEid);
	  static_cast<classad::ClassAd*>(*expr_it)->EvaluateAttrString("mount", SEmount);
	  BrokerInfoData::CloseSEInfo_type CloseSEInfo(new classad::ClassAd(*static_cast<classad::ClassAd*>(*expr_it)));
	  CloseSEInfo->InsertAttr("GlueCESEBindCEAccessPoint", SEmount);
	  bid.m_CloseSEInfo_map[SEid] = CloseSEInfo;
	  edglog( debug ) << CEid << " is close to " << SEid << " mountable on " << SEmount << endl;
	}
	else { result = false; break;}
      }
    }
    else { result = false; }
  }
  else { result = false; }
  return result;
}

void brokerinfoGlueImpl::retrieveCloseSEsInfo(const BrokerInfoData::CEid_type& CEid, 
					      BrokerInfoData& bid, std::vector<std::string>* additional_attrs)
{ 
  // Patch to look up the ISM for CESEBinding information first
  if(retrieveCloseSEsInfoFromISM(CEid, bid)) return;

  edglog_fn(retrieveCloseSAsInfo);  
  const configuration::NSConfiguration* NSconf = configuration::Configuration::instance() -> ns();
  bid.m_CloseSEInfo_map.clear();
  bid.m_referredCEid.assign( CEid );
  std::vector<std::string> CloseSEattr;
  CloseSEattr.push_back("GlueCESEBindGroupSEUniqueID");
  
  std::string CloseSE_filter;
  CloseSE_filter = "(&(objectclass=GlueCESEBindGroup)(GlueCESEBindGroupCEUniqueID=" + CEid + "))";
  
  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
  
  IIconnection.reset( new ldif2classad::LDAPSynchConnection( NSconf -> ii_dn(),
							     NSconf -> ii_contact(),
							     NSconf -> ii_port(),
							     NSconf -> ii_timeout()) );
    
  ldif2classad::LDAPQuery CloseSE_query(IIconnection.get(), CloseSE_filter, CloseSEattr);
  
  try {
    
    IIconnection -> open();
    CloseSE_query.execute();    
    if( !CloseSE_query.tuples() -> empty() ) {
      try {
        
	ldif2classad::LDAPForwardIterator CloseSE_it( CloseSE_query.tuples() );   
	CloseSE_it.first();
	while( CloseSE_it.current() ) {
                
	  vector<string> CloseSEs;
	  (*CloseSE_it).EvaluateAttribute("GlueCESEBindGroupSEUniqueID",CloseSEs);
            
	  vector<string> CloseSEInfo_attr;
	  CloseSEInfo_attr.push_back(GS_GLUECE_SEBIND_ACCESSPOINT);
      
	  for(vector<string>::const_iterator it=CloseSEs.begin(); 
	      it!=CloseSEs.end(); it++) {
      
	    string CloseSEInfo_filter("(&(objectclass=GlueCESEBind)(" + 
				      string(GS_GLUECE_SEBIND_SEUNIQUEID) + string("=") + *it + string("))"));
      
	    ldif2classad::LDAPQuery CloseSEInfo_query(IIconnection.get(), CloseSEInfo_filter, CloseSEInfo_attr);
	    try {
	      CloseSEInfo_query.execute();    
	      if( !CloseSEInfo_query.tuples() -> empty() ) {
      
		ldif2classad::LDAPForwardIterator CloseSEInfo_it( CloseSEInfo_query.tuples() );   
		CloseSEInfo_it.first();
		ldif2classad::LDIFObject ldif_CloseSE = (*CloseSEInfo_it);
		if( additional_attrs && !additional_attrs->empty() ) {
      
		  string SE_info_filter("(&(objectclass=GlueSE)(GlueSEUniqueId=" + *it + "))");
		  edglog( debug ) << "Filtering Information Index: " << SE_info_filter << endl;
		  ldif2classad::LDAPQuery SE_info_query(IIconnection.get(), SE_info_filter, *additional_attrs);
		  try {
		    SE_info_query.execute();
		    if(!SE_info_query.tuples()->empty()) {
                  
		      ldif2classad::LDAPForwardIterator SE_info_it(SE_info_query.tuples());
		      SE_info_it.first();
		      if(SE_info_it.current()) {
                          
			ldif_CloseSE.merge(*SE_info_it);
		      }
		    }
		  }
		  catch( ldif2classad::QueryException& e) {
      
		    edglog( warning ) << e.what() << endl;
		  }
		}
		utilities::ii_attributes::const_iterator attrs_begin, attrs_end;
		boost::tie(attrs_begin,attrs_end) = utilities::ii_attributes::multiValued();
		BrokerInfoData::CloseSEInfo_type CloseSEInfo( ldif_CloseSE.asClassAd(attrs_begin, attrs_end) );
		bid.m_CloseSEInfo_map[*it] = CloseSEInfo;
	      }
	      else {
        
		edglog(warning) << "InformationIndex search (no tuples): " <<  CloseSEInfo_query.what() << endl;
	      }
	    } 
	    catch( ldif2classad::QueryException& e) {
          
	      edglog( warning ) << e.what() << endl;
	    }
	  }  
	  CloseSE_it.next();
	}
      } 
      catch( ldif2classad::LDAPNoEntryEx& ) {
      
	edglog(warning) << "InformationIndex search (no entry): " << CloseSE_query.what() << endl;
      }
    }
    else {
    
      edglog(warning) << "InformationIndex search (no tuples): " << CloseSE_query.what() << endl;
    }
  }
  catch( ldif2classad::QueryException& e) {
    
    edglog(warning) << e.what() << endl;
  }
  catch( ldif2classad::ConnectionException& e) {
    
    edglog(warning) << e.what() << endl;
  }
}

void brokerinfoGlueImpl::retrieveSEsInfo(const classad::ClassAd& requestAd, BrokerInfoData& bid)
{
  edglog_fn(retrieveSEsInfo);  
  const configuration::NSConfiguration* NSconf = configuration::Configuration::instance() -> ns();
  
  vector<string> attributes;
  
  attributes.push_back(GS_GLUESE_ACCESSPROTOCOL_TYPE);
  attributes.push_back(GS_GLUESE_ACCESSPROTOCOL_PORT);
  BrokerInfoData::SE_const_iterator se_begin, se_end;
  boost::tie(se_begin,se_end) = bid.involvedSEs();
  for (BrokerInfoData::SE_const_iterator it = se_begin; it != se_end; it++) {    
    string filter;  
    filter = "(&(objectclass=GlueSEAccessProtocol)(GlueChunkKey=GlueSEUniqueID=" + (*it) + "))";
    
    boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
    IIconnection.reset( new ldif2classad::LDAPSynchConnection( NSconf -> ii_dn(),
							       NSconf -> ii_contact(),
							       NSconf -> ii_port(),
							       NSconf -> ii_timeout()) );
    
    ldif2classad::LDAPQuery query(IIconnection.get(),filter,attributes);
    
    try {
      
      IIconnection -> open();
      query.execute();
      
      if( !query.tuples() -> empty() ) {
  
	try {
    
	  ldif2classad::LDAPForwardIterator ldap_it( query.tuples() );   
	  ldap_it.first();
	  while( ldap_it.current() ) {
      
	    BrokerInfoData::protocol_name protocol;
	    std::string str_port;
      
	    (*ldap_it).EvaluateAttribute(GS_GLUESE_ACCESSPROTOCOL_TYPE, protocol);
	    (*ldap_it).EvaluateAttribute(GS_GLUESE_ACCESSPROTOCOL_PORT, str_port);
      
	    bid.m_SE2Protocol_map[(*it)].push_back( std::make_pair(protocol,std::atoi(str_port.c_str())) );
	    ldap_it.next();
	  }
	} 
	catch( ldif2classad::LDAPNoEntryEx&) {
	  edglog(warning) << "InformationIndex search (no entry): " <<  query.what() << endl;
	}
      } 
      else {
	edglog(warning) << "InformationIndex search (no tuples): " << query.what() << endl;  
      }
    } 
    catch( ldif2classad::QueryException& e) {
      edglog(warning) << e.what() << endl;
    }
    catch ( ldif2classad::ConnectionException& e) {
      edglog(warning) << e.what() << endl;
    }
  }
}


void brokerinfoGlueImpl::retrieveSFNsInfo(const classad::ClassAd& requestAd, BrokerInfoData& bid)
{
   edglog_fn(retrieveSFNsInfo);
 
   bool rlsInUse = false;
   bool dli_siciInUse = false;

   dli::create_dli_t* createDli; dli::create_dli_with_timeout_t* createDli_with_timeout;
   dli::destroy_dli_t* destroyDli; 
   dli::DataLocationInterfaceSOAP* dli;

   rls::create_t* createRls;  rls::destroy_t* destroyRls; 
   rls::ReplicaServiceReal* replica;

   sici::create_t* createSici;  sici::create_t_with_timeout* createSici_with_timeout;
   sici::destroy_t* destroySici;
   sici::StorageIndexCatalogInterface* sici;

   string rlsLib = "libglite_wms_rls.so";
   string dli_siciLib = "libglite_wms_dli_sici.so";

   void *rlsLibHandle;
   void *dli_siciLibHandle;

                                                                                                        
   int timeout = (configuration::Configuration::instance()->ns())->dli_si_catalog_timeout();

   std::string dli_service_name = (configuration::Configuration::instance()->wm())->dli_service_name();
   std::string si_service_name = (configuration::Configuration::instance()->wm())->si_service_name();

   bool  rlsConfig = false; // Determine if RLS is configured

   bool  voInJdl   = false;
   string   vo = "";
                                                                                                
   edglog(debug) << "ClassAd..." << endl;
   edglog(debug) << requestAd << endl;

   try {
      // The vo in JDL is needed for RLS Catalog queries
      //
      vo = requestad::get_virtual_organisation(requestAd);
      voInJdl = true;
   }
   catch (...){
   }
                                                                                                
   // Check if we have the RLS configured for the VO. If this is the case,
   // requests for InputDataType lfn and guid are sent to RLS.
   //
   if ( voInJdl ) {
      if (checkRlsUsage(vo) == 0) {
         rlsConfig = true;
      }
   }

   bool dataReq = false;
   classad::ExprTree* classAdList;
   try {
      classAdList = glite::wms::jdl::get_data_requirements(requestAd);
      dataReq = true; 
   }
   catch(...) {
   }

   if ( dataReq ) {

      vector<string> dli_url_list; bool getDliFromIS = false;
//      if ( voInJdl ) {
//         get_catalog_url( vo, dli_service_name, dli_url_list );
//         if ( ! dli_url_list.empty() ) getDliFromIS = true;
//      }

      vector<string> si_url_list; bool getSiFromIS = false;
//      if ( voInJdl ) {
//         get_catalog_url( vo, si_service_name, si_url_list );
//         if ( ! si_url_list.empty() ) getSiFromIS = true;
//      }

      classad::ExprList* expr_list = static_cast<classad::ExprList*>(classAdList) ;
      for ( classad::ExprList::iterator it = expr_list->begin(); it < expr_list->end(); it++ ){

         string dataCatalogType;
         bool getDataCatalog = false;
         try {
            dataCatalogType = glite::wms::jdl::get_data_catalog_type( *( static_cast<classad::ClassAd*>(*it) ) );
            getDataCatalog = true;
         } 
         catch (...){
         }

         string dataCatalogEndpoint;
         bool getDataCatalogEndpoint = false;
         try {
            vector<string> v;
            requestad::get_data_catalog(*( static_cast<classad::ClassAd*>(*it) ), v);
            dataCatalogEndpoint = v[0]; 
            getDataCatalogEndpoint = true;
         }
         catch (...){
            if ( dataCatalogType == "DLI" && (!getDliFromIS)) {
               if ( voInJdl ) {
                  get_catalog_url( vo, dli_service_name, dli_url_list );
                  if ( ! dli_url_list.empty() ) getDliFromIS = true;
               }
            }
            if ( dataCatalogType == "SI" && (!getSiFromIS) ) {
               if ( voInJdl ) {
                  get_catalog_url( vo, si_service_name, si_url_list );
                  if ( ! si_url_list.empty() ) getSiFromIS = true;
               }
            }
         }

         if ( getDataCatalog ) {

            bool validCatalogType = true;

            if ( dataCatalogType == "RLS" ) {

               if ( !rlsInUse ) {

                  // RLSCatalog property in WMS configuration not used anymore
                  //if ( rlsConfig ) {

                     rlsLibHandle = dlopen (rlsLib.c_str(), RTLD_NOW);
                     if (rlsLibHandle == NULL) {
                        edglog(warning) << "cannot load RLS helper lib " << rlsLib << endl;
                        edglog(warning) << "dlerror returns: " << dlerror() << endl;
                     }
                     else {
                        createRls = (rls::create_t*)dlsym(rlsLibHandle,"create");
                        destroyRls = (rls::destroy_t*)dlsym(rlsLibHandle,"destroy");
                        if (!createRls || !destroyRls) {
                           edglog(warning) << "cannot load RLS helper symbols" << endl;
                           edglog(warning) << "dlerror returns: " << dlerror() << endl;
                           dlclose(rlsLibHandle);
                        }
                        else {
                           replica = createRls(vo);
                           rlsInUse = true;
                        }
                     }

                  //}
                  //else {
                  //   if ( voInJdl ) {
                  //      edglog(warning) << "RLS catalog is not set for " << vo << endl;
                  //   }
                  //   else {
                  //      edglog(warning) << "JDL doesn't contain the VO" << endl;
                  //   }
                  //}

               } //if ( !rlsInUse )

            }
            else if ( dataCatalogType == "DLI" || dataCatalogType == "SI") {

               if ( !dli_siciInUse ) {

                     dli_siciLibHandle = dlopen (dli_siciLib.c_str(), RTLD_NOW);
                     if (dli_siciLibHandle == NULL) {
                        edglog(warning) << "cannot load DLI_SI helper lib " << dli_siciLib << endl;
                        edglog(warning) << "dlerror returns: " << dlerror() << endl;
                     }
                     else {
                        bool dlsym_Ok = true;
                        if ( timeout == 0 ) {
                           createDli = (dli::create_dli_t*)dlsym(dli_siciLibHandle,"create_dli");
                           destroyDli = (dli::destroy_dli_t*)dlsym(dli_siciLibHandle,"destroy_dli");
                           createSici = (sici::create_t*)dlsym(dli_siciLibHandle,"create");
                           destroySici = (sici::destroy_t*)dlsym(dli_siciLibHandle,"destroy");

                           if (!createDli || !destroyDli || !createSici || !destroySici) {
                              dlsym_Ok = false;
                              edglog(warning) << "cannot load DLI_SI helper symbols" << endl;
                              edglog(warning) << "dlerror returns: " << dlerror() << endl;
                              dlclose(dli_siciLibHandle);
                           }
                        }
                        else {
                           createDli_with_timeout = (dli::create_dli_with_timeout_t*)dlsym(dli_siciLibHandle,"create_dli_with_timeout");
                           destroyDli = (dli::destroy_dli_t*)dlsym(dli_siciLibHandle,"destroy_dli");
                           createSici_with_timeout = (sici::create_t_with_timeout*)dlsym(dli_siciLibHandle,"create_with_timeout");
                           destroySici = (sici::destroy_t*)dlsym(dli_siciLibHandle,"destroy");


                           if (!createDli_with_timeout || !destroyDli || !createSici_with_timeout || !destroySici) {
                              dlsym_Ok = false;
                              edglog(warning) << "cannot load DLI_SI helper symbols" << endl;
                              edglog(warning) << "dlerror returns: " << dlerror() << endl;
                              dlclose(dli_siciLibHandle);
                           }
                        }
                        if ( dlsym_Ok ) {
                           if ( timeout == 0 ) {
                              dli = createDli();
                              sici = createSici();
                           }
                           else {
                              dli = createDli_with_timeout(timeout);
                              sici = createSici_with_timeout(timeout);
                           }
                           dli_siciInUse = true;
                        }
                     }
//                  }
//                  else {
//                     edglog(warning) << "cannot get any dli endpoint, neither from JDL nor from IS" << endl;
//                  }

               } // if ( !dli_siciInUse )

                 
            }
            else {
               validCatalogType = false; 
               edglog(warning) <<dataCatalogType<<": unknown DataCatalogType type" << std::endl;
            }

            if ( validCatalogType ) {
               // the list of LFNs from the JDL
               BrokerInfoData::LFN_container_type input_data;
               bool getInputData = false;
               try {
                  requestad::get_input_data(*( static_cast<classad::ClassAd*>(*it) ), input_data);
                  getInputData = true;
               }
               catch (...){
                  edglog(error) << "cannot get input data from jdl" << endl;
               }
               if ( getInputData ){
                  // the list of SFNs matching each LFN
                  BrokerInfoData::SFN_container_type resolved_sfn;

                  for(BrokerInfoData::LFN_container_type::const_iterator lfn = input_data.begin(); lfn != input_data.end();lfn++){
                     try {

                        edglog(debug) << "trying to resolve " << *lfn << std::endl;
                        // the list of SFNs matching each LFN
                        BrokerInfoData::SFN_container_type resolved_sfn;
   
                        if ( rlsInUse && (dataCatalogType == "RLS") ) {
                           resolved_sfn = replica->listReplica(*lfn);
                        }
                        else if ( dli_siciInUse && (dataCatalogType == "DLI") ) {
///////////////////////////////////// this piece of code handles multiple endpoint
                           string prefix = "";
                           if ((*lfn).find("lds") == 0) prefix = "lds";
                           else if ((*lfn).find("query") == 0) prefix = "query";
                           else if  ((*lfn).find("lfn") == 0) prefix = "lfn";
                           else if ((*lfn).find("guid") == 0) prefix = "guid";
                           else edglog(warning) << "unknown prefix while using DLI" << std::endl;

                           if  ( prefix != "" ) {
                              if ( getDataCatalogEndpoint ) {
                                 try {
                                    resolved_sfn = dli->listReplicas(prefix.c_str(), *lfn, requestAd, dataCatalogEndpoint);
                                 }
                                 catch (string& faultstring){
                                    edglog(warning) << faultstring << endl;
                                 }
                              }
                              else if ( getDliFromIS ) {
                                 for ( vector<string>::const_iterator it=dli_url_list.begin(); it != dli_url_list.end(); it++){
                                    try{
                                       resolved_sfn = dli->listReplicas(prefix.c_str(), *lfn, requestAd, *it);
                                       if ( ! resolved_sfn.empty() ) break;
                                    }
                                    catch(string& faultstring) {
                                    //
                                    // Catch soap exceptions from the DataLocationInterface or StorageIndex Interface
                                    // or any other exception due to failures in getting the proxy from the classad
                                    edglog(warning) << faultstring << endl;
                                    }
                                 } 
                              } 
                              else 
                                 edglog(warning) << "Cannot get any dli endpoint, neither from JDL nor from IS" << endl;
                           }
//////////////////////////////////////////////////////////end
                        }
                        else if ( dli_siciInUse && (dataCatalogType == "SI") ) {

                           string noPrefix = *lfn;
                                                                                                                                
                           string::size_type colon_pos;
                           if ((colon_pos = noPrefix.find(":"))!=string::npos) {
                              // Remove any prefix before the leading colon. Including the colon.
                              colon_pos++;
                              noPrefix.erase(0,colon_pos);
                           }
/////////////////////////////////////////////this piece of code handles multiple endpoint
                           if ( getDataCatalogEndpoint ) { 
                              try {
                                 if ( (*lfn).find("lfn") == 0 ) {
                                    sici->listSEbyLFN(noPrefix.c_str(), resolved_sfn, requestAd, dataCatalogEndpoint);
                                 }
                                 else if ( (*lfn).find("guid") == 0 ){
                                    sici->listSEbyGUID(noPrefix.c_str(), resolved_sfn, requestAd, dataCatalogEndpoint);
                                 }
                                 else edglog(warning) << "unknown prefix while using SI" << std::endl;
                              }
                              catch (string& faultstring) {
                                 edglog(warning) << faultstring << endl;
                              }
                           }
                           else if ( getSiFromIS ){
                              for ( vector<string>::const_iterator it=si_url_list.begin(); it != si_url_list.end(); it++){
                                 try{
                                    if ( (*lfn).find("lfn") == 0 ) {
                                       sici->listSEbyLFN(noPrefix.c_str(), resolved_sfn, requestAd, *it);
                                    }
                                    else if ( (*lfn).find("guid") == 0 ){
                                       sici->listSEbyGUID(noPrefix.c_str(), resolved_sfn, requestAd, *it);
                                    }
                                    else {
                                       edglog(warning) << "unknown prefix while using SI" << std::endl;
                                       break;
                                    }
   
                                    if ( ! resolved_sfn.empty() ) break;
                                 }
                                 catch(string& faultstring) {
                                    //
                                    // Catch soap exceptions from the DataLocationInterface or StorageIndex Interface
                                    // or any other exception due to failures in getting the proxy from the classad
                                    edglog(warning) << faultstring << endl;
                                 } 
                              }
                           }
                           else
                              edglog(warning) << "Cannot get any si endpoint, neither from JDL nor from IS" << endl;
/////////////////////////////////////////////////////////////end
                        }
                        else edglog(warning) << "cannot perform " << *lfn <<"'s resolution" << std::endl;
   
                        if( !resolved_sfn.empty() ) {
                           put_results_in_bi_data( *lfn, resolved_sfn, bid);
                        }
                        else {
                           edglog(debug) << "No replica(s) found!" << endl;
                        }
         
                     }
                     catch( std::exception& ex ) {
                        edglog(warning) <<  ex.what() << endl;
                     }
                                                                                                                                
                  } // for

               
               } // if ( getInputData )
       

            }  // if  ( validCatalogType )
   

         } // ( getDataCatalog )
         else edglog(warning) << "cannot get DataCatalogType" << std::endl;
            
      } // for



   }
   else {
      // This block assure the backward compatibility for what concern the data format in the JDL
      //
      // Due to the different SOAP versions used in the three catalog interfaces, we load the
      // client dynamically as a plug-in in order to avoid SOAP version
      // clashes.
      //
      // We check the prefix of InputData's fields we got from the JDL. We have the following
      // possibilities:
      //
      // PREFIX                                 CATALOG to be used
      //
      // 1)lfn, guid                            SI if the SIendpoint is set in the JDL
      //                                        i.e.:StorageIndex="http://lxb2021.cern.ch:9992/AliEn/Service/FC";
      //
      // 2)lfn, guid                            RLS if the SIendpoint is NOT set in the JDL -AND-
      //                                            RLSCatalog is set for the given VO.
      //                                        If RLS doesn't work for any reason, we try DLI
      //
      // 3)si-guid, si-lfn                      StorageIndex, no matter if the endpoint is set or not
      //
      // 4)query, lds                           DLI
      //
      //
      const string   lfnPrefix = "lfn";
      const string   guidPrefix = "guid";
      const string   ldsPrefix = "lds";
      const string   queryPrefix = "query";
      const string   silfnPrefix = "si-lfn";
      const string   siguidPrefix = "si-guid";
   
      string   dliEndpoint = "";
      string   siciEndpoint = "";

      vector<string> dli_url_list;
      vector<string> si_url_list;

      bool     dliEndpointSetInJdl = false;
      bool     dliEndpointSetInIS = false;
      bool     dliEndpointSet = false;
                                                                                                                                
      bool     siciEndpointSetInJdl = false;
      bool     siciEndpointSetInIS = false;
      bool     siciEndpointSet = false;
   
      // Check if Storage Index Catalog and Data Catalog endpoints are set in the jdl
      // If it is not the case, it try to get them from the Information Service
      try {
         vector<string> v;
         requestad::get_data_catalog(requestAd, v);
         dliEndpoint = v[0];
         dliEndpointSetInJdl = true;
      }
      catch(...) {
         if ( voInJdl ) {
            get_catalog_url(vo, dli_service_name, dli_url_list );
            if  ( ! dli_url_list.empty() ) {
               dliEndpointSetInIS = true;
            }
         }
      }
      try {
         vector<string> v;
         requestad::get_storage_index(requestAd, v);
         siciEndpoint = v[0];
         siciEndpointSetInJdl = true;
      }
      catch(...) {
         if ( voInJdl ) {
            get_catalog_url(vo, si_service_name, si_url_list);
            if  ( ! si_url_list.empty() ) {
               siciEndpointSetInIS = true;
            }
         }
      }
                                                                                                                                
      if ( siciEndpointSetInIS || siciEndpointSetInJdl ) {
         siciEndpointSet = true;
      }

      if ( dliEndpointSetInIS || dliEndpointSetInJdl ) {
         dliEndpointSet = true;
      }

   
      BrokerInfoData::LFN_container_type input_data;
   
      try {
         // The InputData field is used to specify a list of LFN, GUID, LDS or Query
         //
         requestad::get_input_data(requestAd, input_data);
      }
      catch (...){
         edglog(error) << "cannot get input data from jdl" << endl;
         return;
      }
   
      for(BrokerInfoData::LFN_container_type::const_iterator lfn = input_data.begin(); lfn != input_data.end();lfn++) {
   
         edglog(debug)  << "trying to resolve: " << *lfn << endl;
   
         try {
   
            BrokerInfoData::SFN_container_type resolved_sfn;
   
            bool lfn_or_guid_found = ((*lfn).find(lfnPrefix.c_str()) == 0) || ((*lfn).find(guidPrefix.c_str()) == 0);
            bool rls_success = false;
   
            if ( lfn_or_guid_found && (!siciEndpointSetInJdl) ) {
   
               if ( !rlsInUse ) {
   
                  if ( rlsConfig ) {
                     rlsLibHandle = dlopen (rlsLib.c_str(), RTLD_NOW);
                     if (rlsLibHandle == NULL) {
                        edglog(warning) << "cannot load RLS helper lib " << rlsLib << endl;
                        edglog(warning) << "dlerror returns: " << dlerror() << endl;
                     }
                     else {
                        createRls = (rls::create_t*)dlsym(rlsLibHandle,"create");
                        destroyRls = (rls::destroy_t*)dlsym(rlsLibHandle,"destroy");
                        if (!createRls || !destroyRls) {
                           edglog(warning) << "cannot load RLS helper symbols" << endl;
                           edglog(warning) << "dlerror returns: " << dlerror() << endl;
                           dlclose(rlsLibHandle);
                        }
                        else {
                           replica = createRls(vo);
                           rlsInUse = true;
                        }
                     }
                  }
                  else {
                     if ( voInJdl ) { 
                        edglog(warning) << "RLS catalog is not set for " << vo << endl;
                     }
                     else {
                        edglog(warning) << "JDL doesn't contain the VO" << endl;
                     }
                  }
               } //if ( !rlsInUse ) 
   
               if ( rlsInUse ) {
                  resolved_sfn = replica->listReplica(*lfn);
                  if( !resolved_sfn.empty() ) rls_success = true;
               }
   
            } // if ( lfn_or_guid_found && (!siciEndpointSetInJdl) )
   
   
            // If the prefix is "lfn" or "guid" but we didn't manage to load the rls plug-in
            //      or the RLSCatalog is not configured for the given vo 
            //      or RLS didn't work for whatever reason,
            //      we try with the dli catalog

            bool dli_to_be_used =  
                 // conditions under which dli is used
                 (    ((*lfn).find(ldsPrefix.c_str()) == 0) || ((*lfn).find(queryPrefix.c_str()) == 0)       ) ||
                 (    ( lfn_or_guid_found && (!siciEndpointSetInJdl) ) && (!rls_success)                     ) ;

            bool sici_to_be_used = 
                 // conditions under which si is used
                 (    ((*lfn).find(silfnPrefix.c_str()) == 0) || ((*lfn).find(siguidPrefix.c_str()) == 0)    ) ||
                 (    lfn_or_guid_found && siciEndpointSetInJdl                                              ) ;

   
            if ( dli_to_be_used || sici_to_be_used ) {

               if ( !dli_siciInUse ) {
                     dli_siciLibHandle = dlopen (dli_siciLib.c_str(), RTLD_NOW);
                     if (dli_siciLibHandle == NULL) {
                        edglog(warning) << "cannot load DLI_SI helper lib " << dli_siciLib << endl;
                        edglog(warning) << "dlerror returns: " << dlerror() << endl;
                     }
                     else {
                        bool dlsym_Ok = true;
                        if ( timeout == 0 ) {
                           createDli = (dli::create_dli_t*)dlsym(dli_siciLibHandle,"create_dli");
                           destroyDli = (dli::destroy_dli_t*)dlsym(dli_siciLibHandle,"destroy_dli");
                           createSici = (sici::create_t*)dlsym(dli_siciLibHandle,"create");
                           destroySici = (sici::destroy_t*)dlsym(dli_siciLibHandle,"destroy");
                                                                                                                             
                           if (!createDli || !destroyDli || !createSici || !destroySici) {
                              dlsym_Ok = false;
                              edglog(warning) << "cannot load DLI_SI helper symbols" << endl;
                              edglog(warning) << "dlerror returns: " << dlerror() << endl;
                              dlclose(dli_siciLibHandle);
                           }
                        }
                        else {
                           createDli_with_timeout = (dli::create_dli_with_timeout_t*)dlsym(dli_siciLibHandle,"create_dli_with_timeout");
                           destroyDli = (dli::destroy_dli_t*)dlsym(dli_siciLibHandle,"destroy_dli");
                           createSici_with_timeout = (sici::create_t_with_timeout*)dlsym(dli_siciLibHandle,"create_with_timeout");
                           destroySici = (sici::destroy_t*)dlsym(dli_siciLibHandle,"destroy");
                                                                                                                             
                                                                                                                             
                           if (!createDli_with_timeout || !destroyDli || !createSici_with_timeout || !destroySici) {
                              dlsym_Ok = false;
                              edglog(warning) << "cannot load DLI_SI helper symbols" << endl;
                              edglog(warning) << "dlerror returns: " << dlerror() << endl;
                              dlclose(dli_siciLibHandle);
                           }
                        }
                        if ( dlsym_Ok ) {
                           if ( timeout == 0 ) {
                              dli = createDli();
                              sici = createSici();
                           }
                           else {
                              dli = createDli_with_timeout(timeout);
                              sici = createSici_with_timeout(timeout);
                           }
                           dli_siciInUse = true;
                        }
                     }

               } // if( !dli_siciInUse )


               if ( dli_to_be_used && !dliEndpointSet )
                  edglog(warning) << "cannot find dli endpoint" << endl;
               else if ( sici_to_be_used && !siciEndpointSet )
                  edglog(warning) << "cannot find si endpoint" << endl;

   
               if ( dli_siciInUse && dliEndpointSet && dli_to_be_used) { 
///////////////////////////////////// this piece of code handles multiple endpoint
                  string prefix = "";
                  if ((*lfn).find(ldsPrefix.c_str()) == 0) prefix = ldsPrefix;
                  else if ((*lfn).find(queryPrefix.c_str()) == 0) prefix = queryPrefix;
                  else if  ((*lfn).find(lfnPrefix.c_str()) == 0) prefix = lfnPrefix;
                  else if ((*lfn).find(guidPrefix.c_str()) == 0) prefix = guidPrefix;
                                                                                                                             
                  if ( dliEndpointSetInJdl ) {
                     try{
                        resolved_sfn = dli->listReplicas(prefix.c_str(), *lfn, requestAd, dliEndpoint);
                     }
                     catch (string& faultstring) {
                        edglog(warning) << faultstring << endl;
                     } 
                  }
                  else if ( dliEndpointSetInIS ) {
                     for ( vector<string>::const_iterator it=dli_url_list.begin(); it != dli_url_list.end(); it++){
                        try{
                           resolved_sfn = dli->listReplicas(prefix.c_str(), *lfn, requestAd, *it);
                           if ( ! resolved_sfn.empty() ) break;
                        }
                        catch(string& faultstring) {
                           //
                           // Catch soap exceptions from the DataLocationInterface or StorageIndex Interface
                           // or any other exception due to failures in getting the proxy from the classad
                           edglog(warning) << faultstring << endl;
                        }
                     }
                  }
//////////////////////////////////////////////////////////end
               }

               if ( dli_siciInUse && siciEndpointSet && sici_to_be_used) { 
                  string noPrefix = *lfn;
   
                  string::size_type colon_pos;
                  if ((colon_pos = noPrefix.find(":"))!=string::npos) {
                     // Remove any prefix before the leading colon. Including the colon.
                     colon_pos++;
                     noPrefix.erase(0,colon_pos);
                  }
///////////////////////////////////////////this piece of code handles multiple endpoint
                  if ( siciEndpointSetInJdl ) {
                     try {
                        if ( (*lfn).find(silfnPrefix.c_str()) == 0 || (*lfn).find(lfnPrefix.c_str()) == 0 ) {
                           sici->listSEbyLFN(noPrefix.c_str(), resolved_sfn, requestAd, siciEndpoint);
                        }
                        else 
                           sici->listSEbyGUID(noPrefix.c_str(), resolved_sfn, requestAd, siciEndpoint);
                     }
                     catch (string& faultstring) {
                        edglog(warning) << faultstring << endl;
                     }
                  }
                  else if ( siciEndpointSetInIS ){
                     for ( vector<string>::const_iterator it=si_url_list.begin(); it != si_url_list.end(); it++){
                        try{
                           if ( (*lfn).find(silfnPrefix.c_str()) == 0 || (*lfn).find(lfnPrefix.c_str()) == 0 ) {
                              sici->listSEbyLFN(noPrefix.c_str(), resolved_sfn, requestAd, *it);
                           }
                           else 
                              sici->listSEbyGUID(noPrefix.c_str(), resolved_sfn, requestAd, *it);

                           if ( ! resolved_sfn.empty() ) break;
                        }
                        catch(string& faultstring) {
                            //
                            // Catch soap exceptions from the DataLocationInterface or StorageIndex Interface
                            // or any other exception due to failures in getting the proxy from the classad
                            edglog(warning) << faultstring << endl;
                        }
                     }
                  }
////////////////////////////////////////////end
               }
            } // if ( dli_to_be_used || si_to_be_used )
   
            if ( ((*lfn).find(lfnPrefix.c_str()) != 0) && ((*lfn).find(guidPrefix.c_str()) != 0) &&                                                                                                
                 ((*lfn).find(ldsPrefix.c_str()) != 0) && ((*lfn).find(queryPrefix.c_str()) != 0) &&
                 ((*lfn).find(silfnPrefix.c_str()) != 0) && ((*lfn).find(siguidPrefix.c_str()) != 0)   ) {
                      
               edglog(warning) << "wrong prefix: " << *lfn << endl;
            }
   
            if( !resolved_sfn.empty() ) {
               put_results_in_bi_data( *lfn, resolved_sfn, bid);
            }
            else {
               edglog(debug) << "No replica(s) found!" << endl;
            }
                                                                                                   
         }
         catch( std::exception& ex ) {
              edglog(warning) <<  ex.what() << endl;
         }
                                                                                                   
      } //for

   } //else dataReq


   if ( rlsInUse ) {
      destroyRls(replica);
      dlclose(rlsLibHandle);
   }

   if ( dli_siciInUse ) {
      destroyDli(dli);
      destroySici(sici);
      dlclose(dli_siciLibHandle);
   }

   edglog(debug) << "finishing retrieveSFNsInfo" << endl;
}


void brokerinfoGlueImpl::put_results_in_bi_data( const std::string& lfn,
                                                 const BrokerInfoData::SFN_container_type& resolved_sfn, 
                                                 BrokerInfoData& bid)
{

   edglog_fn(put_results_in_bi_data);
   bid.m_LFN2SFN_map[lfn] = resolved_sfn;
                                                                                                                    
   // static boost::regex  expression( "(.*):[\\s/]*([^\\s/]+)/.*" );
   static boost::regex  expression( "^\\s*([^:]*):[\\s/]*([^\\s:/]+)(:[0-9]+)?/.*" );
                                                                                                                    
   // Here we check that each returned SFN corresponds to a URL
   // of the general form:  protocol://SE-hostname/filepath
   // Only if this is the case, the SE is included into the bid.
   // If the return SFN is only a hostname, we check if the SEid
   // is registered in the Information System: if yes, fine too.
   //
   for(BrokerInfoData::SFN_container_type::const_iterator sfn = resolved_sfn.begin();
     sfn != resolved_sfn.end(); sfn++) {

      edglog(debug) <<*sfn << endl;

      try {

         boost::smatch pieces;
         std::string   SE_name;
                                                                                                                    
         if( boost::regex_match(*sfn, pieces, expression) ) {
            SE_name.assign(pieces[2].first, pieces[2].second);
            bid.m_involvedSEs.insert(SE_name);
         }
         else {
            // If the SFN doesn't match the regular expression, we assume
            // that only the SE name (SEid) has been returned. We do
            // NOT
            // check if the string really corresponds to a valid SE in the IS.
            //
            string SE = *sfn;
            string str = "://";
            int pos = SE.find (str,0);
            if (pos != string::npos){
               SE.erase(0, pos+str.length());
            }
            //if (validSE(SE) == 0) {

               bid.m_involvedSEs.insert(SE);

               //edglog(debug) << SE << ": " << "is a valid SE"<< endl;
            //}
            //else {
               //edglog(warning) << SE << ": " << "is *not* a valid SE"<< endl;
            //}
         }
      }
      catch( std::exception& ex ) {
         edglog(warning) << ex.what() << endl;
      }
   }

}


/**
 * Contact the Information Service (IS) and check if the given SE (SEid)
 * is registered in the IS. If yes, the SE is valid and 0 is returned, 
 * otherwise -1. 
 */
int brokerinfoGlueImpl::validSE( const std::string& SEid)
{
  int valid = -1; // default value: SE is not valid and not in IS

  edglog_fn(validSE);	
  const configuration::NSConfiguration* NSconf = configuration::Configuration::instance() -> ns();
  
  vector<string> attributes;
  
  attributes.push_back(GS_GLUESE_UNIQUEID);
  
  string filter;	
  filter = "(&(objectclass=GlueSE))";

  boost::scoped_ptr<ldif2classad::LDAPConnection> IIconnection;
  IIconnection.reset( new ldif2classad::LDAPSynchConnection(NSconf -> ii_dn(),
						     NSconf -> ii_contact(),
						     NSconf -> ii_port(),
						     NSconf -> ii_timeout()) );
    
  ldif2classad::LDAPQuery query(IIconnection.get(),filter,attributes);
    
  try {
    IIconnection -> open();
    query.execute();
      
    if( !query.tuples() -> empty() ) {
      try {
	ldif2classad::LDAPForwardIterator ldap_it( query.tuples() );   
	ldap_it.first();
	
	while( ldap_it.current() ) {
	  std::string currentSE;
	  (*ldap_it).EvaluateAttribute(GS_GLUESE_UNIQUEID, currentSE); 
	  if (currentSE == SEid) { valid = 0; }
	  ldap_it.next();
	}
      } 
      catch( ldif2classad::LDAPNoEntryEx&) {
	edglog(warning) << "InformationIndex search (no entry): " <<  query.what() << endl;
      }
    }
    else {
      edglog(warning) << "InformationIndex search (no tuples): " << query.what() << endl;  
    }
  } // try connection 
  catch( ldif2classad::QueryException& e) {
    edglog(warning) << e.what() << endl;
  }
  catch ( ldif2classad::ConnectionException& e) {
    edglog(warning) << e.what() << endl;
  }
  
  return valid;
} // validSE



/**
 * Contact the Information Service (IS) and return the URL (endpoint) of the
 * server the provides the DataLocationInterface(DLI).
 * If no service is found, "" is returned.
 */
void brokerinfoGlueImpl::get_catalog_url(const std::string& vo, const std::string& service_name,
                                         std::vector<std::string>& list)
{
//  string url = "";
                                                                                       
  edglog_fn(get_catalog_url);
  
  edglog(debug) << "Contacting IS for " << service_name<< " service. " << endl;       
//  const configuration::WMConfiguration* WMconf = configuration::Configuration::instance() -> wm();

   SDServiceList *sl=NULL;
   SDException ex;

   char **names = new (char*)[1];

   names[0] = new char[vo.length() +1];
   strcpy(names[0], vo.c_str());

   SDVOList vos = {1, names};

   sl = SD_listServices( service_name.c_str(), NULL, &vos, &ex);
   if (sl != NULL) { 

      if ( sl->numServices > 0 )  {

         for(int k=0; k < sl->numServices; k++) {
              //edglog(debug) <<"EndPoint["<<k<<"]: "<< sl->services[k]->endpoint  << endl;
              list.push_back( strdup(sl->services[k]->endpoint) ) ;
         }
      }
      else {
         edglog(warning) << "No endpoints found" << endl;
      }

      SD_freeServiceList(sl);

   }
   else {
      if (ex.status == SDStatus_SUCCESS) {
         edglog(warning) << "No such services" << endl;
      }
      else {
         edglog(error) <<"Call failed: " <<  ex.reason << endl;
         SD_freeException(&ex);
      }
   }

   delete []names[0];
   delete []names;

}


/*
 * Check the configuration file of the Networkserver if RLS is used for
 * a certain VO. In case the RLS is used, 0 is returned, Otherwise -1.
 */
int brokerinfoGlueImpl::checkRlsUsage(const std::string& vo)
{
  const configuration::NSConfiguration* NSconf = configuration::Configuration::instance() -> ns();

  std::vector<std::string> rls = NSconf->rlscatalog();

  for (unsigned int i=0; i < rls.size(); i++) {
    if (rls[i] == vo) { return 0;}
  }

  return -1;
}// checkRlsUsage



} // namespace brokerinfo
} // namespace wms
} // namespace glite

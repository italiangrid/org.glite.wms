// File: brokerinfoGlueImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

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

#include "glite/wms/common/utilities/ii_attr_utils.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"

#include "glite/wmsutils/exception/Exception.h"

#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"

#include "glite/wms/rls/ReplicaServiceReal.h"

#include "glue_attributes.h"

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

      void brokerinfoGlueImpl::retrieveCloseSEsInfo(const BrokerInfoData::CEid_type& CEid, 
                                                    BrokerInfoData& bid, std::vector<std::string>* additional_attrs)
      { 
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
        try {    
    
          // The InputData field is used to specifu a list of LFN or GUID or LCN
    
          boost::scoped_ptr<rls::ReplicaService> replica
            ( new rls::ReplicaServiceReal(requestad::get_virtual_organisation(requestAd)) ); 
    
          BrokerInfoData::LFN_container_type input_data;
          requestad::get_input_data(requestAd, input_data);
    
          for(BrokerInfoData::LFN_container_type::const_iterator lfn = input_data.begin();
              lfn != input_data.end(); lfn++) {
      
            try {
        
              edglog(debug) << "Listing replica(s) for " << *lfn << endl;
              BrokerInfoData::SFN_container_type resolved_sfn( replica->listReplica(*lfn) );
  
              if( !resolved_sfn.empty() ) {
    
                bid.m_LFN2SFN_map[*lfn] = resolved_sfn;

                // static boost::regex  expression( "(.*):[\\s/]*([^\\s/]+)/.*" );
                static boost::regex  expression( "^\\s*([^:]*):[\\s/]*([^\\s:/]+)(:[0-9]+)?/.*" );

                for(BrokerInfoData::SFN_container_type::const_iterator sfn = resolved_sfn.begin();
                    sfn != resolved_sfn.end(); sfn++) {
                  edglog(debug) << *sfn << endl;
                  try {
        
                    boost::smatch pieces;
                    std::string   SE_name;
        
                    if( boost::regex_match(*sfn, pieces, expression) ) {
    
                      SE_name.assign(pieces[2].first, pieces[2].second);
                      bid.m_involvedSEs.insert(SE_name);
                    }
                  }
                  catch( std::exception& ex ) {
                    edglog(warning) << ex.what() << endl;
                  }
                }
              }
              else {
                edglog(debug) << "No replica(s) found!" << endl;
              }
            } 
            catch( std::exception& ex ) {
              edglog(warning) << ex.what() << endl;
            }
          }
        } 
        catch( std::exception& ex ) {
          edglog(warning) << ex.what() << endl;
        }
      }

    } // namespace brokerinfo
  } // namespace wms
} // namespace glite

/*
 * File: BrokerInfoData.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <algorithm>
#include <classad_distribution.h>
#include <boost/tuple/tuple.hpp>
#include <boost/regex.hpp>
#include "glite/wms/brokerinfo/BrokerInfoData.h"
#include "glue_attributes.h"

using namespace std;

namespace glite {
namespace wms {
namespace brokerinfo {

  BrokerInfoData::SE_container_type BrokerInfoData::getCompatibleCloseSEs(const vector<string>& data_access_protocols) const
  {
    SE_container_type compatibleCloseSEs;
    for (CloseSEInfo_const_iterator close_se = m_CloseSEInfo_map.begin();
        close_se != m_CloseSEInfo_map.end(); close_se++) 
      {
      
      SE2Protocol_const_iterator it;
      if ((it = m_SE2Protocol_map.find(close_se->first)) !=  m_SE2Protocol_map.end()) {
        
        const protocol_container_type& supported_protocols(it->second);
        for (protocol_container_type::const_iterator p = supported_protocols.begin();
             p != supported_protocols.end(); p++) 
          {
            vector<string>::const_iterator protocol;
            if ((protocol = std::find(data_access_protocols.begin(),
                                      data_access_protocols.end(), 
                                      p->first)) != data_access_protocols.end() ) { 
                                
              if (*protocol == "file") {
                
                CloseSEInfo_const_iterator close_se_info = m_CloseSEInfo_map.find(close_se->first);
                CloseSEInfo_type CloseSEInfoAd = close_se_info->second;
                bool is_mount_defined = CloseSEInfoAd->Lookup("GlueCESEBindCEAccesspoint") != 0;
                if (is_mount_defined) {
                                        
                  compatibleCloseSEs.insert(close_se->first);
                  break;
                }
              }
              else {
                compatibleCloseSEs.insert(close_se->first);
                break;
              }
            }
          }
      }
      }
    return compatibleCloseSEs;
  }       
  
  BrokerInfoData::LFN_container_type BrokerInfoData::getProvidedLFNs(const SE_container_type& compatilbleCloseSEs) const
  {
    //static boost::regex  expression( "(.+):[\\s/]*([^\\s/]+)/.*" );
    static boost::regex  expression( "^\\s*([^:]*):[\\s/]*([^\\s:/]+)(:[0-9]+)?/.*" );
    std::set<LFN> providedLFNs;
    for (LFN2SFN_const_iterator it = m_LFN2SFN_map.begin(); it != m_LFN2SFN_map.end(); it++) 
      {
        const SFN_container_type& SFNs( it->second );
        for (SFN_container_type::const_iterator sfn = SFNs.begin(); sfn != SFNs.end(); sfn++) 
          { 
            try {
              boost::smatch pieces;
              std::string se;
              if (boost::regex_match(*sfn, pieces, expression)) {
                
                se.assign(pieces[2].first, pieces[2].second);
                if (compatilbleCloseSEs.find(se) != compatilbleCloseSEs.end()) {
                  
                  providedLFNs.insert(it->first);
                  break;
                }
              }
            }
            catch( ... ) 
              {
              }
          }       
      }
    LFN_container_type result(providedLFNs.begin(), providedLFNs.end());
    return result;
  }
  
  classad::ExprList* BrokerInfoData::CloseStorageElements() const
  {
    std::vector<classad::ExprTree*>  CloseSE_exprs;
    
    for (CloseSEInfo_const_iterator se_info = m_CloseSEInfo_map.begin();
         se_info != m_CloseSEInfo_map.end(); se_info++)
      {
        CloseSEInfo_type info;
        BrokerInfoData::SE_name     name;
        boost::tie(name, info) = *se_info;
        classad::ClassAd* SEad = static_cast<classad::ClassAd*>(info->Copy());
        SEad->InsertAttr("name", name);
        
        // Inserts an alias for the mount point
        classad::AttributeReference* mount_ref
          (classad::AttributeReference::MakeAttributeReference(0, GS_GLUECE_SEBIND_ACCESSPOINT));
        classad::AttributeReference* free_ref
          (classad::AttributeReference::MakeAttributeReference(0, "GlueSAStateAvailableSpace"));
        
        SEad->Insert("mount",     mount_ref);
        SEad->Insert("freespace", free_ref);    
        CloseSE_exprs.push_back( SEad );
      }
    return classad::ExprList::MakeExprList(CloseSE_exprs);
  }
  
  classad::ClassAd* BrokerInfoData::asClassAd() const 
  {
    /*
      ComputingElement = [
      name = ResourceId;
      CloseStorageElements = {
      [ name = CloseSE1; mount = mountpoint1 ],
      [ name = CloseSE2; mount = mountpoint2 ],
      ...
      [ name = CloseSEn; mount = mountpointn ]
      };
      ];
    */
    classad::ClassAd ComputingElementAd;
    ComputingElementAd.InsertAttr("name", m_referredCEid);
    ComputingElementAd.Insert("CloseStorageElements", CloseStorageElements());
    /*
      InputFNs = {
      [ name = FN1; SFNs = {SFN1,1, SFN1,2, ..., SFN1,m1} ],
      [ name = FN2; SFNs = {SFN2,1, SFN2,2, ..., SFN2,m2} ],
      ...
      [ name = FNn; SFNs = {SFNn,1, SFNn,2, ..., SFNn,mn} ]
      };
    */
    std::vector<classad::ExprTree*>  InputFN_exprs;
    for( LFN2SFN_const_iterator fn = m_LFN2SFN_map.begin();
         fn != m_LFN2SFN_map.end(); fn++ ) 
      {
        classad::ClassAd        fn_ad;
        std::string             fn_name;
        SFN_container_type      SFN_container;
        
        boost::tie(fn_name, SFN_container) = *fn;
        fn_ad.InsertAttr("name", fn_name);
        std::vector<classad::ExprTree*> SFN_exprs;
        for(SFN_container_type::const_iterator sfn = SFN_container.begin();
            sfn != SFN_container.end(); sfn++) 
          {
            classad::Value v;
            v.SetStringValue(*sfn);
            SFN_exprs.push_back(classad::Literal::MakeLiteral(v));
          }
        fn_ad.Insert("SFNs", classad::ExprList::MakeExprList(SFN_exprs));
        InputFN_exprs.push_back( fn_ad.Copy() );
      }
    /*
      StorageElements = {
      [ name = SE1; protocols = { [ name = proto1,1; port = port1,1 ],
      [ name = proto1,2; port = port1,2 ],
      ...
      [ name = proto1,p1; port = port1,p1 ] }
      ],
      ...
      [ name = SEq; protocols = { [ name = protoq,1; port = portq,1 ],
      [ name = protoq,2; port = portq,2 ],
      ...
      [ name = protoq,pq; port = portq,pq ] }
      ]
      };
    */
    std::vector<classad::ExprTree*>  StorageElement_exprs;
    for( SE2Protocol_const_iterator s2p = m_SE2Protocol_map.begin();
         s2p != m_SE2Protocol_map.end(); s2p++ )
      {
        classad::ClassAd        s2p_ad;
        std::string             se_name;
        protocol_container_type protocol_container;
        boost::tie(se_name, protocol_container) = *s2p;
        s2p_ad.InsertAttr("name", se_name);
        std::vector<classad::ExprTree*> protocol_exprs;
        for(protocol_container_type::const_iterator p = protocol_container.begin();
            p != protocol_container.end(); p++) 
          {
            std::string protocol_name;
            int         protocol_port;
            boost::tie(protocol_name, protocol_port) = *p;
            classad::ClassAd protocol_ad;
            protocol_ad.InsertAttr("name", protocol_name);
            if(protocol_port)  protocol_ad.InsertAttr("port", protocol_port);
            protocol_exprs.push_back(protocol_ad.Copy());
          }
        s2p_ad.Insert("protocols", classad::ExprList::MakeExprList(protocol_exprs));
        StorageElement_exprs.push_back(s2p_ad.Copy());
      }               
    
    classad::ClassAd biAd;
    biAd.Insert("ComputingElement", ComputingElementAd.Copy());
    biAd.Insert("InputFNs",         classad::ExprList::MakeExprList(InputFN_exprs));
    biAd.Insert("StorageElements",  classad::ExprList::MakeExprList(StorageElement_exprs));
    biAd.InsertAttr("VirtualOrganisation", m_referredVO);
    return static_cast<classad::ClassAd*>(biAd.Copy());
  }
  
} // namespace brokerinfo
} // namespace wms
} // namespace glite

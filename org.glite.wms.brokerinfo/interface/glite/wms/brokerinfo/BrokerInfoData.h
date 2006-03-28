/*
 * File: BrokerInfoData.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef _GLITE_WMS_BROKERINFO_BROKERINFODATA_H_
#define _GLITE_WMS_BROKERINFO_BROKERINFODATA_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>
#include <set>

namespace classad
{
  class ClassAd;
  class ExprList;	
}

namespace glite {
namespace wms {
namespace brokerinfo {

class BrokerInfoData : boost::noncopyable
{
public:
 typedef std::string			    VO_name_type;
 typedef std::string                        CEid_type; 
 typedef std::string                        SE_name;
 typedef std::set<SE_name>                  SE_container_type;
 typedef SE_container_type::const_iterator  SE_const_iterator;
 typedef SE_const_iterator                  SE_iterator;
  
 typedef std::string                        SFN;
 typedef std::string                        LFN;
 typedef std::vector<SFN>             	    SFN_container_type;
 typedef std::vector<LFN>		    LFN_container_type;
 typedef std::map<LFN, SFN_container_type>  LFN2SFN_map_type;
 typedef LFN2SFN_map_type::const_iterator   LFN2SFN_const_iterator;
 typedef LFN2SFN_const_iterator             LFN2SFN_iterator;
  
 typedef std::string                                protocol_name;
 typedef int                                        protocol_port;
 typedef std::pair<protocol_name, protocol_port>    protocol_type;
 typedef std::vector<protocol_type>                 protocol_container_type;
 typedef std::map<SE_name, protocol_container_type> SE2Protocol_map_type;
 typedef SE2Protocol_map_type::const_iterator       SE2Protocol_const_iterator;
 typedef SE2Protocol_const_iterator                 SE2Protocol_iterator;
  
 typedef boost::shared_ptr<classad::ClassAd>    CloseSEInfo_type;
 typedef std::map<SE_name, CloseSEInfo_type>    CloseSEInfo_map_type;
 typedef CloseSEInfo_map_type::const_iterator   CloseSEInfo_const_iterator;
 typedef CloseSEInfo_const_iterator             CloseSEInfo_iterator;

  BrokerInfoData() {}
  ~BrokerInfoData() {} 
  std::pair<SE_const_iterator, SE_const_iterator> involvedSEs() const
  { 
    return std::make_pair(m_involvedSEs.begin(), m_involvedSEs.end()); 
  }
  std::pair<LFN2SFN_const_iterator, LFN2SFN_const_iterator> LFN2SFN_map() const 
  { 
    return std::make_pair(m_LFN2SFN_map.begin(),m_LFN2SFN_map.end()); 
  }
  std::pair<SE2Protocol_const_iterator,  SE2Protocol_const_iterator> SE2Protocol_map() const
  {
    return std::make_pair(m_SE2Protocol_map.begin(), m_SE2Protocol_map.end());
  }
  std::pair<CloseSEInfo_const_iterator, CloseSEInfo_const_iterator> CloseSEInfo_map() const
  {
    return std::make_pair(m_CloseSEInfo_map.begin(), m_CloseSEInfo_map.end());
  }
  
  classad::ClassAd*  asClassAd() const;
  classad::ExprList* CloseStorageElements() const; 
  SE_container_type getCompatibleCloseSEs(const std::vector<std::string>& data_access_protocols) const;
  LFN_container_type getProvidedLFNs(const SE_container_type& compatilbleCloseSEs) const;

private:
  friend class brokerinfoISMImpl; 
  /* InputData dependent */
  SE_container_type      m_involvedSEs;     /**< storage elements referenced by SFN. */
  LFN2SFN_map_type       m_LFN2SFN_map;     /**< logical-file-names to physical-file-names mapping. */
  SE2Protocol_map_type   m_SE2Protocol_map; /**< protocols and ports for a given SE. */
  
  /* Selected CE dependet */
  CloseSEInfo_map_type     m_CloseSEInfo_map; /**< close sorage elements and information (i.e. mount point). */
  CEid_type	           m_referredCEid;
  VO_name_type		   m_referredVO;
};

}; // brokerinfo
}; // wms
}; // glite

#endif

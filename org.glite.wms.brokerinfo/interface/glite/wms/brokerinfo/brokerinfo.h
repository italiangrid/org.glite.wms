/*
 * File: BrokerInfo.h
 * Author: Monforte Salvatore
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef _GLITE_WMS_BROKERINFO_BROKERINFO_H_
#define _GLITE_WMS_BROKERINFO_BROKERINFO_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include "glite/wms/brokerinfo/BrokerInfoData.h"

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace brokerinfo {

template <class implementation> class BrokerInfo;

class BrokerInfoImpl : boost::noncopyable
{
public:
  virtual void retrieveCloseSAsInfo (const BrokerInfoData::VO_name_type&, BrokerInfoData&) = 0;
  virtual void retrieveCloseSEsInfo (const BrokerInfoData::CEid_type&, BrokerInfoData&)   = 0;
  virtual void retrieveSEsInfo      (const classad::ClassAd& requestAd, BrokerInfoData&)  = 0;
  virtual void retrieveSFNsInfo     (const classad::ClassAd& requestAd, BrokerInfoData&)  = 0;
  virtual ~BrokerInfoImpl() {}
};

template<class implementation>
class BrokerInfo : boost::noncopyable
{ 
public:
 
  BrokerInfo() : m_data( new BrokerInfoData ), m_impl( new implementation ) {} 
  ~BrokerInfo() {}
  
  void retrieveCloseSAsInfo (const BrokerInfoData::VO_name_type& vo)
  {
    m_impl -> retrieveCloseSAsInfo(vo, *this -> m_data);	  
  }
  void retrieveCloseSEsInfo (const BrokerInfoData::CEid_type& CEid) 
  { 
    m_impl -> retrieveCloseSEsInfo(CEid, *this -> m_data); 
  }
  
  void retrieveSEsInfo (const classad::ClassAd& requestAd) 
  { 
    m_impl -> retrieveSEsInfo(requestAd, *this -> m_data); 
  }
  
  void retrieveSFNsInfo     (const classad::ClassAd& requestAd) 
  { 
    m_impl -> retrieveSFNsInfo(requestAd, *this -> m_data); 
  }
  
  classad::ClassAd* asClassAd() const
  {
    return m_data -> asClassAd();		  	
  }
  const BrokerInfoData* operator->() const { return m_data.get(); }
private:
  boost::scoped_ptr<BrokerInfoData> m_data;
  boost::scoped_ptr<BrokerInfoImpl> m_impl;
};

}; // namespace brokerinfo
}; // namespace wms
}; // namespace glite

#endif

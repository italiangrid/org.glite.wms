// File: brokerinfoGlueImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef _GLITE_WMS_BROKERINFO_BROKERINFOGLUEIMPL_H
#define _GLITE_WMS_BROKERINFO_BROKERINFOGLUEIMPL_H

#include "glite/wms/brokerinfo/brokerinfo.h"

namespace glite {
namespace wms {
namespace brokerinfo {
	
class brokerinfoGlueImpl : public BrokerInfoImpl
{
 public:
  brokerinfoGlueImpl();
  ~brokerinfoGlueImpl();
  void retrieveCloseSAsInfo(const BrokerInfoData::VO_name_type& VO, BrokerInfoData& bid, std::vector<std::string>* = 0);
  void retrieveCloseSEsInfo(const BrokerInfoData::CEid_type& CEId, BrokerInfoData& bid, std::vector<std::string>* = 0);
  void retrieveSEsInfo     (const classad::ClassAd& requestAd, BrokerInfoData& bid);
  void retrieveSFNsInfo    (const classad::ClassAd& requestAd, BrokerInfoData& bid);
};

} // namespace brokerinfo
} // namespace wms
} // namespace glite

#endif

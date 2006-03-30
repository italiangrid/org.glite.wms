// File: brokerinfoISMImpl.h
// Author: Salvatore Monforte
//
// Revision history
// 12-11-2004 new catolog interfaces added. Author: Enzo Martelli <enzo.martelli@mi.infn.it>
//
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_BROKERINFO_BROKERINFOISMIMPL_H
#define GLITE_WMS_BROKERINFO_BROKERINFOISMIMPL_H

#include "glite/wms/brokerinfo/brokerinfo.h"

namespace glite {
namespace wms {
namespace brokerinfo {
	
class brokerinfoISMImpl : public BrokerInfoImpl
{
 public:
  void retrieveCloseSAsInfo(const BrokerInfoData::VO_name_type& VO, BrokerInfoData& bid);
  void retrieveCloseSEsInfo(const BrokerInfoData::CEid_type& CEId, BrokerInfoData& bid);
  void retrieveSEsInfo     (const classad::ClassAd& requestAd, BrokerInfoData& bid);
  void retrieveSFNsInfo    (const classad::ClassAd& requestAd, BrokerInfoData& bid);
private:
  /**
   * Contact the Information Service (IS) and check if the given SE (SEid)
   * is registered in the IS. If yes, the SE is valid and 0 is returned, 
   * otherwise -1. 
   */
  int  validSE(const std::string& SEid);

  /**
   * Queries the IS for the catalog url throught ServiceDiscovery
   */
  void get_catalog_url(const std::string& vo, const std::string& service_name, 
                       std::vector<std::string>& url_list);

  /*
   * Check the configuration file of the Networkserver if RLS is used for
   * a certain VO. In case the RLS is used, 0 is returned, Otherwise -1.
   */
  int checkRlsUsage(const std::string& vo);

  /*
   * Put the result of the matching of a given logical file name in the BrokerInfoData
   */
  void put_results_in_bi_data( const std::string& lfn, 
                               const BrokerInfoData::SFN_container_type& resolved_sfn, 
                               BrokerInfoData& bid);
};

} // namespace brokerinfo
} // namespace wms
} // namespace glite

#endif

// File: brokerinfoGlueImpl.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
//
// Revision history
// 12-11-2004 new catolog interfaces added. Author: Enzo Martelli <enzo.martelli@mi.infn.it>
//
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
  bool retrieveCloseSEsInfoFromISM(const BrokerInfoData::CEid_type& CEId, BrokerInfoData& bid);
  void retrieveCloseSAsInfo(const BrokerInfoData::VO_name_type& VO, BrokerInfoData& bid, std::vector<std::string>* = 0);
  void retrieveCloseSEsInfo(const BrokerInfoData::CEid_type& CEId, BrokerInfoData& bid, std::vector<std::string>* = 0);
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
   * Contact the Information Service (IS) and return the URL (endpoint) of the
   * server the provides the StorageIndex Catalog(SI).
   * If no service is found, "" is returned.
   */
  std::string getSICIurl(const std::string& vo);

  /**
   * Contact the Information Service (IS) and return the URL (endpoint) of the
   * server the provides the DataLocationInterface(DLI).
   * If no service is found, "" is returned.
   */
  std::string getDLIurl(const std::string& vo);

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

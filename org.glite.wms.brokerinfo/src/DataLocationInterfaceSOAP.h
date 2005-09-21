//
//  File :     DataLocationInterfaceSOAP.h
//
//
//  Author :   Enzo Martelli ($Author$)
//  e-mail :   "enzo.martelli@mi.infn.it"
//
//  Revision history :
//  02-12-2004 Original release
//
//  Description:
//  Wraps the DLI soap client API
//
//
//  Copyright (c) 2004 Istituto Nazionale di Fisica Nucleare (INFN).
//  All rights reserved.
//  See http://grid.infn.it/grid/license.html for license details.
//

#ifndef GLITE_WMS_BROKERINFO_DLI_DATALOCATIONINTERFACESOAP_H
#define GLITE_WMS_BROKERINFO_DLI_DATALOCATIONINTERFACESOAP_H

#include <string>
#include <vector>

#include <stdsoap2.h>


namespace glite {
namespace wms {
namespace brokerinfo {
namespace dli {


class DataLocationInterfaceSOAP {
public:
  /**
   * Constructor for DataLocationInterface
   *
   * @param endpoint SOAP endpoint (URL) of the remote catalogue
   *                 example: http://localhost:8085/
   */
  DataLocationInterfaceSOAP(std::string endpoint);

  /**
   * Constructor for DataLocationInterface
   *
   * @param endpoint SOAP endpoint (URL) of the remote catalogue
   *                 example: http://localhost:8085/
   * @param timeout  connection and IO timeout
   */
  DataLocationInterfaceSOAP(std::string endpoint,
                            int timeout);

  /**
   * List all replicas of a given InputDataType. A replica needs to contain
   * a valid SEId that is registered with the Information Service.
   *
   * @param inputDataType Defines one of the following InputDataTypes:
   *                      lfn   ... LogicalFileName
   *                      guid  ... GUID Global Unique Idenifier
   *                      lds   ... LogicalDataSet
   *                      query ... generic query to the catalogue
   *        Further InputDataTypes can be extended in the future but need to
   *        be understood by the remote catalogue. 
   *        Note that a catalogue does not need to implement all of the four
   *        InputDataTypes but is free to support any subset.
   * @param inputData     Actutual InputData variable
   *
   * @returns a vector of URLs that represent the locations of where
   *          the InputData is located. The URL can either be a full URL
   *          of the form    protocol://hostname/pathname
   *          or             hostname
   *          where hostname is a registered SEId.
   */
  virtual std::vector<std::string> listReplicas(std::string inputDataType,
						std::string inputData);

  /**
   * Destructor: clean up the SOAP environment
   */
  virtual ~DataLocationInterfaceSOAP();

private:
  DataLocationInterfaceSOAP(){};

  struct soap m_soap;     // gSOAP structure for message exchange
  std::string m_endpoint; // endpoint (URL) of the data catalogue to contact
};

typedef DataLocationInterfaceSOAP* create_t(const std::string&);

typedef DataLocationInterfaceSOAP* create_t_with_timeout(const std::string&,
                                                         int timeout);
typedef void destroy_t(DataLocationInterfaceSOAP*);


} // namespace dli
} // namespace brokerinfo
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_BROKERINFO_DLI_DATALOCATIONINTERFACESOAP_H

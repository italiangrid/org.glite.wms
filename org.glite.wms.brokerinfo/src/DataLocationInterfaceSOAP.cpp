//
//  File :     STorageInedxCatalogInterface.h
//
//
//  Author :   Heinz Stockinger 
//  e-mail :   "heinz.stockinger@cern.ch"
//
//  Revision history :
//  Original release has been implemented for LCG 
//  02-12-2004 Integration in glite
//
//  Description:
//  Wraps the DLI soap client API
//
//
//  Copyright (c) 2004 Istituto Nazionale di Fisica Nucleare (INFN).
//  All rights reserved.
//  See http://grid.infn.it/grid/license.html for license details.
//

#include <DataLocationInterfaceSOAP.h>

#include <soapH.h>
#include <DataLocationInterface.nsmap>


namespace dli = glite::wms::brokerinfo::dli;

/*****************************************************************************/
/*                          c o n s t r u c t o r                            */
/*****************************************************************************/

/**
 * Constructor for DataLocationInterface
 *
 * @param endpoint SOAP endpoint (URL) of the remote catalogue
 *                 example: http://localhost:8085/
 */
dli::DataLocationInterfaceSOAP::DataLocationInterfaceSOAP(std::string endpoint)
{
  // Initialise SOAP
  //
  soap_init1(&m_soap, SOAP_IO_CHUNK);

  m_endpoint = endpoint;
} // Constructor

dli::DataLocationInterfaceSOAP::DataLocationInterfaceSOAP(std::string endpoint,
                                                          int timeout)
{
  // Initialise SOAP
  //
  soap_init1(&m_soap, SOAP_IO_CHUNK);

  // connection timeout 
  m_soap.connect_timeout = timeout;
  // IO timeouts
  m_soap.send_timeout = timeout;
  m_soap.recv_timeout = timeout;
                                                                                                     
  m_endpoint = endpoint;
} // Constructor



/*****************************************************************************/
/*                          l i s t R e p l i c a s                          */
/*****************************************************************************/

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
std::vector<std::string> 
dli::DataLocationInterfaceSOAP::listReplicas(std::string inputDataType,
					     std::string inputData)
{
  std::vector<std::string> urlVector;

  URLArray urlList;
  
  // Call listReplicas and handle potential SOAP Faults
  //
  if (soap_call_ns__listReplicas(&m_soap, m_endpoint.c_str(), "",
				 inputDataType, inputData, &urlList)) {
    // In case of a SOAP Fault, throw an error string that the server sent
    //
      if ( m_soap.fault != NULL ) {
         throw m_soap.fault->faultstring;
      }
      else {
          throw "Error in soap request towards DLI Catalog. Unknown fault reason, soap.fault=NULL";
      }

    
  }

  // Convert the URLArray into a vector that we then return to the caller
  //
  for (int i=0; i < urlList.__size; i++ ) {
    urlVector.push_back(urlList[i]);
  }

  return urlVector;
} // listReplicas


/*****************************************************************************/
/*                            d e s t r u c t o r                            */
/*****************************************************************************/

dli::DataLocationInterfaceSOAP::~DataLocationInterfaceSOAP()
{
  // Finalise SOAP
  //
  soap_destroy(&m_soap);
  soap_end(&m_soap);
  soap_done(&m_soap);
} // destructor

/*****************************************************************************/
/*  Class factories that can be used for a plug-in                           */
/*****************************************************************************/

extern "C" dli::DataLocationInterfaceSOAP* create(const std::string& endpoint)
{
  return new dli::DataLocationInterfaceSOAP(endpoint);
}

extern "C" dli::DataLocationInterfaceSOAP* create_with_timeout(const std::string& endpoint,
                                                               int timeout)
{
  return new dli::DataLocationInterfaceSOAP(endpoint, timeout);
}


extern "C" void destroy(dli::DataLocationInterfaceSOAP* p) {
  delete p;
}


/***************************************************************************/
/*                         URLArray                                        */
/***************************************************************************/

/**
 * URLArray: class to implement a string array in gSOAP. The class is
 *           defined in DataLocationInterface.h
 */

URLArray::URLArray()
{ __ptr = NULL;
  __size = 0;
  soap = NULL;
} // URLArray constructor

std::string& URLArray::operator[](int i) const 
{
  assert(__ptr && i >= 0 && i < __size);
  return __ptr[i];
} // URLArray::operator

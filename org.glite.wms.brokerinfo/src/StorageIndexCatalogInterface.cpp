//
//  File :     StorageInedxCatalogInterface.cpp
//
//
//  Author :   Enzo Martelli ($Author$)
//  e-mail :   "enzo.martelli@mi.infn.it"
//
//  Revision history :
//  29-11-2004 Original release
//
//  Description:
//  Wraps the Storage Index Catalog client API
//
//
//  Copyright (c) 2004 Istituto Nazionale di Fisica Nucleare (INFN).
//  All rights reserved.
//  See http://grid.infn.it/grid/license.html for license details.
//

 

/*
 * Include CGSI GSOAP plugin
 */
#include <cgsi_plugin.h>


#include <string.h>
#include <glite/data/catalog/storageindex/c/storageindexH.h>
#include <glite/data/catalog/storageindex/c/storageindex.nsmap>

#include <StorageIndexCatalogInterface.h>



glite::wms::sici::StorageIndexCatalogInterface::StorageIndexCatalogInterface( const std::string &endpoint)
{
   // Initialise SOAP
   //
   soap_init(&m_soap);
   m_soap.namespaces = storageindex_namespaces;

   m_endpoint = endpoint;
}

void glite::wms::sici::StorageIndexCatalogInterface::listSEbyGUID( const std::string &guid, std::vector<std::string> & list) 
{

   struct storageindex__listSEbyGUIDResponse output;
   

   char  *guidCopy = new char[guid.length() + 1];
   
   strcpy(guidCopy, guid.c_str());   
   
   if (soap_call_storageindex__listSEbyGUID(&m_soap, m_endpoint.c_str(), NULL, guidCopy, &output) ) {
      delete [] guidCopy;
      throw m_soap.fault->faultstring;
   }

   for (int i=0; i < output._listSEbyGUIDReturn->__size; i++ ) {
      std::string str = output._listSEbyGUIDReturn->__ptr[i];
      list.push_back(str);
   }

   delete [] guidCopy;

}

void 
glite::wms::sici::StorageIndexCatalogInterface::listSEbyLFN( const std::string &lfn, std::vector<std::string> & list) 
{
   struct storageindex__listSEbyLFNResponse output;
				                    
   char  *lfnCopy = new char[lfn.length() + 1];
   
   strcpy(lfnCopy, lfn.c_str()); 

   if( soap_call_storageindex__listSEbyLFN(&m_soap, m_endpoint.c_str(), NULL, lfnCopy, &output) ) {
      delete [] lfnCopy;
      throw m_soap.fault->faultstring;
   }
                                                                                                                 
   for (int i=0; i < output._listSEbyLFNReturn->__size; i++ ) {
      std::string str = output._listSEbyLFNReturn->__ptr[i];
      list.push_back(str);
   }

   delete [] lfnCopy;

}
    


glite::wms::sici::StorageIndexCatalogInterface::~StorageIndexCatalogInterface()
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
extern "C" glite::wms::sici::StorageIndexCatalogInterface* create(const std::string& endpoint)
{
  return new glite::wms::sici::StorageIndexCatalogInterface(endpoint);
}
 
extern "C" void destroy(glite::wms::sici::StorageIndexCatalogInterface* p) {
  delete p;
}


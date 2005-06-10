//
//  File :     StorageInedxCatalogInterface.cpp
//
//
//  Author :   Enzo Martelli ($Author$)
//  e-mail :   "enzo.martelli@mi.infn.it"
//
//  Revision history :
//  02-12-2004 Original release
//
//  Description:
//  Wraps the Storage Index Catalog soap client API
//
//
//  Copyright (c) 2004 Istituto Nazionale di Fisica Nucleare (INFN).
//  All rights reserved.
//  See http://grid.infn.it/grid/license.html for license details.
//


#include <string.h>
#include <malloc.h>
#include "glite/data/catalog/storageindex/c/storageindexH.h"
#include "glite/data/catalog/storageindex/c/storageindex.nsmap"

extern "C" {
//   #include "cgsi_plugin.h"
   #include "glite/security/glite_gsplugin.h"
}

#include "StorageIndexCatalogInterface.h"
#include <classad_distribution.h>
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include <string>

using namespace glite::wms;


glite::wms::brokerinfo::sici::StorageIndexCatalogInterface::StorageIndexCatalogInterface( const std::string &endpoint)
{
   // Initialise SOAP
   //
   soap_init(&m_soap);
   m_soap.namespaces = storageindex_namespaces;
                                                                                                     
   m_endpoint = endpoint;

   ctx = NULL;
}

glite::wms::brokerinfo::sici::StorageIndexCatalogInterface::StorageIndexCatalogInterface( const std::string &endpoint, int timeout)
{
   // Initialise SOAP
   //
   soap_init(&m_soap);
   m_soap.namespaces = storageindex_namespaces;
  
   // connection timeout 
   m_soap.connect_timeout = timeout;

   // IO timeouts
   m_soap.send_timeout = timeout;
   m_soap.recv_timeout = timeout;

   m_endpoint = endpoint;

   ctx = NULL;
}

void glite::wms::brokerinfo::sici::StorageIndexCatalogInterface::listSEbyGUID( const std::string &guid, std::vector<std::string> & list, const classad::ClassAd & classad) 
{

   //////////////...for using secure endpoint
   bool proxyInJdl = true;
   std::string proxy;
   try {
      proxy =  jdl::get_x509_user_proxy(classad);
   }
   catch(...) {
      proxyInJdl = false;
   }

   if( 0 == strncasecmp(m_endpoint.c_str(),"https://",8) ){

      if ( proxyInJdl ){
         if ( !ctx ){
            if ( glite_gsplugin_init_context(&ctx) ) {
               throw "gsplugin_init_context FAILED"; 
            }
         }
         //glite_gsplugin_free_context will free *both* cert and key
         ctx->cert_filename = strdup(proxy.c_str());
         ctx->key_filename = strdup(proxy.c_str());
      }
      else throw "UserProxy not specified in the ClassAd";

      if ( soap_register_plugin_arg(&m_soap, glite_gsplugin, ctx) ) {
         throw "soap_register_plugin_arg FAILED";
      }

   }


////////////////////////////////////////////////////////
   struct storageindex__listSEbyGUIDResponse output;
   
   char  *guidCopy = new char[guid.length() + 1];
   
   strcpy(guidCopy, guid.c_str());   
   
   if (soap_call_storageindex__listSEbyGUID(&m_soap, m_endpoint.c_str(), NULL, guidCopy, &output) ) {
      delete [] guidCopy;
      if ( m_soap.fault != NULL ) {

          const char** details_ptr = soap_faultdetail( & m_soap );
          std::string detail;
          if (details_ptr[0] != NULL ) detail = details_ptr[0];
             else detail = "unknown";
          std::string fault_code;
          if ( m_soap.fault->faultcode != NULL ) fault_code = m_soap.fault->faultcode;
             else fault_code = "unknown";
          std::string fault_string;
          if ( m_soap.fault->faultstring != NULL ) fault_string =  m_soap.fault->faultstring;
             else fault_string = "unknown";
          std::string SOAP_FAULTCODE = "SOAP_FAULTCODE: ";
          std::string SOAP_FAULTSTRING = "SOAP_FAULTSTRING: ";
          std::string SOAP_FAULT_DETAIL = "SOAP_FAULT_DETAIL: ";
          std::string new_line = "\n";
 
          std::string ex = SOAP_FAULTCODE + fault_code + new_line +
                           SOAP_FAULTSTRING +  fault_string + new_line +
                           SOAP_FAULT_DETAIL + detail + new_line;
                                                                                                      
          throw ex.c_str();
      }
      else {
          const char** details_ptr = soap_faultdetail( & m_soap );
          std::string detail;
          if (details_ptr[0] != NULL ) detail = details_ptr[0];
             else detail = "unknown";
          std::string error = "Error in soap request towards StorageIndex Catalog. soap.fault=NULL";
          std::string SOAP_FAULT_DETAIL = "SOAP_FAULT_DETAIL: ";
          std::string new_line = "\n";
          std::string ex = error + new_line + SOAP_FAULT_DETAIL + detail + new_line;

          throw ex.c_str();
      }
   }

   for (int i=0; i < output._listSEbyGUIDReturn->__size; i++ ) {
      std::string str = output._listSEbyGUIDReturn->__ptr[i];
      list.push_back(str);
   }


   delete [] guidCopy;

}

void 
glite::wms::brokerinfo::sici::StorageIndexCatalogInterface::listSEbyLFN( const std::string &lfn, std::vector<std::string> & list, const classad::ClassAd & classad) 
{
   //////////////...for using secure endpoint
   bool proxyInJdl = true;
   std::string proxy;
   try {
      proxy =  jdl::get_x509_user_proxy(classad);
   }
   catch(...) {
      proxyInJdl = false;
   }

   if( 0 == strncasecmp(m_endpoint.c_str(),"https://",8) ){

      if ( proxyInJdl ){
         if ( !ctx ) {
            if ( glite_gsplugin_init_context(&ctx) ) {
               throw "gsplugin_init_context FAILED";
            }
         }
         //glite_gsplugin_free_context will free *both* cert and key
         ctx->cert_filename = strdup(proxy.c_str());
         ctx->key_filename = strdup(proxy.c_str());
      }
      else throw "UserProxy not specified in the ClassAd";
                                                                                                                             
      if ( soap_register_plugin_arg(&m_soap, glite_gsplugin, ctx) ) {
         throw "soap_register_plugin_arg FAILED";
      }
                                                                                                                             
   }
                                                                                                                             
////////////////////////////////////////////////////

   struct storageindex__listSEbyLFNResponse output;
				                    
   char  *lfnCopy = new char[lfn.length() + 1];
   
   strcpy(lfnCopy, lfn.c_str()); 

   if( soap_call_storageindex__listSEbyLFN(&m_soap, m_endpoint.c_str(), NULL, lfnCopy, &output) ) {
      delete [] lfnCopy;
      if ( m_soap.fault != NULL ) { 

          const char** details_ptr = soap_faultdetail( & m_soap );
          std::string detail; 
          if (details_ptr[0] != NULL ) detail = details_ptr[0];
             else detail = "unknown";
          std::string fault_code;
          if ( m_soap.fault->faultcode != NULL ) fault_code = m_soap.fault->faultcode;
             else fault_code = "unknown";
          std::string fault_string;
          if ( m_soap.fault->faultstring != NULL ) fault_string =  m_soap.fault->faultstring;
             else fault_string = "unknown";
          std::string SOAP_FAULTCODE = "SOAP_FAULTCODE: ";
          std::string SOAP_FAULTSTRING = "SOAP_FAULTSTRING: ";
          std::string SOAP_FAULT_DETAIL = "SOAP_FAULT_DETAIL: ";
          std::string new_line = "\n";

          std::string ex = SOAP_FAULTCODE + fault_code + new_line +
                           SOAP_FAULTSTRING +  fault_string + new_line +
                           SOAP_FAULT_DETAIL + detail + new_line;

          throw ex.c_str();
      }
      else {
          const char** details_ptr = soap_faultdetail( & m_soap );
          std::string detail;
          if (details_ptr[0] != NULL ) detail = details_ptr[0];
             else detail = "unknown";          
          std::string error = "Error in soap request towards StorageIndex Catalog. soap.fault=NULL";
          std::string SOAP_FAULT_DETAIL = "SOAP_FAULT_DETAIL: ";
          std::string new_line = "\n";
          std::string ex = error + new_line + SOAP_FAULT_DETAIL + detail + new_line;

          throw ex.c_str();
      }
   }

   for (int i=0; i < output._listSEbyLFNReturn->__size; i++ ) {
      std::string str = output._listSEbyLFNReturn->__ptr[i];
      list.push_back(str);
   }

   delete [] lfnCopy;

}
    


glite::wms::brokerinfo::sici::StorageIndexCatalogInterface::~StorageIndexCatalogInterface()
{
  // Finalise SOAP
  //
  soap_destroy(&m_soap);
  soap_end(&m_soap);
  soap_done(&m_soap);
  if ( ctx ) glite_gsplugin_free_context(ctx);
} // destructor


/*****************************************************************************/
/*  Class factories that can be used for a plug-in                           */
/*****************************************************************************/
extern "C" glite::wms::brokerinfo::sici::StorageIndexCatalogInterface* create(const std::string& endpoint)
{
  return new glite::wms::brokerinfo::sici::StorageIndexCatalogInterface(endpoint);
}

extern "C" glite::wms::brokerinfo::sici::StorageIndexCatalogInterface* create_with_timeout(const std::string& endpoint, int timeout)
{
  return new glite::wms::brokerinfo::sici::StorageIndexCatalogInterface(endpoint, timeout);
}

 
extern "C" void destroy(glite::wms::brokerinfo::sici::StorageIndexCatalogInterface* p) {
  delete p;
}


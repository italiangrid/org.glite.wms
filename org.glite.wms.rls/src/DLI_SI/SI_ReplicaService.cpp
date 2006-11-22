//
//  File :     SI_ReplicaService.cpp
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


#include <storageindexH.h>
#include <storageindex.nsmap>
#include <string>
#include "ReplicaServiceException.h"
#include "SI_ReplicaService.h"
#include "gsoap_rls_utils.h"


namespace glite {
namespace wms {
namespace rls {
namespace SI {

SI_ReplicaService::SI_ReplicaService(
   const std::string& endpoint,
   const std::string& proxy,
   int timeout
): m_endpoint(endpoint), m_proxy(proxy)
{
   soap_init(&m_soap);
   m_soap.namespaces = storageindex_namespaces;

   if (timeout){  
      m_soap.connect_timeout = timeout;

      m_soap.send_timeout = timeout;
      m_soap.recv_timeout = timeout;
   }

   m_ctx = NULL;
}

void 
SI_ReplicaService::listReplica(
   const std::string& inputData,
   std::vector<std::string>& sfns){

   if( 0 == strncasecmp(m_endpoint.c_str(),"https://",8) ){
 
     if ( !m_ctx ){
       if ( glite_gsplugin_init_context(&m_ctx) ) {
            throw ReplicaServiceException("gsplugin_init_context FAILED");
       }
       //glite_gsplugin_free_context will free *both* cert and key
       m_ctx->cert_filename = strdup(m_proxy.c_str());
       m_ctx->key_filename = strdup(m_proxy.c_str());
 
       if ( soap_register_plugin_arg(&m_soap, glite_gsplugin, m_ctx) ) {
          throw ReplicaServiceException("soap_register_plugin_arg FAILED");
       }
     }
   } 
  
   string lfn_no_prefix = inputData;
   string::size_type colon_pos;
   if ((colon_pos = lfn_no_prefix.find(":"))!=string::npos) {
      colon_pos++;
      lfn_no_prefix.erase(0,colon_pos);
   }

   string prefix = inputData;
   if ((colon_pos = prefix.find(":"))!=string::npos) {
      prefix.erase(colon_pos, inputData.size());
   }

   if ( prefix == "guid" ){
      struct ns1__listSEbyGUIDResponse output;

      if ( soap_call_ns1__listSEbyGUID(
            &m_soap, 
            m_endpoint.c_str(), 
            NULL, 
            const_cast<char*>(lfn_no_prefix.c_str()),
            output
          ) 
      ) {
         string ex = "trying to resolve "+inputData+"\n";
         throw ReplicaServiceException(ex+exception_reason(m_soap));
      }

      for (int i=0; i < output._listSEbyGUIDReturn->__size; i++ ) {
         std::string str = output._listSEbyGUIDReturn->__ptr[i];
         sfns.push_back(str);
      }

   }
   else if( prefix == "lfn" ){
      struct ns1__listSEbyLFNResponse output;

      if( soap_call_ns1__listSEbyLFN(
             &m_soap, 
             m_endpoint.c_str(), 
             NULL, 
             const_cast<char*>(lfn_no_prefix.c_str()),
             output) 
      ) {
         string ex = "trying to resolve "+inputData+"\n";
         throw ReplicaServiceException(ex+exception_reason(m_soap));
      }

      for (int i=0; i < output._listSEbyLFNReturn->__size; i++ ) {
         std::string str = output._listSEbyLFNReturn->__ptr[i];
         sfns.push_back(str);
      }

   } 
   else throw ReplicaServiceException("wrong prefix in input data");

}

SI_ReplicaService::~SI_ReplicaService()
{
   soap_destroy(&m_soap);
   soap_end(&m_soap);
   soap_done(&m_soap);
   if ( m_ctx ) glite_gsplugin_free_context(m_ctx);
} 

extern "C" SI_ReplicaService* create_SI( 
                std::string endpoint,
                std::string proxy,
                int timeout
           )
{
   return new SI_ReplicaService( 
                   endpoint,
                   proxy, 
                   timeout
              );
}

 
extern "C" void destroy_SI(SI_ReplicaService* p) {
   delete p;
}

} // SI
} // rls
} // wms
} // glite

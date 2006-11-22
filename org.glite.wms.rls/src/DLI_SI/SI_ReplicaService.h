//
//  File :     SI_ReplicaService.h
//
//  Author :   Enzo Martelli ($Author$)
//  e-mail :   "enzo.martelli@ct.infn.it"
//
//  Description:
//  Wraps the Storage Index soap Catalog client API
//   
//   
//  Copyright (c) 2004 Istituto Nazionale di Fisica Nucleare (INFN). 
//  All rights reserved.
//  See http://grid.infn.it/grid/license.html for license details. 
//
#ifndef GLITE_WMS_RLS_SI_REPLICASERVICE_H
#define GLITE_WMS_RLS_SI_REPLICASERVICE_H 

#include <string>
#include <vector>

#include <stdsoap2.h>

#include "ReplicaService.h"

extern "C" {
   #include "glite/security/glite_gsplugin.h"
}


namespace glite {
namespace wms {
namespace rls {
namespace SI {

class SI_ReplicaService : public ReplicaService
{
public:

   SI_ReplicaService( 
      const std::string& endpoint, 
      const std::string& proxy, 
      int timeout 
   );

   void listReplica(
              const std::string& inputData,
              std::vector<std::string>& sfns
        );

   ~SI_ReplicaService();

private:
   struct soap m_soap;     
   std::string m_endpoint; 
   glite_gsplugin_Context m_ctx;
   std::string m_proxy;
};

typedef SI_ReplicaService* create_SI_t(
                      const std::string& endpoint,
                      const std::string& proxy,
                      int timeout
        );

typedef void destroy_SI_t(SI_ReplicaService*);

} // end namespace SI
} // end namespace rls
} // end namespace wms
} // end namespace glite


#endif 
    

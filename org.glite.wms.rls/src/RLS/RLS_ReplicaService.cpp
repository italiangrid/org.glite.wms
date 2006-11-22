/***************************************************************************
 **  filename  : RLS_ReplicaService.cpp
 **  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 **  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 **  For license conditions see LICENSE file or
 **  http://www.edg.org/license.html
 ****************************************************************************/


#include <string>
#include <vector>

#include "RLS_ReplicaService.h" 
#include "ReplicaServiceException.h"

#include <EdgReplicaManager/ReplicaManagerImpl.h>
#include <EdgReplicaManager/ReplicaManagerExceptions.h>
#include <EdgReplicaLocationService/ReplicationExceptions.h>
#include <EdgReplicaMetadataCatalog/ReplicationExceptions.h>


using namespace std;

namespace replicamanager = EdgReplicaManager;
namespace replicalocationservice = EdgReplicaLocationService;
namespace replicametadatacatalog = EdgReplicaMetadataCatalog;
namespace replicaoptimization = EdgReplicaOptimization;


namespace glite {
namespace wms {
namespace rls {
namespace RLS {


RLS_ReplicaService::RLS_ReplicaService(const string& vo)
{
  try {
    m_rm.reset(new replicamanager::ReplicaManagerImpl(vo));
  }
  catch (replicamanager::ReplicaManagerException& ex) {
    throw ReplicaServiceException(ex.what());
  }
}

void
RLS_ReplicaService::listReplica(const string& lfn, vector<string>& sfns)
{
  try {
    m_rm->listReplicas(lfn, sfns);
  }
  catch (replicamanager::ReplicaManagerException& ex) {
    throw ReplicaServiceException(ex.what());
  }
  catch (replicalocationservice::NoSuchGuidException& ex) {
    throw ReplicaServiceException(ex.what());
  }
  catch (replicalocationservice::NoSuchPfnException& ex) {
    throw ReplicaServiceException(ex.what());
  }
  catch (replicalocationservice::CommunicationException& ex) {
    throw ReplicaServiceException(ex.what());
  }
  catch (replicametadatacatalog::NoSuchAliasException& ex) {
    throw ReplicaServiceException(ex.what());
  }
  catch (replicametadatacatalog::CommunicationException& ex) {
    throw ReplicaServiceException(ex.what());
  }
}

RLS_ReplicaService::~RLS_ReplicaService(){
}

// the class factories
 
extern "C" RLS_ReplicaService* create_RLS(const std::string& vo){
            return new RLS_ReplicaService(vo);
}

extern "C" void destroy_RLS(RLS_ReplicaService* p) {
            delete p;
}

} // namespace RLS
} // namespace rls
} // namespace wms
} // namespace glite



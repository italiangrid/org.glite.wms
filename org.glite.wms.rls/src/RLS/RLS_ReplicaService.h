/***************************************************************************
 **  filename  : ReplicaServiceReal.h
 **  authors   : Enzo Martelli <enzo.ronchieri@ct.infn.it>
 **  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 **  For license conditions see LICENSE file or
 **  http://www.edg.org/license.html
 ****************************************************************************/


#ifndef GLITE_WMS_RLS_REPLICASERVICE_H
#define GLITE_WMS_RLS_REPLICASERVICE_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "ReplicaService.h"

namespace EdgReplicaManager {
  class ReplicaManagerImpl;
}

namespace replicamanager = EdgReplicaManager;

namespace glite {
namespace wms {
namespace rls {
namespace RLS {


class RLS_ReplicaService : public ReplicaService
{

public:

  RLS_ReplicaService(const std::string& vo);
  ~RLS_ReplicaService();

public:

  void listReplica(
             const std::string& lfn,
             std::vector<std::string>& sfns
  );

private:

  boost::shared_ptr<replicamanager::ReplicaManagerImpl> m_rm;

}; 

// Types for plug-in
//
typedef RLS_ReplicaService* create_RLS_t(const std::string&);
typedef void destroy_RLS_t(RLS_ReplicaService*);

} // namespace RLS
} // namespace rls
} // namespace wms
} // namespace glite

#endif



/***************************************************************************
 **  filename  : ReplicaServiceReal.h
 **  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 **  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 **  For license conditions see LICENSE file or
 **  http://www.edg.org/license.html
 ****************************************************************************/

// $Id$

#ifndef GLITE_WMS_RLS_REPLICASERVICEREAL_H
#define GLITE_WMS_RLS_REPLICASERVICEREAL_H

#include <string>
#include <vector>

#ifndef GLITE_WMS_RLS_REPLICASERVICE_H
#include "glite/wms/rls/ReplicaService.h"
#endif

namespace EdgReplicaManager {
  class ReplicaManagerImpl;
}

namespace replicamanager = EdgReplicaManager;

namespace glite {
namespace wms {
namespace rls {

class ReplicaServiceReal : public ReplicaService
{
public:

  ReplicaServiceReal(const std::string& vo);

  virtual ~ReplicaServiceReal(void);

public:

  virtual std::vector<std::string> listReplica(const std::string& lfn);

  virtual access_cost_info_container_type getAccessCost(const std::vector<std::string>& lfns,
		                                 const std::vector<std::string>& ces,
						 const std::vector<std::string>& protocols);
  
private:

  replicamanager::ReplicaManagerImpl* m_rm;
}; 

// Types for plug-in
//
typedef ReplicaServiceReal* create_t(const std::string&);
typedef void destroy_t(ReplicaServiceReal*);


} // namespace rls
} // namespace wms
} // namespace glite

#endif

/*
  Local Variables:
  mode: c++
  End:
*/


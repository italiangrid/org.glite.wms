/***************************************************************************
 **  filename  : ReplicaService.h
 **  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 **  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 **  For license conditions see LICENSE file or
 **  http://www.edg.org/license.html
 ****************************************************************************/

#ifndef GLITE_WMS_RLS_REPLICASERVICE_H
#define GLITE_WMS_RLS_REPLICASERVICE_H

#include <string>
#include <vector>
#include <boost/tuple.hpp>

namespace glite {
namespace wms {
namespace rls {
	
typedef boost::tuple::triple<std::string, float, float> access_cost_info_type;
typedef std::vector<access_cost_info_type> access_cost_info_container_type;

class ReplicaService
{
public:
  
  virtual ~ReplicaService(void) {};

public:

  virtual std::vector<std::string> listReplica(const std::string& lfn) = 0;
    
  virtual access_cost_info_container_type getAccessCost(const std::vector<std::string>& lfns,
    			                         const std::vector<std::string>& ces,
						 const std::vector<std::string>& protocols) = 0;
  
};

} // namespace rls
} // namespace wms
} // namespace glite

#endif 

/*
  Local Variables:
  mode: c++
  End:
*/


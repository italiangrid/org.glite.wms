/***************************************************************************
 **  filename  : ReplicaServiceReal.cpp
 **  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 **  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 **  For license conditions see LICENSE file or
 **  http://www.edg.org/license.html
 ****************************************************************************/

// $Id$

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <utility>
#include <numeric>

#include "glite/wms/rls/ReplicaServiceReal.h" 

#include <EdgReplicaManager/ReplicaManagerImpl.h>
#include <EdgReplicaManager/ReplicaManagerExceptions.h>
#include <EdgReplicaLocationService/ReplicationExceptions.h>
#include <EdgReplicaMetadataCatalog/ReplicationExceptions.h>

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "ReplicaServiceException.h"

using namespace std;

namespace replicamanager = EdgReplicaManager;
namespace replicalocationservice = EdgReplicaLocationService;
namespace replicametadatacatalog = EdgReplicaMetadataCatalog;
namespace replicaoptimization = EdgReplicaOptimization;

namespace logger = glite::wms::common::logger;
namespace ts = glite::wms::common::logger::threadsafe;

namespace glite {
namespace wms {
namespace rls {

namespace {

struct insertAccessCostInfoInVector : 
	binary_function<access_cost_info_container_type*,
			replicaoptimization::AccessCost,
			access_cost_info_container_type*>
{
  access_cost_info_container_type* operator()(
    access_cost_info_container_type* v, 
    replicaoptimization::AccessCost c)
  {
    v->push_back(boost::make_tuple(string(c.getCeId()), c.getTotalTime(), c.getSizeToBeReplicated() ));
    return v;
  }
};

}

ReplicaServiceReal::ReplicaServiceReal(const string& vo)
{
  logger::StatePusher pusher(ts::edglog, "ReplicaServiceReal()");
  try {
    m_rm = new replicamanager::ReplicaManagerImpl(vo);
  }
  catch (replicamanager::ReplicaManagerException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
    throw InvalidRLS(ex.what());
  }
}

ReplicaServiceReal::~ReplicaServiceReal(void)
{
  delete m_rm;
}

vector<string>
ReplicaServiceReal::listReplica(const string& lfn)
{
  vector<string> pfns;
  logger::StatePusher pusher(ts::edglog, "listReplica()");
  
  try {
    m_rm->listReplicas(lfn, pfns);
  }
  catch (replicamanager::ReplicaManagerException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::NoSuchGuidException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::NoSuchPfnException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::CommunicationException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicametadatacatalog::NoSuchAliasException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicametadatacatalog::CommunicationException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  return pfns;
}

access_cost_info_container_type
ReplicaServiceReal::getAccessCost(const vector<string>& lfns,
	                          const vector<string>& ces,
				  const vector<string>& protocols)
{
  vector<replicaoptimization::AccessCost> costs;
  access_cost_info_container_type         costs_info;
  
  logger::StatePusher pusher(ts::edglog, "getAccessCost()");
 
  try {
    m_rm->getAccessCost(lfns, ces, protocols, costs);

    accumulate(costs.begin(), costs.end(), &costs_info, insertAccessCostInfoInVector());
  }
  catch (replicamanager::ReplicaManagerException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::NoSuchGuidException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::NoSuchPfnException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicalocationservice::CommunicationException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicametadatacatalog::NoSuchAliasException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  catch (replicametadatacatalog::CommunicationException& ex) {
    ts::edglog << logger::setlevel(logger::error) << ex.what() << endl;
  }
  
  return costs_info;
}

// the class factories
 
extern "C" ReplicaServiceReal* create(const std::string& vo){
            return new ReplicaServiceReal(vo);
}
 
extern "C" void destroy(ReplicaServiceReal* p) {
            delete p;
}


} // namespace rls
} // namespace wms
} // namespace glite



// File: RBMinimizeAccessCostImpl.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$ 

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector> 

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <classad_distribution.h>

#include "glite/jdl/ManipulationExceptions.h"
#include "glite/jdl/JobAdManipulation.h"

#include "glite/wms/matchmaking/matchmakerGlueImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "glite/wms/broker/RBMinimizeAccessCostImpl.h"
#include "utility.h"
#include "glite/wms/rls/ReplicaServiceReal.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)

using namespace std;

namespace requestad = glite::jdl;

namespace glite {
namespace wms {
	
namespace logger        = common::logger;

using namespace matchmaking;
using namespace brokerinfo;
using namespace rls;


namespace broker {

namespace 
{
	struct IsAvailableSpaceAtLeast
	{
		IsAvailableSpaceAtLeast( float s ) : required_space( s ) {}
		bool operator()(const BrokerInfoData::CloseSEInfo_map_type::value_type& i)
		{
			double size;
			return i.second->EvaluateAttrReal("freespace", size) &&
					size >= required_space;
			
		}
		float required_space;	
	};
}

RBMinimizeAccessCostImpl::RBMinimizeAccessCostImpl(BrokerInfo<brokerinfoGlueImpl> *bi)
{
	BI = bi;
}
 
RBMinimizeAccessCostImpl::~RBMinimizeAccessCostImpl()
{
}

match_table_t* RBMinimizeAccessCostImpl::findSuitableCEs(const classad::ClassAd* requestAd)
{
  if (!requestAd) return 0;

  classad::ClassAd jdl(*requestAd);

  match_table_t* suitableCEs = 0;
  MatchMaker<matchmakerGlueImpl> MM;
  suitableCEs = new match_table_t;
  MM.checkRequirement(jdl, *suitableCEs);
  // prepare the vector of computing element which will be used to call
  // getAccessCost function
  vector<string> CEs;
  std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &CEs, insertCEsInVector() );

  BrokerInfoData::LFN_container_type input_data;
  vector<string> data_access_protocol;
  requestad::get_input_data(jdl, input_data);
  requestad::get_data_access_protocol(jdl, data_access_protocol);

  // Collects all the information about SFNs and involved SEs.
  // NOTE: this information is not CE dependent.
  BI->retrieveSFNsInfo(jdl);
  BI->retrieveSEsInfo (jdl);
					
  // call getAccessCost function
  // there is no need to rank the CE using the checkRank function
  // since the access cost will be used as rank value
  bool exits_virtual_organisation;
  string vo(requestad::get_virtual_organisation(jdl, exits_virtual_organisation));
  if (!exits_virtual_organisation) {
  
	edglog(error) << "VirtualOrganisation field does not exist..." << endl;
  	return 0;
  }

// Commenting this breaks the accesscost-based ranking, but removes the 
// dependency of libbroker from the EDG RLS.
// The getaccesscost function is going to disappear, so this is
// the first step before removing this impl entirely.
//
//  boost::scoped_ptr<ReplicaService> replica( new ReplicaServiceReal(vo) );
// 
//   access_cost_info_container_type aic( replica->getAccessCost(input_data, CEs, data_access_protocol) );
//   
//   vector<string> deletingCEs;
//   for( access_cost_info_container_type::const_iterator it = aic.begin();
// 	       it != aic.end(); it++ ) {
// 	       
// 	BI->retrieveCloseSEsInfo(boost::tuples::get<0>(*it));
// 	BI->retrieveCloseSAsInfo(vo); // Retrieves only GlueSAAvailableVOSpace
// 	
// 	BrokerInfoData::CloseSEInfo_const_iterator CloseSEInfo_begin, CloseSEInfo_end;
// 	boost::tie(CloseSEInfo_begin, CloseSEInfo_end) = (*BI)->CloseSEInfo_map();
// 	
// 	if( std::find_if( CloseSEInfo_begin, CloseSEInfo_end, IsAvailableSpaceAtLeast(boost::tuples::get<2>(*it)) ) ==
// 			CloseSEInfo_end ) {
// 
// 		deletingCEs.push_back(boost::tuples::get<0>(*it));
// 	}
// 	
// 	edglog( debug ) << boost::tuples::get<0>(*it) << " access cost = " << boost::tuples::get<1>(*it) << " replication size = " << boost::tuples::get<2>(*it) << endl;
// 	(*suitableCEs)[boost::tuples::get<0>(*it)].setRank(boost::tuples::get<1>(*it));
//   }
//   std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs));
  
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite


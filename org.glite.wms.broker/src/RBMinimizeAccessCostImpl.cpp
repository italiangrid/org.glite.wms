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

#include "glite/wms/common/requestad/ManipulationExceptions.h"
#include "glite/wms/common/requestad/JobAdManipulation.h"

#include "glite/wms/matchmaking/matchmakerGlueImpl.h"
#include "glite/wms/matchmaking/glue_attributes.h"
#include "glite/wms/matchmaking/jdl_attributes.h"
#include "glite/wms/broker/RBMinimizeAccessCostImpl.h"
#include "glite/wms/broker/utility.h"
#include "glite/wms/rls/ReplicaServiceReal.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)

using namespace std;

namespace glite {
namespace wms {
	
namespace logger        = common::logger;
namespace requestad     = common::requestad;

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

RBMinimizeAccessCostImpl::RBMinimizeAccessCostImpl(brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> *bi)
{
	BI = bi;
}
 
RBMinimizeAccessCostImpl::~RBMinimizeAccessCostImpl()
{
}

match_table_t* RBMinimizeAccessCostImpl::findSuitableCEs(const classad::ClassAd* requestAd)
{
  if (!requestAd) return 0;
 	  
  matchmaking::match_table_t* suitableCEs = 0;
  MatchMaker<matchmakerGlueImpl> MM;
  suitableCEs = new match_table_t;
  MM.checkRequirement(requestAd, *suitableCEs);
  // prepare the vector of computing element which will be used to call
  // getAccessCost function
  vector<string> CEs;
  std::accumulate( suitableCEs -> begin(), suitableCEs -> end(), &CEs, insertCEsInVector() );

  BrokerInfoData::LFN_container_type input_data;
  vector<string> data_access_protocol;
  requestad::get_input_data(*requestAd, input_data);
  requestad::get_data_access_protocol(*requestAd, data_access_protocol);

  // Collects all the information about SFNs and involved SEs.
  // NOTE: this information is not CE dependent.
  BI->retrieveSFNsInfo(*requestAd);
  BI->retrieveSEsInfo (*requestAd);
					
  // call getAccessCost function
  // there is no need to rank the CE using the checkRank function
  // since the access cost will be used as rank value
  bool exits_virtual_organisation;
  string vo(requestad::get_virtual_organisation(*requestAd, exits_virtual_organisation));
  if (!exits_virtual_organisation) {
  
	edglog(error) << "VirtualOrganisation field does not exist..." << endl;
  	return 0;
  }
  boost::scoped_ptr<ReplicaService> replica( new ReplicaServiceReal(vo) );

  access_cost_info_container_type aic( replica->getAccessCost(input_data, CEs, data_access_protocol) );
  
  vector<string> deletingCEs;
  for( access_cost_info_container_type::const_iterator it = aic.begin();
	       it != aic.end(); it++ ) {
	       
	BI->retrieveCloseSEsInfo(it->first);
	BI->retrieveCloseSAsInfo(vo); // Retrieves only GlueSAAvailableVOSpace
	
	BrokerInfoData::CloseSEInfo_const_iterator CloseSEInfo_begin, CloseSEInfo_end;
	boost::tie(CloseSEInfo_begin, CloseSEInfo_end) = (*BI)->CloseSEInfo_map();
	
	if( std::find_if( CloseSEInfo_begin, CloseSEInfo_end, IsAvailableSpaceAtLeast(it->third) ) ==
			CloseSEInfo_end ) {

		deletingCEs.push_back(it->first);
	}
	
	edglog( debug ) << it->first << " access cost = " << it->second 
		<< " replication size = " << it->third << endl;
	(*suitableCEs)[it->first].setRank( it->second );
  }
  std::for_each(deletingCEs.begin(), deletingCEs.end(), removeCEFromMatchTable(suitableCEs));
  
  return suitableCEs;
}

} // namespace broker
} // namespace wms
} // namespace glite


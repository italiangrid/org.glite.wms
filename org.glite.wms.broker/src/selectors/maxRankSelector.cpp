/*
 * File: maxRankSelector.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_smallint.hpp>

#include <vector>
#include "glite/wms/broker/maxRankSelector.h"

using namespace std;

namespace glite {
namespace wms {
namespace broker {

namespace 
{
 static boost::minstd_rand f_rnd;

 struct clustered_rank_less_than_comparator :
	 public unary_function< maxRankSelector::rank_to_match_container_map_type::value_type&, bool>
	 {
		 bool operator()(const maxRankSelector::rank_to_match_container_map_type::value_type& a,
				 const maxRankSelector::rank_to_match_container_map_type::value_type& b)
		 {
	          return a.first < b.first;
		 }
	 };
 struct clustered_rank_greater_than_comparator :
	 public unary_function< maxRankSelector::rank_to_match_container_map_type::value_type&, bool>
	 {
		 bool operator()(const maxRankSelector::rank_to_match_container_map_type::value_type& a,
				 const maxRankSelector::rank_to_match_container_map_type::value_type& b)
		 {
	          return a.first > b.first;
		 }
	 };

}
	
maxRankSelector::maxRankSelector()
{
  f_rnd.seed(time(0));
}
 
maxRankSelector::~maxRankSelector()
{
}	  

matchmaking::match_const_iterator maxRankSelector::selectBestCE(const matchmaking::match_table_t& match_table)
{
  rank_to_match_container_map_type clustered_rank_match_table;
  
  for( matchmaking::match_table_t::const_iterator it=match_table.begin(); it!=match_table.end(); it++) {
  	
	  clustered_rank_match_table[it->second.getRank()].push_back(it);
  }
  rank_to_match_container_map_type::const_iterator max_cluster = 
   	std::max_element(clustered_rank_match_table.begin(), 
			 clustered_rank_match_table.end(), 
			 clustered_rank_less_than_comparator());
  
  if( max_cluster == clustered_rank_match_table.end() ) return match_table.end();
  
  size_t n = max_cluster -> second.size();
  if( n==1 ) return max_cluster -> second.front();
  boost::uniform_smallint<boost::minstd_rand> unirand(f_rnd, 0, n-1); 
  return max_cluster -> second[ unirand() ];
}	

}; // namespace broker
}; // namespace wms
}; // namespace glite

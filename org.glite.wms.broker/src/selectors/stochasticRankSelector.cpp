/*
 * File: stochasticRankSelector.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <algorithm>
#include <vector>

#include "edg/workload/planning/broker/stochasticRankSelector.h"

using namespace std;

namespace edg {
namespace workload {
namespace planning {
namespace broker {

namespace 
{
  static boost::minstd_rand f_rnd;
}
	
stochasticRankSelector::stochasticRankSelector()
{
  f_rnd.seed(time(0));
  m_unirand01.reset( new boost::uniform_01<boost::minstd_rand>(f_rnd) );
}
 
stochasticRankSelector::~stochasticRankSelector()
{
}	  

matchmaking::match_const_iterator stochasticRankSelector::selectBestCE(const matchmaking::match_table_t& match_table)
{
  if( match_table.empty() ) return match_table.end();

  double rank_sum = 0.0;
  double rank_min = 0.0;
  
  vector<double> rank;

  for(matchmaking::match_table_t::const_iterator it = match_table.begin(); it != match_table.end(); it++) {

    double k = it -> second.getRank(); 
  	    
    if( k < rank_min ) rank_min = k;  
    
    rank_sum += k;    
    rank.push_back( k );
  }

  double traslation = 0.0;
  double prob_sum   = 0.0;
  
  // if rank_min is less than 0 then we need to translate the
  // ranks in order to have only positive values...
  if( rank_min != 0.0 ) traslation =  - 2.0 * rank_min;
 
  double p = (*m_unirand01)() * (rank_sum + rank.size()*traslation);
  
  size_t i = 0;
  matchmaking::match_table_t::const_iterator best = match_table.begin();
  do {
    prob_sum += (rank[i++]+traslation) ;
    if(prob_sum >= p) break;
  } while( ++best != match_table.end() );
  
  return best;
}	

}; // namespace broker
}; // namespace planning
}; // workload
}; // edg

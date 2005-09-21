/*
 * File: stochasticRankSelector.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <algorithm>
#include <vector>

#include "stochasticRankSelector.h"

using namespace std;

namespace glite {
namespace wms {
namespace broker {

namespace 
{
  static boost::minstd_rand f_rnd(time(0));
}
	
stochasticRankSelector::stochasticRankSelector()
{
  m_unirand01.reset( new boost::uniform_01<boost::minstd_rand>(f_rnd) );
}
 
stochasticRankSelector::~stochasticRankSelector()
{
}	  

matchmaking::match_const_iterator stochasticRankSelector::selectBestCE(const matchmaking::match_table_t& match_table)
{
  if( match_table.empty() ) return match_table.end();

  vector<double> rank;

  double rank_sum = 0.0;

  for(matchmaking::match_table_t::const_iterator it = match_table.begin(); it != match_table.end(); it++) {

    double r = it -> second.getRank();
    rank.push_back( r );
    rank_sum += r;
  }

  double rank_mean      =  rank_sum / (double)(rank.size());
  double rank_variance  =  0.0;
  double rank_deviation =  0.0;

  // We smooth rank values according to the following function:
  // f(x) = atan( V * (x - mean ) / dev ) + PI
  // Thanks to Alessio Gianelle for his usefull support and suggestions.

  static const double PI = std::atan(1.0)*4.0;
  static const double V = PI;

  // Computing the variance and standard deviation of rank samples...

  for(size_t r=0; r < rank.size(); r++)
          rank_variance += pow(( rank[r] - rank_mean), 2);

  rank_variance /= (double)(rank.size());
  rank_deviation = rank_variance > 0 ? sqrt( rank_variance ) : V;
  rank_sum = 0.0;

  for(size_t r=0; r < rank.size(); r++) {
    double x = rank[r];
    rank[r] = atan( V * (x - rank_mean) / rank_deviation ) + PI;
    rank_sum += rank[r];
  }

  double prob_sum   = 0.0;
  double p = (*m_unirand01)() * rank_sum;
  size_t i = 0;
  matchmaking::match_table_t::const_iterator best = match_table.begin();
  do {
    prob_sum += rank[i++];
    if ( p < prob_sum ) break;
  } while( ++best != match_table.end() );

  return best;
}	

}; // namespace broker
}; // namespace wms
}; // namespace glite

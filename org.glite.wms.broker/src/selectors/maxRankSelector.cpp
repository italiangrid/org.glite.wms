/*
 * File: maxRankSelector.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include "maxRankSelector.h"
#include <vector>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>

using namespace std;

namespace glite {
namespace wms {
namespace broker {

namespace {

boost::minstd_rand f_rnd;

struct clustered_rank_less_than_comparator:
    public unary_function<maxRankSelector::rank_to_match_container_map_type::value_type&, bool>
{
  bool operator()(
    maxRankSelector::rank_to_match_container_map_type::value_type const& a,
    maxRankSelector::rank_to_match_container_map_type::value_type const& b
  )
  {
    return a.first < b.first;
  }
};

struct clustered_rank_greater_than_comparator :
    public unary_function<maxRankSelector::rank_to_match_container_map_type::value_type&, bool>
{
  bool operator()(
    maxRankSelector::rank_to_match_container_map_type::value_type const& a,
    maxRankSelector::rank_to_match_container_map_type::value_type const& b
  )
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

matchmaking::match_const_iterator
maxRankSelector::selectBestCE(matchmaking::match_table_t const& match_table)
{
  rank_to_match_container_map_type clustered_rank_match_table;

  for (matchmaking::match_table_t::const_iterator it = match_table.begin();
       it != match_table.end(); ++it) {

    clustered_rank_match_table[it->second.getRank()].push_back(it);
  }
  rank_to_match_container_map_type::const_iterator max_cluster =
    std::max_element(
      clustered_rank_match_table.begin(),
      clustered_rank_match_table.end(),
      clustered_rank_less_than_comparator()
    );

  if (max_cluster == clustered_rank_match_table.end()) {
    return match_table.end();
  }

  size_t n = max_cluster->second.size();

  if (n == 1) {
    return max_cluster->second.front();
  }

  boost::uniform_smallint<size_t> distrib(0, n - 1);
  boost::variate_generator<
    boost::minstd_rand,
    boost::uniform_smallint<size_t>
    > unirand(f_rnd, distrib);

  return max_cluster->second[unirand()];
}

}}} // glite::wms::broker

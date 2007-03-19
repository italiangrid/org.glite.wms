/*
 * File: maxRankSelector.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>

#include "max_selector.h"

using namespace std;

namespace glite {
namespace wms {
namespace broker {

namespace {

boost::minstd_rand f_rnd(std::time(0));

matchtable::iterator max_partition(matchtable& matches)
{
  matchtable::const_iterator const max(
    std::max_element(
      matches.begin(), matches.end(),
      rank_less_than_comparator()
    )
  );
  return std::partition(
    matches.begin(),
    matches.end(),
    rank_greater_than(max->get<Rank>())
  );
}

}

matchtable::iterator
max_selector(matchtable& matches)
{
  matchtable::iterator const max_partition_end(
    max_partition(matches)
  );
  size_t const n = std::distance(matches.begin(), max_partition_end);

  boost::uniform_smallint<size_t> distrib(0, n-1);

  boost::variate_generator<
    boost::minstd_rand,
    boost::uniform_smallint<size_t>
  > unirand(f_rnd, distrib);

  matchtable::iterator result = matches.begin();
  std::advance(result, unirand());
  return result;
}

}}} // glite::wms::broker

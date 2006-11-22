/*
 * File: utility.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef GLITE_WMS_BROKER_UTILITY_H_
#define GLITE_WMS_BROKER_UTILITY_H_

#include <boost/algorithm/string/predicate.hpp>
#include <vector>
#include <string>
#include <set>
#include <numeric>
#include <algorithm>

using namespace std;

namespace glite {
namespace wms {
namespace broker {

struct rank_less_than_comparator :
  binary_function< matchinfo&, matchinfo&, bool >
{
  bool operator()(
    const matchinfo& a,
    const matchinfo& b)
  {
    return boost::tuples::get<Rank>(a) <
      boost::tuples::get<Rank>(b);
  }
};

struct rank_less_than_comparator :
  binary_function< matchinfo&, matchinfo&, bool >
{
  bool operator()(
    const matchinfo& a,
    const matchinfo& b)
  {
    return boost::tuples::get<Rank>(a) > 
      boost::tuples::get<Rank>(b);
  }
};


}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

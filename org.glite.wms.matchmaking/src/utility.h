/*
 * File: utility.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef _GLITE_WMS_MATCHMAKING_UTILITY_H_
#define _GLITE_WMS_MATCHMAKING_UTILITY_H_

#include <vector>
#include <numeric>
#include <algorithm>
#include "matchmaker.h"

using namespace std;

namespace glite {
namespace wms {
namespace matchmaking {

typedef std::vector< std::pair<match_table_t::key_type,match_table_t::mapped_type> > match_vector_t;
	
struct rank_less_than_comparator :
    public binary_function< match_table_t::value_type&, match_table_t::value_type&, bool>
{ 
	 bool operator()(const match_table_t::value_type& a, const match_table_t::value_type& b)
	 {
	    return a.second.getRank() < b.second.getRank();
	 }
};

struct rank_greater_than_comparator :
    public binary_function< match_table_t::value_type&, match_table_t::value_type&, bool>
{
         bool operator()(const match_table_t::value_type& a, const match_table_t::value_type& b)
         {
             return a.second.getRank() > b.second.getRank();
         }
};

}; // namespace matchmaking
}; // namespace wms
}; // namespace glite

#endif

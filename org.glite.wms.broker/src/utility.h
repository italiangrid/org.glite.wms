/*
 * File: utility.h
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#ifndef GLITE_WMS_BROKER_UTILITY_H_
#define GLITE_WMS_BROKER_UTILITY_H_

#include <vector>
#include <set>
#include <numeric>
#include <algorithm>

using namespace std;

namespace glite {
namespace wms {
namespace broker {

struct insertUnRankedCEsInVector : binary_function<vector<string>*, matchmaking::match_table_t::value_type, vector<string>*>
{
	vector<string>* operator()(vector<string>* v, matchmaking::match_table_t::value_type mv)
	{
		if( mv.second.isRankUndefined() ) v -> push_back( mv.first );
		return v;
	}
};

struct insertNotInClassCEsInVector : binary_function<vector<string>*, matchmaking::match_table_t::value_type, vector<string>*>
{
		insertNotInClassCEsInVector(const std::set<std::string>& c) : CEs_class(c) {}
	        vector<string>* operator()(vector<string>* v, matchmaking::match_table_t::value_type mv)
	        {
	                if( CEs_class.find( mv.first ) == CEs_class.end() ) v -> push_back( mv.first );
		       	return v;
		}
		
const std::set<std::string>& CEs_class;
};

struct insertCEsInVector : binary_function<vector<string>*, matchmaking::match_table_t::value_type, vector<string>*>
{
		insertCEsInVector() {}
	        vector<string>* operator()(vector<string>* v, matchmaking::match_table_t::value_type mv)
	        {
	                v -> push_back( mv.first );
		       	return v;
		}
};


struct removeCEFromMatchTable
{
	removeCEFromMatchTable(matchmaking::match_table_t* mt) : suitableCEs( mt ) {}
	void operator()(const string& ce_name) { suitableCEs -> erase( ce_name); }
	matchmaking::match_table_t* suitableCEs;
};

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

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

struct extract_computing_elements_id : binary_function<
  set<string>*,
  brokerinfo::storagemapping::const_iterator,
  set<string>*
>
{
  std::set<std::string>*
  operator()(
    std::set<std::string>* s,
    brokerinfo::storagemapping::const_iterator si
  )
  {
    vector<pair<string,string> > const& info(
      boost::tuples::get<2>(si->second)
    );
    vector<pair<string,string> >::const_iterator it(info.begin());
    vector<pair<string,string> >::const_iterator const e(info.end());
    for( ; it != e ; ++it ) {
       
       s->insert(it->first);
    }
    return s;
  }
};

struct is_storage_close_to
{
  is_storage_close_to(string const& id) : m_ceid(id) {}
  bool operator()(brokerinfo::storagemapping::const_iterator const& v)
  {
   vector<pair<string,string> > const& i(
        boost::tuples::get<2>(v->second)
   );
   vector<pair<string,string> >::const_iterator it(i.begin());
   vector<pair<string,string> >::const_iterator const e(i.end());
   for( ; it != e ; ++it) {
     if(boost::starts_with(m_ceid, it->first)) return true;
   }
   return false;
  }
  string m_ceid;
};

}; // namespace broker
}; // namespace wms
}; // namespace glite

#endif

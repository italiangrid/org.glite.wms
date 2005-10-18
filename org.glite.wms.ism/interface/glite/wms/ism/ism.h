// File: ISM.h
// Author: Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//
// $Id$

#ifndef GLITE_WMS_ISM_ISM_H
#define GLITE_WMS_ISM_ISM_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace ism {

// resource identifier
typedef std::string id_type;
// resource descritpion
typedef boost::shared_ptr<classad::ClassAd> ad_ptr;
// update function
typedef boost::function<bool(int&, ad_ptr)> update_uf_type;

// 1. update time
// 2. expiry time
// 3. resource descritpion
// 4. update function
typedef boost::tuple<int, int, ad_ptr, update_uf_type> ism_entry_type;

// 1. resource identifier
// 2. ism entry type
typedef std::map<id_type, ism_entry_type> ism_type;

ism_type& get_ism();
boost::recursive_mutex& get_ism_mutex();

ism_type::value_type make_ism_entry(
  std::string const& id, // resource identifier
  int ut,	 // update time
  ad_ptr const& ad,	 // resource descritpion
  update_uf_type  const& uf = update_uf_type(), // update function
  int et = time(0) + 600    // expiry time with defualt 5*60 
);

std::ostream& operator<<(std::ostream& os, ism_type::value_type const& value);

struct call_update_ism_entries
{
  call_update_ism_entries() {};
  void operator()();
};

struct update_ism_entry 
{
  bool operator()(ism_entry_type entry);
};

bool is_expired_ism_entry(const ism_entry_type& entry);

// Returns whether the ism entry is assigned an expiry_time less or equal to 0, or not
bool is_void_ism_entry(const ism_entry_type& entry);

std::string get_ism_dump(void);

struct call_dump_ism_entries
{
  call_dump_ism_entries() {};
  void operator()();
  ~call_dump_ism_entries() { (*this)(); }
};

} // ism
} // wms
} // glite

#endif // GLITE_WMS_ISM_ISM_H

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
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace ism {

typedef std::string ce_id_type;
typedef boost::shared_ptr<classad::ClassAd> ce_ad_ptr;
typedef boost::xtime timestamp_type;
typedef boost::tuple<timestamp_type, ce_ad_ptr> ism_entry_type;
typedef std::map<ce_id_type, ism_entry_type> ism_type;

ism_type& get_ism();
boost::mutex& get_ism_mutex();

ism_type::value_type make_ism_entry(
  std::string const& ce_id,
  timestamp_type const& xt,
  ce_ad_ptr const& ce_ad
);

std::ostream& operator<<(std::ostream& os, ism_type::value_type const& value);

} // ism
} // wms
} // glite

#endif // GLITE_WMS_ISM_ISM_H

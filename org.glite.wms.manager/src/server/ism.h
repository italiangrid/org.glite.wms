// File: ISM.h
// Author: Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//
// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_ISM_H
#define GLITE_WMS_MANAGER_SERVER_ISM_H

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <map>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

typedef std::string ce_id_type;
typedef boost::shared_ptr<classad::ClassAd> ce_ad_ptr;
typedef boost::xtime timestamp_type;
typedef boost::tuple<timestamp_type, ce_ad_ptr> ism_entry_type;
typedef std::map<ce_id_type, ism_entry_type> ism_type;

timestamp_type& get_current_time(void);
ism_type& get_ism(void);
boost::mutex& get_ism_mutex(void);

ism_type::value_type make_ism_entry(std::string ce_id,
	       	timestamp_type xt,
	       	ce_ad_ptr ce_ad);

ce_id_type get_ce_id(ce_ad_ptr ad);

std::ostream& operator<<(std::ostream& os,
	       	const ism_type::value_type& value);

} // server
} // manager
} // wms
} // glite

#endif // GLITE_WMS_MANAGER_SERVER_ISM_H

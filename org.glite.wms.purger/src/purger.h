// File: purger.h
// Author: Salvatore Monforte <salvatore.monforte@ct.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//
// $Id$

#ifndef EDG_WORKLOAD_NETWORKSERVER_UTILS_PURGER_H
#define EDG_WORKLOAD_NETWORKSERVER_UTILS_PURGER_H

#include <boost/filesystem/operations.hpp> 

namespace edg {
namespace workload {
namespace common {
namespace jobid {
class JobId;
} // namespace jobid
} // namespace common
namespace purger {

 bool purgeStorageEx(const boost::filesystem::path&, int purge_threshold = 0, bool fake_rm = false);
 bool purgeStorage(const workload::common::jobid::JobId&, const std::string& sandboxdir = "");

} // namespace purger
} // namespace workload
} // edg

// Local Variables:
// mode: c++
// End:
// 

#endif /* EDG_WORKLOAD_NETWORKSERVER_UTILS_PURGER_H */

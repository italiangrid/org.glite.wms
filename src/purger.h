// File: purger.h
// Author: Salvatore Monforte <salvatore.monforte@ct.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
//
// $Id$

#ifndef GLITE_WMS_PURGER_PURGER_H
#define GLITE_WMS_PURGER_PURGER_H

#include <boost/filesystem/operations.hpp> 

namespace glite {
namespace wms {
namespace jobid {
class JobId;
} // namespace jobid
namespace purger {

 bool purgeStorageEx(const boost::filesystem::path&, int purge_threshold = 0, bool fake_rm = false);
 bool purgeStorage(const workload::common::jobid::JobId&, const std::string& sandboxdir = "");

} // namespace purger
} // namespace wms
} // namespace glite

// Local Variables:
// mode: c++
// End:
// 

#endif /* GLITE_WMS_PURGER_PURGER_H */

// File: WMFileListProxy.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_PLANNING_MANAGER_WMFILELISTPROXY_H
#define EDG_WORKLOAD_PLANNING_MANAGER_WMFILELISTPROXY_H

#include <string>
#include "edg/workload/planning/manager/WMImpl.h"
#include "edg/workload/common/utilities/FileList.h"

namespace edg {
namespace workload {

namespace common {
namespace utilities {
class FileListMutex;
}}

namespace planning {
namespace manager {

class WMFileListProxy: public WMImpl
{
  common::utilities::FileList<std::string>& m_filelist;
  common::utilities::FileListMutex&         m_filelist_mutex;

public:
  WMFileListProxy(common::utilities::FileList<std::string>& filelist,
                  common::utilities::FileListMutex& filelist_mutex);
  ~WMFileListProxy();

  void submit(classad::ClassAd const* request_ad);
  void resubmit(common::jobid::JobId const& request_id);
  void cancel(common::jobid::JobId const& request_id);
};

}}}} // edg::workload::planning::manager

#endif

// Local Variables:
// mode: c++
// End:

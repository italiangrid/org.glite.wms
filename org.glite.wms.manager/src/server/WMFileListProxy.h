// File: WMFileListProxy.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_WMFILELISTPROXY_H
#define GLITE_WMS_MANAGER_SERVER_WMFILELISTPROXY_H

#include <string>
#include "../common/WMImpl.h"
#include "glite/wms/common/utilities/FileList.h"

namespace glite {

namespace jobid = wmsutils::jobid;

namespace wms {

namespace common {
namespace utilities {
class FileListMutex;
}}

namespace common = manager::common;
namespace utilities = common::utilities;

namespace manager {
namespace server {

class WMFileListProxy: public common::WMImpl
{
  utilities::FileList<std::string>& m_filelist;
  utilities::FileListMutex&         m_filelist_mutex;

public:
  WMFileListProxy(utilities::FileList<std::string>& filelist,
                  utilities::FileListMutex& filelist_mutex);
  ~WMFileListProxy();

  void submit(classad::ClassAd const* request_ad);
  void resubmit(jobid::JobId const& request_id);
  void cancel(jobid::JobId const& request_id);
};

} // server
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:

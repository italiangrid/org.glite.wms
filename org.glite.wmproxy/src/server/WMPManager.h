/*
 * File: WMPManager.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _WMPManager_h_
#define _WMPManager_h_

#include <boost/thread/thread.hpp> 
#include <classad_distribution.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "glite/wms/common/task/Task.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "wmpresponsestruct.h"

namespace task          = glite::wms::common::task;

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

class WMPManager: public task::PipeWriter <classad::ClassAd*>
{
  
public:

  WMPManager();
  virtual ~WMPManager();
  virtual void run() {};

  virtual wmp_fault_t runCommand(const std::string& cmdname, const std::vector<std::string>& param, void* result = NULL); 

};

}
}
}
}

#endif


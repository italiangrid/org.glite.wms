/*
 * File: WMPManager.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */
 
#ifndef _WMPManager_h_
#define _WMPManager_h_

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

class WMPManager  
{
  
public:
  WMPManager();
  virtual ~WMPManager();
  
  virtual void runCommand(std::string cmdname, std::vector<std::string> param, void* result, wmp_fault_t &fault); 

};

}
}
}
}

#endif


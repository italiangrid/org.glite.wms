/*
 * File: utils.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2005 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_UTILS_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_UTILS_H_

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {
				
extern "C++"
{
  bool doPurge(std::string dg_jobid);
  bool getUserQuota(std::pair<long, long>& result, std::string uname);
  bool getUserFreeQuota(std::pair<long, long>& result, std::string uname);
}

}
}
}
}

#endif

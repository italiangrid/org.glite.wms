/*
 * DagCommandFactoryServerImpl.cpp
 * 
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_DAGCOMMANDFACTORYSERVERIMPL_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_DAGCOMMANDFACTORYSERVERIMPL_H_

namespace commands  = glite::wms::wmproxy::commands;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

  class Command;

namespace dag {

  bool createStagingDirectories(commands::Command* cmd);

} // namespace dag  
} // namespace commands
} // namespace wmproxy
} // namespace wms
} // namespace glite 

#endif

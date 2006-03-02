/*
 * File: listjobmatch.h
 * Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_LISTJOBMATCH_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_LISTJOBMATCH_H_

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {
				
extern "C++"
{
  bool listjobmatch(Command*);
  bool listjobmatchex(Command* cmd);
  std::string
listjobmatchex(const std::string &credentials_file, std::string &pipepath);
}

}
}
}
}

#endif

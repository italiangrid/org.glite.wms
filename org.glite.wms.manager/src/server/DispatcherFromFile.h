// File: DispatcherFromFile.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILE_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILE_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H
#include "DispatcherImpl.h"
#endif

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherFromFile: public DispatcherImpl
{
  std::string const m_file;

public:
  DispatcherFromFile(std::string const& file);
  void run(common::task::PipeWriteEnd<pipe_value_type>& write_end);
};

} // server
} // manager
} // wms
} // glite 

#endif

// Local Variables:
// mode: c++
// End:

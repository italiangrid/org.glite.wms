/*
 * File: classad-plugin-loader.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#define ENABLE_SHARED_LIBRARY_FUNCTIONS

#include "classad_plugin_loader.h"
#include <classad_distribution.h>
#include <fnCall.h>

namespace glite {
namespace wms {
namespace classad_plugin {

namespace {
classad::ClassAdParser f_parser;
boost::mutex f_mtx;
}

init::init(std::string const& name)	
{
  boost::mutex::scoped_lock lk(f_mtx);
  classad::FunctionCall::RegisterSharedLibraryFunctions("libglite_wms_"+name+"classad_plugin.so");
}

} // namespace classad_plugin
} // namespace wms
} // namespace glite

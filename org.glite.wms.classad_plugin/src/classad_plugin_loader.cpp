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

boost::mutex	classad_plugin_loader::mtx;
int 		    classad_plugin_loader::count;

classad_plugin_loader::classad_plugin_loader()	
{
  boost::mutex::scoped_lock lk(mtx);
  if (count++ == 0) {
	  // TODO: change code to allow reading of plugin modules to be loaded froma configuration file	
	  classad::ClassAdParser parser;
          classad::FunctionCall::RegisterSharedLibraryFunctions("libglite_wms_gangmatch_classad_plugin.so");
  }
}

classad_plugin_loader::~classad_plugin_loader()
{
  boost::mutex::scoped_lock lk(mtx);
  if (--count == 0) {
  
	  // the classad library does not provide any function to
	  // unregister previous registered functions..
  }
}

} // namespace classad_plugin
} // namespace wms
} // namespace glite

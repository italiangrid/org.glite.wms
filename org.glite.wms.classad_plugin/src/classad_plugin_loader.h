/*
 * File: classad-plugin-loader.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>


namespace glite {
namespace wms {
namespace classad_plugin {

class classad_plugin_loader : boost::noncopyable
{
  static boost::mutex   mtx;
  static int count;
		
  public:
	classad_plugin_loader();
        ~classad_plugin_loader();
};

namespace { classad_plugin_loader init; }

} // namespace classad_plugin
} // namespace wms
} // namespace glite

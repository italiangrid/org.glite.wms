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

// FIXME: the following global instantiation is commented out because,
// FIXME: if this creator is invoked -before- the creator for
// FIXME: the global classad::FunctionCall::functionTable, it will
// FIXME: cause a SEGV while trying to access the non-initialized 
// FIXME: functionTable. For the time being this was moved to the NS main
// FIXME: and to the broker Helper.cpp. This can come back when the
// FIXME: matchmaker is finally done only via the broker_helper DL.
// namespace { classad_plugin_loader init; }

} // namespace classad_plugin
} // namespace wms
} // namespace glite

/*
 * File: classad-plugin-loader.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include <string>

namespace glite {
namespace wms {
namespace classad_plugin {

class init : boost::noncopyable
{
  explicit init(std::string const& name);
};

} // namespace classad_plugin
} // namespace wms
} // namespace glite

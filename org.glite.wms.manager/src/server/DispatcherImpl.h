// File: DispatcherImpl.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#include "pipedefs.h"
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class DispatcherImpl: boost::noncopyable
{
public:
  DispatcherImpl();
  virtual ~DispatcherImpl();

  virtual void run(common::task::PipeWriteEnd<pipe_value_type>& write_end) = 0;
};

} // server
} // manager
} // wms
} // glite

#endif // GLITE_WMS_MANAGER_SERVER_DISPATCHERIMPL_H

// File: pipedefs.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#define GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif
#ifndef GLITE_WMS_X_BOOST_SHAREDPTR_HPP
#define GLITE_WMS_X_BOOST_SHAREDPTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef GLITE_WMS_X_BOOST_FUNCTION_HPP
#define GLITE_WMS_X_BOOST_FUNCTION_HPP
#include <boost/function.hpp>
#endif
#ifndef GLITE_WMS_COMMON_TASK_TASK_H
#include "glite/wms/common/task/Task.h"
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace task = glite::wms::common::task;

typedef boost::function0<void> PostProcessFunction;
typedef boost::shared_ptr<classad::ClassAd> ClassAdPtr;
typedef std::pair<PostProcessFunction, ClassAdPtr> pipe_value_type;
typedef task::Pipe<pipe_value_type> pipe_type;

} // server
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:

// File: dispatching_utils.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHING_UTILS_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHING_UTILS_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#include "pipedefs.h"
#endif
#ifndef GLITE_WMS_COMMON_TASK_TASK_H
#include "glite/wms/common/task/Task.h"
#endif

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace task = glite::wms::common::task;

typedef std::string dispatcher_type;

enum PPResult {
  INVALID_REQUEST,
  NOTHING_TO_DO,
  FORWARD_TO_WM,
  QUIT
};

boost::mutex& submit_cancel_mutex();

PPResult preprocess_submit(ClassAdPtr command_ad);
PPResult preprocess_resubmit(ClassAdPtr command_ad);
PPResult preprocess_cancel(ClassAdPtr command_ad);
PPResult preprocess_quit(ClassAdPtr command_ad);
PPResult preprocess(ClassAdPtr command_ad);

class InvalidRequest: public std::exception
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;
public:
  InvalidRequest(std::string const& s);
  ~InvalidRequest() throw();
  std::string str() const;
  char const* what() const throw();
};

bool process(
  std::string const& ad_str,
  PostProcessFunction f,
  task::PipeWriteEnd<pipe_value_type>& write_end
);



} // server
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:

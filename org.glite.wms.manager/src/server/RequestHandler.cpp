// File: RequestHandler.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "RequestHandler.h"

#include <string>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/xtime.hpp>
#include <classad_distribution.h>

#include "../common/WorkloadManager.h"
#include "../common/CommandAdManipulation.h"

#include "signal_handling.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/wmsutils/exception/Exception.h"

#include "glite/wmsutils/common/utilities/classad_utils.h"

namespace jobid = glite::wmsutils::jobid;
namespace task = glite::wms::common::task;

namespace glite {
namespace wms {
namespace manager {
namespace server {

class RequestHandler::Impl 
{
  WorkloadManager m_wm;

public:
  void run(task::PipeReadEnd<pipe_value_type>& read_end);
};

void
RequestHandler::Impl::run(task::PipeReadEnd<pipe_value_type>& read_end)
try {

  Info("RequestHandler: starting");

  while (true) {
    if (received_quit_signal()) {
      Debug("RequestHandler: asked to quit");
      break;
    }
    PostProcessFunction post_process;
    ClassAdPtr command_ad;
    boost::tie(post_process, command_ad) = read_end.read();

    std::string command(command_get_command(*command_ad));
 
    if (command == "jobsubmit") {

      m_wm.submit(submit_command_get_ad(*command_ad));

    } else if (command == "jobresubmit") {

      m_wm.resubmit(jobid::JobId(resubmit_command_get_id(*command_ad)));

    } else if (command == "jobcancel") {

      m_wm.cancel(jobid::JobId(cancel_command_get_id(*command_ad)));

    } else {
      Error("Invalid command (" << command << ")");
    }

    Debug("postprocessing");
    post_process();
  }

  Info("RequestHandler: exiting");

} catch (task::Eof&) {
  Info("RequestHandler: End of input. Exiting...");
} catch (std::exception const& e) {
  Error("RequestHandler: " << e.what() << ". Exiting...");
} catch (...) {
  Error("RequestHandler: caught unknown exception. Exiting...");
}

RequestHandler::RequestHandler()
  : m_impl(new Impl)
{
}

RequestHandler::~RequestHandler()
{
  delete m_impl;
}

void
RequestHandler::run()
{
  m_impl->run(read_end());
}

} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

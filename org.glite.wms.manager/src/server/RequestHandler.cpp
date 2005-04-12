// File: RequestHandler.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "RequestHandler.h"
#include <string>
#include <ctime>
#include <classad_distribution.h>
#include "WorkloadManager.h"
#include "CommandAdManipulation.h"
#include "signal_handling.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/helper/exceptions.h"
#include "Request.hpp"
#include "TaskQueue.hpp"
#include "listmatch.h"

namespace jobid = glite::wmsutils::jobid;
namespace task = glite::wms::common::task;
namespace common = glite::wms::manager::common;
namespace exception = glite::wmsutils::exception;
namespace requestad = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace server {

struct RequestHandler::Impl
{
  common::WorkloadManager m_wm;
};

RequestHandler::RequestHandler()
  : m_impl(new Impl)
{
}

void
RequestHandler::operator()()
try {

  Info("RequestHandler: starting");

  common::WorkloadManager& wm = m_impl->m_wm;

  while (!received_quit_signal()) {

    RequestPtr req(read_end().read());
    req->last_processed(std::time(0));

    try {

      if (req->jdl() && !req->marked_cancelled()) {
        req->state(Request::PROCESSING);
        
        if (req->marked_match()) {
          std::string filename;
          int number;
          bool include_brokerinfo;
          boost::tie(filename, number, include_brokerinfo) = req->match_parameters();
           
          Info(
            "considering match " << req->id()
            << ' ' << filename
            << ' ' << number
            << ' ' << include_brokerinfo
          );

          if (!match(*req->jdl(), filename, number, include_brokerinfo)) {
             Info("Failed match for " << req->id());
          }
        } else {
           Info("considering (re)submit of " << req->id());
           wm.submit(req->jdl());
        }
        
        req->state(Request::DELIVERED);

      } else {
        Info("considering cancel of " << req->id());

        wm.cancel(req->id());
        req->state(Request::CANCELLED);

      }


    } catch (helper::HelperError const& e) {
      req->state(Request::RECOVERABLE, e.what());
    } catch (std::invalid_argument const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    } catch (requestad::ManipulationException const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    } catch (exception::Exception const& e) {
      req->state(Request::UNRECOVERABLE, e.what());
    }

  }

  Info("RequestHandler: exiting");

} catch (task::Eof&) {
  Info("RequestHandler: End of input. Exiting...");
} catch (std::exception const& e) {
  Error("RequestHandler: " << e.what() << ". Exiting...");
} catch (...) {
  Error("RequestHandler: caught unknown exception. Exiting...");
}

}}}} // glite::wms::manager::server

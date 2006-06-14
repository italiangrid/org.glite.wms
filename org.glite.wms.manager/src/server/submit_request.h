// File: Request.hpp
// Author: Francesco Giacomini
// Copyright (c) Members of the EGEE Collaboration 2004
// For license conditions see http://public.eu-egee.org/license/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_SUBMIT_REQUEST_H
#define GLITE_WMS_MANAGER_SERVER_SUBMIT_REQUEST_H

#include <classad_distribution.h>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include "glite/wmsutils/jobid/JobId.h"
#include "lb_utils.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

boost::tuple<
  std::string,                  // command
  jobid::JobId,                 // jobid
  std::string,                  // sequence code
  std::string                   // x509_proxy
>
parse_request(classad::ClassAd const& command_ad);

class Submit
{
public:
  enum State {
    WAITING          = 0x01,
    READY            = 0x02,
    PROCESSING       = 0x04,
    RECOVERABLE      = 0x08,
    UNRECOVERABLE    = 0x10,
    DELIVERED        = 0x20,
    CANCELLED        = 0x40,
    CANCEL_DELIVERED = 0x80
  };

public:
  struct Pimpl;
  boost::shared_ptr<pimpl> m_pimpl;

  Submit(classad::ClassAd& command_ad,
    std::string const& command,
    jobid::JobId const& id,
    boost::function<void()> const& cleanup
  );
  ~Submit();
  glite::wmsutils::jobid::JobId id() const { return m_id; }
  void state(State s, std::string const& message = std::string());
  State state() const { return m_state; }
  std::string message() const { return m_message; }
  void last_processed(time_t t) { m_last_processed = t; }
  std::time_t last_processed() const { return m_last_processed; }
  ContextPtr lb_context() const { return m_lb_context; }

  // what about?
  // classad::ClassAd const& jdl() const { return *m_jdl; }

  classad::ClassAd const* jdl() const { return m_jdl.get(); }
  void jdl(std::auto_ptr<classad::ClassAd> jdl);
  void clear_jdl() { m_jdl.reset(); }

  void mark_cancelled(ContextPtr cancel_context);
  bool marked_cancelled() const { return m_cancel_context; }
  ContextPtr cancel_context() const { return m_cancel_context; }

  void mark_resubmitted() { m_resubmitted = true; }
  bool marked_resubmitted() const { return m_resubmitted; }

  bool marked_match() const { return !m_match_parameters.get<0>().empty(); }
  boost::tuple<std::string, int, bool> match_parameters() const;
  void add_cleanup(boost::function<void()> const& cleanup);

  std::time_t expiry_time() const;

};

}}}} // glite::wms::manager::server

#endif

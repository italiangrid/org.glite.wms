// File: WMReal.tcc
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <classad_distribution.h>

#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/helper/exceptions.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wmsutils/exception/Exception.h"

#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/lb/producer.h"
#include "glite/lb/consumer.h"

#include "glite/security/proxyrenewal/renewal.h"

#include "purger.h"

#include "glite/wms/common/utilities/scope_guard.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace configuration = glite::wms::common::configuration;
namespace utilities = glite::wms::common::utilities;
namespace common = glite::wms::manager::common;
namespace jdl = glite::wms::jdl;
namespace purger = glite::wms::purger;

namespace {

int get_max_retry_count()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();

  if (!config) {
    Fatal("empty or invalid configuration");
  }
  configuration::WMConfiguration const* const wm_config = config->wm();
  if (!wm_config) {
    Fatal("empty WM configuration");
  }

  return wm_config->max_retry_count();
}

} // {anonymous}

template<typename PlanningP, typename DeliveryP, typename CancellingP>
WMReal<PlanningP, DeliveryP, CancellingP>::WMReal()
{
}

template<typename PlanningP, typename DeliveryP, typename CancellingP>
WMReal<PlanningP, DeliveryP, CancellingP>::~WMReal()
{
}

template<typename PlanningP, typename DeliveryP, typename CancellingP>
void
WMReal<PlanningP, DeliveryP, CancellingP>::submit(classad::ClassAd const* request_ad_p)
{
  if (!request_ad_p) {
    Error("request ad is null");
    return;
  }

  // make a copy because we change the sequence code, see below
  classad::ClassAd request_ad(*request_ad_p);

  wmsutils::jobid::JobId jobid;

  try {

    wmsutils::jobid::JobId tmp_jobid(jdl::get_edg_jobid(request_ad));
    jobid = tmp_jobid;

  } catch (jdl::CannotGetAttribute const& e) {

    Error(e.what() << " for " << utilities::unparse_classad(request_ad));
    return;

  } catch (wmsutils::jobid::JobIdException const& e) {

    Error(e.what() << " for " << utilities::unparse_classad(request_ad));
    return;

  }

  common::ContextPtr context_ptr = common::get_context(jobid);
  if (!context_ptr) {
    Info("LB context not available for " << jobid << "(job already cancelled?)");
    return;
  }
  edg_wll_Context context = *context_ptr;

  // keep the storage guard outside the try block, because the lb logs in the
  // catch blocks require the proxy, which lives in the storage
  utilities::scope_guard storage_guard(
    boost::bind(purger::purgeStorage, jobid, std::string(""))
  );

  try {

    utilities::scope_guard proxy_guard(
      boost::bind(edg_wlpr_UnregisterProxy, jobid, static_cast<char const*>(0))
    );

    utilities::scope_guard context_guard(boost::bind(common::unregister_context, jobid));

    // update the sequence code before doing the planning
    // some helpers may need a more recent sequence code than what appears in
    // the classad originally received

    char* c_sequence_code = edg_wll_GetSequenceCode(context);
    std::string sequence_code(c_sequence_code);
    free(c_sequence_code);

    jdl::remove_lb_sequence_code(request_ad);
    jdl::set_lb_sequence_code(request_ad, sequence_code);

    boost::scoped_ptr<classad::ClassAd> planned_ad(Plan(request_ad));

    char const* const c_ce_id = jdl::get_ce_id(*planned_ad).c_str();
    int lb_error = edg_wll_LogMatch(context, c_ce_id); 
    if (lb_error != 0) {
      Warning("edg_wll_LogMatch failed for " << jobid
              << " (" << common::get_lb_message(context) << ")");
    }

    Deliver(*planned_ad);

    context_guard.dismiss();
    proxy_guard.dismiss();

  } catch (jdl::ManipulationException const& e) {

    std::string reason("Cannot plan: ");
    reason += e.what();
    Error(reason << " for " << jobid);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }
    return;

  } catch (helper::HelperError& e) {

    std::string reason("Cannot plan: ");
    reason += e.what();
    Error(reason << " for " << jobid);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }
    return;

  } catch (std::invalid_argument const& e) {

    std::string reason("Cannot plan: ");
    reason += e.what();
    Error(reason << " for " << jobid);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }
    return;

    // better: keep the request pending for a while in a
    // std::vector<std::pair<time_type, jdl, retry_count>
    // and log:
    //  edg_wll_LogPending(context, reason);
    //  edg_wll_Enqueue(context, queue_name, jdl, "OK", "");
    // or resubmit()?
  }

  storage_guard.dismiss();
}

template<typename PlanningP, typename DeliveryP, typename CancellingP>
void
WMReal<PlanningP, DeliveryP, CancellingP>::resubmit(wmsutils::jobid::JobId const& request_id)
{
  common::ContextPtr context_ptr = common::get_context(request_id);
  if (!context_ptr) {
    Info("LB context not available for " << request_id << "(job already cancelled?)");
    return;
  }
  edg_wll_Context context = *context_ptr;

  // keep the storage guard outside the try block, because the lb logs in the
  // catch blocks require the proxy, which lives in the storage
  utilities::scope_guard storage_guard(
    boost::bind(purger::purgeStorage, request_id, std::string(""))
  );

  int lb_error;

  try {

    utilities::scope_guard proxy_guard(
      boost::bind(edg_wlpr_UnregisterProxy, request_id, static_cast<char const*>(0))
    );

    utilities::scope_guard context_guard(boost::bind(common::unregister_context, request_id));


    // flush the lb events since we'll query the server
    struct timeval* timeout = 0;
    lb_error = edg_wll_LogFlush(context, timeout);
    if (lb_error != 0) {
      Warning("edg_wll_LogFlush failed for " << request_id
              << " (" << common::get_lb_message(context) << ")");
    }

    // retrieve previous matches; continue if failure
    std::vector<std::pair<std::string,int> > const previous_matches_ex
      = common::get_previous_matches_ex(context, request_id);

    if (previous_matches_ex.empty()) {
      std::ostringstream os;
      os << "cannot retrieve previous matches for " << request_id;
      std::string reason = os.str();
      Warning(reason);
    }

    std::vector<std::string> previous_matches;
    for (std::vector<std::pair<std::string,int> >::const_iterator it = previous_matches_ex.begin();
         it != previous_matches_ex.end(); ++it) {
      previous_matches.push_back(it->first);
    }

    // check the system max retry count; abort if exceeded
    int max_retry_count = get_max_retry_count();
    if (max_retry_count <= 0
        || previous_matches.size() > static_cast<unsigned>(max_retry_count)) {
      std::ostringstream os;
      os << "MaxRetryCount (" << max_retry_count << ") hit for " << request_id;
      std::string reason = os.str();
      Info(reason);
      int err; 
      common::ContextPtr ctx;
      boost::tie(err, ctx) = lb_log(
        boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
        context_ptr
      );
      if (err) {
        Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
      }

      return;
    }

    // retrieve original jdl
    std::string const job_ad_str = common::get_original_jdl(context, request_id);
    boost::scoped_ptr<classad::ClassAd> job_ad(utilities::parse_classad(job_ad_str));

    // check the job max retry count; abort if exceeded
    bool count_valid = false;
    int job_retry_count = jdl::get_retry_count(*job_ad, count_valid);
    if (!count_valid) {
      job_retry_count = 0;
    }
    if (job_retry_count <= 0
        || previous_matches.size() > static_cast<unsigned>(job_retry_count)) {
      std::ostringstream os;
      os << "Job RetryCount (" << job_retry_count << ") hit";
      std::string reason = os.str();
      Info(reason << " for " << request_id);
      int err; 
      common::ContextPtr ctx;
      boost::tie(err, ctx) = lb_log(
        boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
        context_ptr
      );
      if (err) {
        Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
      }

      return;
    }

    // in practice the actual retry limit is
    // max(0, min(job_retry_count, max_retry_count))

    // do the planning/delivery; abort if failure
    jdl::set_edg_previous_matches(*job_ad, previous_matches);
    jdl::set_edg_previous_matches_ex(*job_ad, previous_matches_ex);

    boost::scoped_ptr<classad::ClassAd> planned_ad(Plan(*job_ad));

    char const* const c_ce_id = jdl::get_ce_id(*planned_ad).c_str();
    lb_error = edg_wll_LogMatch(context, c_ce_id);
    if (lb_error != 0) {
      Warning("edg_wll_LogMatch failed for " << request_id
              << " (" << common::get_lb_message(context) << ")");
    }

    Deliver(*planned_ad);

    context_guard.dismiss();
    proxy_guard.dismiss();

  } catch (utilities::CannotParseClassAd const& e) {

    std::string reason;
    if (e.str().empty()) {
      reason = "cannot retrieve original JDL";
    } else {
      reason = "original JDL is not a classad";
    }
    Error(reason << " for " << request_id);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }

    return;

  } catch (jdl::ManipulationException const& e) {

    std::string reason(e.what());
    Error(reason << " for " << request_id);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }

    return;

  } catch (helper::HelperError& e) {

    std::string reason("Cannot plan: ");
    reason += e.what();
    Error(reason << " for " << request_id);
    int err; 
    common::ContextPtr ctx;
    boost::tie(err, ctx) = lb_log(
      boost::bind(edg_wll_LogAbort, _1, reason.c_str()),
      context_ptr
    );
    if (err) {
      Warning(get_logger_message("edg_wll_LogAbort", err, context_ptr, ctx));
    }

    return;
  }

  storage_guard.dismiss();
}

template<typename PlanningP, typename DeliveryP, typename CancellingP>
void
WMReal<PlanningP, DeliveryP, CancellingP>::cancel(wmsutils::jobid::JobId const& request_id)
{
  Cancel(request_id);
}

} // server
} // manager
} // wms
} // glite 

// Local Variables:
// mode: c++
// End:

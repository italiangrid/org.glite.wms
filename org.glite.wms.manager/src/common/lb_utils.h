// File: lb_utils.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_LB_UTILS_H
#define GLITE_WMS_MANAGER_COMMON_LB_UTILS_H

#ifndef GLITE_WMS_X_VECTOR
#define GLITE_WMS_X_VECTOR
#include <vector>
#endif
#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif
#ifndef GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#define GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef GLITE_LB_CONTEXT_H
#include "glite/lb/context.h"
#endif

#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>

namespace glite {

namespace wmsutils {
namespace jobid {
class JobId;
} // jobid
} // wmsutils

namespace wms {
namespace manager {
namespace common {

class lb_context_adapter
{
  edg_wll_Context m_context;

public:
  explicit lb_context_adapter(edg_wll_Context const& ctx): m_context(ctx)
  {}
  ~lb_context_adapter()
  {
    edg_wll_FreeContext(m_context);
  }
  operator edg_wll_Context() const { return m_context; }
};

typedef boost::shared_ptr<lb_context_adapter> ContextPtr;

ContextPtr
create_context(
  wmsutils::jobid::JobId const& id,
  std::string const& x509_proxy,
  std::string const& sequence_code
);
bool register_context(wmsutils::jobid::JobId const& id, ContextPtr context);
bool unregister_context(wmsutils::jobid::JobId const& id);
ContextPtr get_context(wmsutils::jobid::JobId const& id);

std::vector<std::string> get_previous_matches(edg_wll_Context context, wmsutils::jobid::JobId const& id);

// <ce id, timestamp in seconds>
std::vector<std::pair<std::string,int> > get_previous_matches_ex(edg_wll_Context context, wmsutils::jobid::JobId const& id);
std::string get_original_jdl(edg_wll_Context context, wmsutils::jobid::JobId const& id);

std::string get_user_x509_proxy(wmsutils::jobid::JobId const& jobid);
std::string get_host_x509_proxy();

std::string get_lb_message(edg_wll_Context context);
std::string get_lb_message(ContextPtr context_ptr);

boost::tuple<int,ContextPtr>
lb_log(boost::function<int(edg_wll_Context)> log_f, ContextPtr user_context);

std::string
get_logger_message(
  std::string const& function_name,
  int error,
  ContextPtr user_context,
  ContextPtr last_context
);

} // common
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:

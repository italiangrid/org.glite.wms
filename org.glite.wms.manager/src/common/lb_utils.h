// File: lb_utils.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_LB_UTILS_H
#define GLITE_WMS_MANAGER_COMMON_LB_UTILS_H

#include <vector>
#include <string>

#ifndef GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#define GLITE_WMS_X_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif

#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#ifndef GLITE_LB_CONTEXT_H
#include "glite/lb/context.h"
#endif

namespace glite {

namespace wmsutils {
namespace jobid {
class JobId;
}}

namespace wms {
namespace manager {
namespace common {

typedef boost::shared_ptr<boost::remove_pointer<edg_wll_Context>::type> ContextPtr;

ContextPtr
create_context(
  wmsutils::jobid::JobId const& id,
  std::string const& x509_proxy,
  std::string const& sequence_code
);

// <ce id, timestamp in seconds>
std::vector<std::pair<std::string,int> >
get_previous_matches(edg_wll_Context context, wmsutils::jobid::JobId const& id);

std::string get_original_jdl(edg_wll_Context context, wmsutils::jobid::JobId const& id);

std::string get_user_x509_proxy(wmsutils::jobid::JobId const& jobid);
std::string get_host_x509_proxy();

std::string get_lb_message(ContextPtr const& context_ptr);
std::string get_lb_sequence_code(ContextPtr const& context_ptr);

boost::tuple<int, ContextPtr>
lb_log(boost::function<int(edg_wll_Context)> log_f, ContextPtr user_context);

std::string
get_logger_message(
  std::string const& function_name,
  int error,
  ContextPtr user_context,
  ContextPtr last_context
);

}}}} // glite::wms::manager::common

#endif

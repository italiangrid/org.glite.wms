// File: WMImpl.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_WMIMPL_H
#define GLITE_WMS_MANAGER_COMMON_WMIMPL_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif

namespace classad {
class ClassAd;
}

namespace glite {

namespace wmsutils {
namespace jobid {
class JobId;
}}

namespace wms {
namespace manager {
namespace common {


class WMImpl: boost::noncopyable
{
public:
  WMImpl();
  virtual ~WMImpl();

  virtual void submit(classad::ClassAd const* request_ad) = 0;
  virtual void cancel(wmsutils::jobid::JobId const& request_id) = 0;
};

}}}} // glite::wms::manager::common

#endif // GLITE_WMS_MANAGER_COMMON_WMIMPL_H


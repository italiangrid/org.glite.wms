// File: WorkloadManager.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_WORKLOADMANAGER_H
#define GLITE_WMS_MANAGER_COMMON_WORKLOADMANAGER_H

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

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

class WMImpl;

/**
 * The Workload Manager service
 *
 * The WorkloadManager class represents the access point to the workload
 * management service, intended as scheduling plus reliable job management.
 */
class WorkloadManager: boost::noncopyable
{
  boost::scoped_ptr<WMImpl> m_impl;

public:
  /**
   * Constructor: chooses the right implementation for the current environment.
   *
   * The default constructor chooses the right implementation of the
   * WorkloadManager depending on the current environment. Information may be
   * gathered from the configuration, the process environment, etc.
   */
  WorkloadManager();

  // the dtor is necessary even if it doesn't do anything, because otherwise it
  // is generated automatically inline (?) calling the dtor of m_impl, that
  // requires that the definition of WMImpl is available (now it is a forward
  // declaration) 
  ~WorkloadManager();

  /** 
   * Submit a request to the Workload Manager
   * 
   * The request, if valid, is subject to:
   * -# some planning
   * -# delivering to the resource that is able to satisfy it
   *
   * @param request_ad description of the request
   */
  void submit(classad::ClassAd const* request_ad);

  /** 
   * Cancel a request 
   * 
   * @param request_id identifier of the request to be aborted. 
   */
  void cancel(wmsutils::jobid::JobId const& request_id);

};

}}}} // glite::wms::manager::common

#endif // GLITE_WMS_MANAGER_COMMON_WORKLOADMANAGER_H


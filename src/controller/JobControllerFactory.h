#ifndef EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERFACTORY_H
#define EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERFACTORY_H

// File: JobControllerFactory.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <memory>

#include <classad_distribution.h>

#include <boost/shared_ptr.hpp>

#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/wms/common/utilities/jobdir.h"

#include "jobcontrol_namespace.h"

typedef  struct _edg_wll_Context  *edg_wll_Context;

JOBCONTROL_NAMESPACE_BEGIN {

namespace utilities = glite::wms::common::utilities;

namespace controller {

class JobControllerImpl;
class JobControllerClientImpl;

class Empty {};

class JobControllerFactory {
  friend class Empty;

public:
  JobControllerFactory();

  JobControllerImpl *create_server( edg_wll_Context *cont );
  JobControllerClientImpl *create_client( void );
  static JobControllerFactory *instance( void );
  void createQueue();

private:
  boost::shared_ptr<utilities::FileListMutex>               jcf_mutex;
  boost::shared_ptr<utilities::FileList<classad::ClassAd> > jcf_queue;
  boost::shared_ptr<utilities::JobDir>                      jcf_jobdir;

  static JobControllerFactory *jcf_s_instance;
};

} // namespace controller

} JOBCONTROL_NAMESPACE_END

#endif /* EDG_WORKLOAD_JOBCONTROL_CONTROLLER_JOBCONTROLLERFACTORY_H */

// Local Variables:
// mode: c++
// End:

// File: JCCancellingPolicy.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "JCCancellingPolicy.h"

#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/wms/jobcontrol/controller/JobController.h"

namespace jobid = glite::wmsutils::jobid;

namespace glite {
namespace wms {
namespace manager {
namespace server {

JCCancellingPolicy::~JCCancellingPolicy()
{
}

void JCCancellingPolicy::Cancel(wmsutils::jobid::JobId const& id)
{
  Debug("cancelling job " << id);

  ContextPtr context_ptr = get_context(id);
  assert(context_ptr);
  assert(unregister_context(id));
  edg_wll_Context context = *context_ptr;
  jobcontrol::controller::JobController(&context).cancel(id);
}

} // server
} // manager
} // wms
} // glite


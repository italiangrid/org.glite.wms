// File: RequestPlanningPolicy.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SEVER_REQUESTPLANNINGPOLICY_H
#define GLITE_WMS_MANAGER_SEVER_REQUESTPLANNINGPOLICY_H

#include <classad_distribution.h>

#include "glite/wms/helper/Request.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

class RequestPlanningPolicy
{
protected:
  ~RequestPlanningPolicy() {}

public:
  static classad::ClassAd* Plan(classad::ClassAd const& ad)
  {
    helper::Request request(&ad);

    while (!request.is_resolved()) {
      request.resolve();
    }

    classad::ClassAd const* res_ad = resolved_ad(request);
    return res_ad ? new classad::ClassAd(*res_ad) : 0;
  }
};

typedef RequestPlanningPolicy PlanningPolicy;

} // server
} // manager
} // wms
} // glite

#endif // GLITE_WMS_MANAGER_SERVER_REQUESTPLANNINGPOLICY_H

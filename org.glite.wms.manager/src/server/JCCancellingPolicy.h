// File: JCCancellingPolicy.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_JCCANCELLINGPOLICY_H
#define GLITE_WMS_MANAGER_SERVER_JCCANCELLINGPOLICY_H

#include <string>

namespace glite {

namespace wmsutils {
namespace jobid {
class JobId;
}}

namespace wms {
namespace manager {
namespace server {

class JCCancellingPolicy
{
protected:
  ~JCCancellingPolicy();

public:
  static void Cancel(wmsutils::jobid::JobId const& id);
  
};

typedef JCCancellingPolicy CancellingPolicy;

} // server
} // manager
} // wms
} // glite 

#endif // GLITE_WMS_MANAGER_SERVER_JCCANCELLINGPOLICY_H

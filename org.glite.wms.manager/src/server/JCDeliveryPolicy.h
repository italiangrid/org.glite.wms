// File: JCDeliveryPolicy.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DELIVERYPOLICY_H
#define GLITE_WMS_MANAGER_SERVER_DELIVERYPOLICY_H

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

class JCDeliveryPolicy
{
protected:
  ~JCDeliveryPolicy();

public:
  static void Deliver(classad::ClassAd const& ad);
};

typedef JCDeliveryPolicy DeliveryPolicy;

} // server
} // manager
} // wms
} // glite

#else
#error Multiple delivery policies defined
#endif // GLITE_WMS_MANAGER_SERVER_DELIVERYPOLICY_H

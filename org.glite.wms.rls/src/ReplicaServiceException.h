/***************************************************************************
 *  filename  : ReplicaServiceException.h
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

// $Id$

#ifndef GLIDE_WMS_RLS_REPLICASERVICEEXCEPTION_H
#define GLIDE_WMS_RLS_REPLICASERVICEEXCEPTION_H

#include <string>

namespace glite {
namespace wms {
namespace rls {

class ReplicaServiceException  {

  std::string what;
public:
  ReplicaServiceException(const std::string& str);
  ~ReplicaServiceException();

  const std::string reason() const;
};


} // namespace rls
} // namespace wms
} // namespace glite

#endif

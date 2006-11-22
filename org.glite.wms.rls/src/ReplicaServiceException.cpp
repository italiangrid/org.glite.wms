/***************************************************************************
 *  filename  : ReplicaServiceException.cpp
 *  authors   : Enzo Martelli <enzo.martelli@ct.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include "ReplicaServiceException.h"

using namespace std;

namespace glite {
namespace wms {
namespace rls {


ReplicaServiceException::ReplicaServiceException(const string& str) 
            : what(str)
{
}

ReplicaServiceException::~ReplicaServiceException()
{
}

const string
ReplicaServiceException::reason() const 
{
   return what;
}


} // namespace rls
} // namespace wms
} // namespace glite

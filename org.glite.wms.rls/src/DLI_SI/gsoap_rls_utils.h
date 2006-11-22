/***************************************************************************
 *  filename  : gsoap_rls_utils.h
 *  authors   : Enzo Martelli <enzo.martelli@ct.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <stdsoap2.h>


namespace glite {
namespace wms {
namespace rls {


const string 
exception_reason(struct soap& m_soap);


} // namespace rls
} // namespace wms
} // namespace glite

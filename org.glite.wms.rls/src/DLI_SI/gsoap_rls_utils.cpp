/***************************************************************************
 *  filename  : gsoap_rls_utils.cpp
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
exception_reason(struct soap& m_soap) {

  std::string ex;
  if( m_soap.error ) {

    soap_set_fault( & m_soap );


    const char** faultdetail_ptr = soap_faultdetail( & m_soap );
    std::string faultdetail;
    if ( *faultdetail_ptr != NULL ) faultdetail = *faultdetail_ptr;
       else faultdetail = "unknown";


    const char** faultcode_ptr = soap_faultcode( & m_soap );
    std::string faultcode;
    if ( *faultcode_ptr != NULL ) faultcode = *faultcode_ptr;
       else faultcode = "unknown";


    const char** faultstring_ptr = soap_faultstring( & m_soap );
    std::string faultstring;
    if (*faultstring_ptr != NULL ) faultstring = *faultstring_ptr;
       else faultstring = "unknown";


    std::string SOAP_FAULT_CODE = "SOAP_FAULT_CODE: ";
    std::string SOAP_FAULT_STRING = "SOAP_FAULT_STRING: ";
    std::string SOAP_FAULT_DETAIL = "SOAP_FAULT_DETAIL: ";
    std::string new_line = "\n";
                                                                                                 
    ex = new_line + SOAP_FAULT_CODE + faultcode + new_line +
                    SOAP_FAULT_STRING +  faultstring + new_line +
                    SOAP_FAULT_DETAIL + faultdetail + new_line;
  }
  else {
    ex = "Error in soap request towards the catalog. Unknown error.";
  }
  return ex;

}



} // namespace rls
} // namespace wms
} // namespace glite

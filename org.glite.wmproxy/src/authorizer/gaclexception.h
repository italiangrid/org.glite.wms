#ifndef GLITE_WMS_WMPROXY_AUTHORIZER_GACLEXCEPTION_H
#define GLITE_WMS_WMPROXY_AUTHORIZER_GACLEXCEPTION_H

#include "glite/wmsutils/exception/Exception.h"


/**
 * GaclException
 * This Exception is thrown when a error occurs using gacl files
 * @ingroup Common
 * @version 0.1
 * @date 
 * @author Marco Sottilaro <marco.sottilaro@datamat.it>
*/

class GaclException : public glite::wmsutils::exception::Exception {
public:


   GaclException (std::string file,
                                   int line,
                                  std::string method,
                                   int code,
                                  std::string exception_name) ;
  virtual ~GaclException() throw () {};
  std::string          error_description ;

};
//End CLass  GaclException

#endif

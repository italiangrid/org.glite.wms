
#include "gaclexception.h"
#include "glite/wmsutils/exception/exception_codes.h"

using namespace std ;

/*
namespace glite {
namespace wms {
namespace jdl {
*/


GaclException::GaclException (string file,
                                   int line,
                                   string method,
                                   int code,
                                   string message ):
   Exception(file,line,method,code , "GaclException" ){

   		char ln[64];
		sprintf ( ln, "%d", line);
		error_message = "GaclException : " + message + "\n";
		error_message += "at  " + file;
		error_message += " (" +method + ") line:" + ln;

   };


/*// jdl namespace
} // wms namespace
} // glite namespace
*/

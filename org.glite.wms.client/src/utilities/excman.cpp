// HEADER
#include "excman.h"
using namespace std ;
namespace glite{namespace wms{namespace client{
WmsClientException::WmsClientException (const string& file, int line, const string& method,
				int code,const string& exception_name,const string& error):
				Exception(file,line,method,code,exception_name){
	error_message=error;
}; // End Class WmsClientException
const std::string errMsg(severity sev,glite::wmsutils::exception::Exception& exc, bool debug){
	string result = "**** ";
	// Add the severity Message
	switch (sev){
		case WMS_WARNING:
		result+="Warning: ";
		break;
		case WMS_ERROR:
		result+="Error: ";
		break;
		case WMS_FATAL:
		result+="Fatal Error: ";
		break;
		default:
		break;
	}
	// Add the Exception Name
	result+=exc.getExceptionName();
	// Add the message itself
	if (debug) {result+=exc.printStackTrace();}
	else {result+=string(exc.what());}
	return result;
}
}}} // ending namespaces

// HEADER
#include "excman.h"
using namespace std ;
namespace glite{namespace wms{namespace client{namespace utilities{
WmsClientException::WmsClientException (const string& file, int line, const string& method,
				int code,const string& exception_name,const string& error):
				Exception(file,line,method,code,exception_name){
	error_message=error;
}; // End Class WmsClientException

const std::string errMsg(severity sev,glite::wmsutils::exception::Exception& exc, bool debug){
	return errMsg( sev,exc.getExceptionName(),debug?exc.printStackTrace():string(exc.what()), debug);
}
const std::string errMsg(severity sev,const string& title, const string& err, bool debug){
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
	result+=title+" ****\n"+err +"\n";
	return result;
};
}}}} // ending namespaces

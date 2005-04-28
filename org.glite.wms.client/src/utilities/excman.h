#ifndef GLITE_WMS_CLIENT_EXCMAN
#define GLITE_WMS_CLIENT_EXCMAN
/*
 * excman.h
 */
#include "glite/wmsutils/exception/Exception.h"
namespace glite{namespace wms{namespace client{

enum severity{
	WMS_NONE,
	WMS_WARNING,
	WMS_ERROR,
	WMS_FATAL
};
class WmsClientException: public glite::wmsutils::exception::Exception {
public:
	   WmsClientException(const std::string& file, int line,const std::string& method,
	   		int code,const std::string& exception_name, const std::string& error);
};
// Static methods:

const std::string errMsg(severity sev,glite::wmsutils::exception::Exception& exc, bool debug=false);
}}} // ending namespaces

#endif


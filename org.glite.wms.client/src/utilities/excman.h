#ifndef GLITE_WMS_CLIENT_EXCMAN
#define GLITE_WMS_CLIENT_EXCMAN
/*
 * excman.h
 */
#include "glite/wmsutils/exception/Exception.h"
namespace glite{namespace wms{namespace client{namespace utilities{
#define WMS_EXCM_TRY()try{
#define WMS_EXCM_CATCH(sev)}catch (glite::wmsutils::exception::Exception &exc){cout << errMsg(sev,exc,true);}
enum severity{
	WMS_NONE,
	WMS_WARNING,
	WMS_ERROR,
	WMS_FATAL
};

enum errCode{
	DEFAULT_ERR_CODE
};

class WmsClientException: public glite::wmsutils::exception::Exception {
public:
	   WmsClientException(const std::string& file, int line,const std::string& method,
	   		int code,const std::string& exception_name, const std::string& error);
};
// Static methods:
const std::string errMsg(severity sev,glite::wmsutils::exception::Exception& exc, bool debug);
const std::string errMsg(severity sev,const std::string& title,const std::string& err, bool debug);
}}}} // ending namespaces

#endif


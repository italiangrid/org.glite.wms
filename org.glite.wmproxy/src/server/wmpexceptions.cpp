#include "wmpexceptions.h"

//namespace glite {
//namespace wms {
//namespace wmproxy {

using namespace std;
using namespace glite::wmsutils::exception;



JobException::JobException(const std::string& file, int line, const std::string& method, int code, const std::string& exception_name)
	: Exception(file, line, method, code, exception_name)
{}

JobTimeoutException::JobTimeoutException(const std::string& file, int line, const std::string& method, int code)
	: JobException(file, line, method, code, "JobTimeoutException")
{
	error_message = "Submit notification timeout expired";
}

JobOperationException::JobOperationException(const std::string& file, int line, const std::string& method, int code, const std::string& reason)
	:JobException(file, line, method, code, "JobOperationException")
    {
	error_message = "The Operation is not allowed: " + reason;
	error_message += reason;
}

//} // wmproxy
//} // wms
//} // glite

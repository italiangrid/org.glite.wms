/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpexceptions.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpexceptions.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

using namespace std;
using namespace glite::wmsutils::exception;

JobException::JobException(const std::string& file, int line,
	const std::string& method, int code, const std::string& exception_name)
	: Exception(file, line, method, code, exception_name)
{}

JobTimeoutException::JobTimeoutException(const std::string& file, int line,
	const std::string& method, int code)
	: JobException(file, line, method, code, "JobTimeoutException")
{
	error_message = "Submit notification timeout expired";
}

JobOperationException::JobOperationException(const std::string& file, int line,
	const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "JobOperationException")
{
	error_message = "The Operation is not allowed: " + reason;
}

ProxyOperationException::ProxyOperationException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "ProxyOperationException")
{
	error_message = "Proxy exception: " + reason;
}

NotAVOMSProxyException::NotAVOMSProxyException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "NotAVOMSProxyException")
{
	error_message = reason;
}

FileSystemException::FileSystemException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "FileSystemException")
{
	error_message += reason;
}

AuthorizationException::AuthorizationException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "AuthorizationException")
{
	error_message += reason;
}

AuthenticationException::AuthenticationException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "AuthenticationException")
{
	error_message += reason;
}

GaclException::GaclException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "GaclException")
{
	error_message += reason;
}

LBException::LBException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "LBException")
{
	error_message += reason;
}

ServerOverloadedException::ServerOverloadedException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "ServerOverloadedException")
{
	error_message += reason;
}

ConfigurationException::ConfigurationException(const std::string& file,
	int line, const std::string& method, int code, const std::string& reason)
	: JobException(file, line, method, code, "ConfigurationException")
{
	error_message += reason;
}

} // utilities
} // wmproxy
} // wms
} // glite

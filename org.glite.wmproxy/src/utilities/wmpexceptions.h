/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpexceptions.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H
#define GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H

#include "wmpexception_codes.h"
#include "glite/wmsutils/exception/Exception.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

class JobException : public glite::wmsutils::exception::Exception {
	protected:
	JobException(const std::string& file, int line, const std::string& method,
		int code, const std::string& exception_name);
};

class JobTimeoutException : public JobException {
	public:
	JobTimeoutException(const std::string& file, int line,
		const std::string& method, int code);
};

class JobOperationException : public JobException {
	public:
	JobOperationException(const std::string& file, int line, 
		const std::string& method, int code, const std::string& reason);
};

class ProxyOperationException : public JobException {
	public:
	ProxyOperationException(const std::string& file, int line, 
		const std::string& method, int code, const std::string& reason);
};

class NotAVOMSProxyException : public JobException {
	public:
	NotAVOMSProxyException(const std::string& file, int line, 
		const std::string& method, int code, const std::string& reason);
};

class FileSystemException : public JobException {
	public:
	FileSystemException(const std::string& file, int line, 
		const std::string& method, int code, const std::string& reason);
};

class AuthorizationException : public JobException {
	public:
	AuthorizationException(const std::string& file, int line, 
		const std::string& method, int code, const std::string& reason);
};

class AuthenticationException : public JobException {
	public:
	AuthenticationException(const std::string& file, int line,
		const std::string& method, int code, const std::string& reason);
};

class GaclException : public JobException {
	public:
	GaclException(const std::string& file, int line,
		const std::string& method, int code, const std::string& reason);
};

class LBException : public JobException {
	public:
	LBException(const std::string& file, int line,
		const std::string& method, int code, const std::string& reason);
};

} // utilities
} // wmproxy
} // wms
} // glite

#endif // GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H

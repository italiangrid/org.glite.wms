/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H
#define GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H

#include "exception_codes.h"
#include "glite/wmsutils/exception/Exception.h"

//namespace glite {
//namespace wms {
//namespace wmproxy {

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

//} // wmproxy
//} // wms
//} // glite

#endif // GLITE_WMS_WMPROXY_WMPEXCEPTIONS_H

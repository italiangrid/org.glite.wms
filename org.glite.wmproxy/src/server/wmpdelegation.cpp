/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpdelegation.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <string>
#include <iostream>

// boost
#include <boost/lexical_cast.hpp>

#include "wmpdelegation.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

// Common utility methods
#include "utilities/wmputils.h" // getUserDN()

// Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"

// Gridsite C library
extern "C" {
	#include "gridsite.h"
}

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

const char * WMPDelegation::GRST_PROXYCACHE = "proxycache";
const char * WMPDelegation::DOCUMENT_ROOT = "DOCUMENT_ROOT";

// gLite environment variables
const char * GLITE_LOCATION = "GLITE_LOCATION";
const char * GLITE_WMS_LOCATION = "GLITE_WMS_LOCATION";

using namespace std;
using namespace glite::wms::wmproxy::server;  //Exception codes

namespace logger       = glite::wms::common::logger;
namespace wmputilities = glite::wms::wmproxy::utilities;

char *
WMPDelegation::getProxyDir()
{
	GLITE_STACK_TRY("getProxyDir()");
	edglog_fn("WMPDelegation::getProxyDir");

	char * docroot = getenv(DOCUMENT_ROOT);
	if (!docroot) {
		edglog(fatal)<<"Unable to get DOCUMENT_ROOT environment variable value"
			<<endl;
		throw wmputilities::FileSystemException(__FILE__, __LINE__,
			"getProxyDir()", wmputilities::WMS_FILE_SYSTEM_ERROR,
			"Unable to get DOCUMENT_ROOT environment variable value\n(please "
				"contact server administrator)");
	}
	char * proxydir = (char*) malloc(1024);
	asprintf(&proxydir, "%s/%s", docroot, GRST_PROXYCACHE);
	
	// Creating proxydir if it doesn't exist
	wmputilities::createSuidDirectory(string(proxydir));
	
	return proxydir;
	
	GLITE_STACK_CATCH();
}

string
WMPDelegation::getProxyRequest(const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyRequest()");
	edglog_fn("WMPDelegation::getProxyRequest");
	
	char * user_dn = NULL;
    user_dn = wmputilities::getUserDN();
	char * request = NULL;
	if (GRSTx509MakeProxyRequest(&request, getProxyDir(), 
			(char*) delegation_id.c_str(), user_dn) != 0) {
		edglog(critical)<<"Unable to complete Proxy request"<<endl;
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq()", wmputilities::WMS_PROXY_ERROR,
			"Unable to complete Proxy request");
	}
	
	string proxy_req = "";
	int i = 0;
	while (request[i] != '\0') {
		proxy_req += request[i];
		i++;
	}
	return proxy_req;
	
	GLITE_STACK_CATCH();
}

void
WMPDelegation::putProxy(const string &delegation_id, const string &proxy_req)
{
	GLITE_STACK_TRY("putProxy()");
	edglog_fn("WMPDelegation::putProxy");
	
	char * user_dn = NULL;
  	user_dn = wmputilities::getUserDN();
  	edglog(debug)<<"Proxy dir: "<<string(WMPDelegation::getProxyDir())<<endl;
  	edglog(debug)<<"delegation id: "<<delegation_id<<endl;
  	edglog(debug)<<"User DN: "<<string(user_dn)<<endl;
	if (GRSTx509CacheProxy(WMPDelegation::getProxyDir(),
			(char*) delegation_id.c_str(), user_dn,
			(char*) proxy_req.c_str()) != GRST_RET_OK) {
		edglog(critical)<<"Unable to store client Proxy"<<endl;
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"putProxy()", wmputilities::WMS_PROXY_ERROR,
			"Unable to store client Proxy");
    }
    
    GLITE_STACK_CATCH();
}

string
WMPDelegation::getDelegatedProxyPath(const string &delegation_id)
{
	GLITE_STACK_TRY("getDelegatedProxyPath()");
	edglog_fn("WMPDelegation::getDelegatedProxyPath");
	
	char * user_dn = NULL;
	user_dn = wmputilities::getUserDN();
	char * delegated_proxy = GRSTx509CachedProxyFind(getProxyDir(), 
		(char*) delegation_id.c_str(), user_dn);
	if (delegated_proxy == NULL) {
		edglog(critical)<<"Unable to get delegated Proxy"<<endl;
		throw wmputilities::JobOperationException(__FILE__, __LINE__,
			"regist()", wmputilities::WMS_DELEGATION_ERROR,
			"Unable to get delegated Proxy");
	}
	string path = "";
	int i = 0;
	while (delegated_proxy[i] != '\0') {
		path += delegated_proxy[i];
		i++;
	}
	return path;
	
	GLITE_STACK_CATCH();
}

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include <string>
#include <iostream>

#include "wmpdelegation.h"

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "glite/wms/jdl/RequestAdExceptions.h"

// Common utility methods
#include "utilities/wmputils.h" // getUserDN()

// Gridsite C library
extern "C" {
	#include "gridsite.h"
}

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

const char* WMPDelegation::GRST_PROXYCACHE = "proxycache";
const char* WMPDelegation::DOCUMENT_ROOT = "DOCUMENT_ROOT";

using namespace std;
using namespace glite::wms::wmproxy::server;  //Exception codes

namespace wmputilities = glite::wms::wmproxy::utilities;

char *
WMPDelegation::getProxyDir()
{
	char *proxydir = NULL;
  	char *docroot = NULL;
  	
	docroot = getenv(DOCUMENT_ROOT);
	//if ((docroot == NULL) || (docroot == '\0')) {
		docroot = "/home/gridsite"; 
	//}
	asprintf(&proxydir, "%s/%s", docroot, GRST_PROXYCACHE);
	return proxydir;
}

string
WMPDelegation::getProxyRequest(const string &delegation_id)
{
	char *user_dn = NULL;
  	try {
  		user_dn = wmputilities::getUserDN();
  	} catch (ProxyOperationException &poe) {
  		throw poe;	
  	}
  	
	char *request = NULL;
	if (GRSTx509MakeProxyRequest(&request, getProxyDir(), 
			(char*) delegation_id.c_str(), user_dn) != 0) {
		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq(getProxyReqResponse &getProxyReq_response, "
			"const string &delegation_id)",
			wmputilities::WMS_PROXY_ERROR, "Unable to complete Proxy request");
	}
	
	string proxy_req = "";
	int i = 0;
	while (request[i] != '\0') {
		proxy_req += request[i];
		i++;
	}
	return proxy_req;	
}

void
WMPDelegation::putProxy(const string &delegation_id, const string &proxy_req)
{
	char *user_dn = NULL;
  	try {
  		user_dn = wmputilities::getUserDN();
  	} catch (ProxyOperationException &poe) {
  		throw poe;	
  	}
  
	if (GRSTx509CacheProxy(WMPDelegation::getProxyDir(),
			(char*) delegation_id.c_str(), user_dn,
			(char*) proxy_req.c_str()) != GRST_RET_OK) {
		throw ProxyOperationException(__FILE__, __LINE__,
			"putProxy(putProxyResponse &putProxyReq_response, const string "
			"&delegation_id, const string &proxy)",
			wmputilities::WMS_PROXY_ERROR, "Unable to store client Proxy");
    }	
}

string
WMPDelegation::getDelegatedProxyPath(const string &delegation_id)
{
	char *user_dn = NULL;
	try {
		user_dn = wmputilities::getUserDN();
	} catch (ProxyOperationException &poe) {
		throw poe;	
	}
	//char *delegated_proxy = GRSTx509CachedProxyKeyFind(getProxyDir(), 
	char *delegated_proxy = GRSTx509CachedProxyFind(getProxyDir(), 
		(char*) delegation_id.c_str(), user_dn);
	if (delegated_proxy == NULL) {
		throw JobOperationException(__FILE__, __LINE__,
			"regist(jobRegisterResponse &jobRegister_response, const string "
			"&delegation_id, const string &jdl, JobAd *jad)",
			wmputilities::WMS_DELEGATION_ERROR, "Unable to get delegated Proxy");
	}
	string path = "";
	int i = 0;
	while (delegated_proxy[i] != '\0') {
		path += delegated_proxy[i];
		i++;
	}
	return path;
}

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


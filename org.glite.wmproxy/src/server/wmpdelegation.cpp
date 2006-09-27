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

string
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
	
	char * proxydir;
	asprintf(&proxydir, "%s/%s", docroot, GRST_PROXYCACHE);
	string returnproxydir = string(proxydir);
	free(proxydir);
	
	// Creating proxydir if it doesn't exist
	wmputilities::createSuidDirectory(returnproxydir);
	
	return returnproxydir;
	
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
	if (GRSTx509MakeProxyRequest(&request, (char*) getProxyDir().c_str(), 
			(char*) delegation_id.c_str(), user_dn) != 0) {
		edglog(critical)<<"Unable to complete Proxy request"<<endl;
		free(user_dn);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq()", wmputilities::WMS_PROXY_ERROR,
			"Unable to complete Proxy request");
	}
	
	string proxy_req = string(request);
	
	free(user_dn);
	free(request);
	
	return proxy_req;
	
	GLITE_STACK_CATCH();
}

pair<string, string>
WMPDelegation::getNewProxyRequest()
{
	GLITE_STACK_TRY("getNewProxyRequest()");
	edglog_fn("WMPDelegation::getNewProxyRequest");

	char * delegation_id =  GRSTx509MakeDelegationID();

	delegation_id = GRSTx509MakeDelegationID();
	edglog(debug)<<"Generated Delegation ID: "<<delegation_id<<endl;

	char * user_dn = NULL;
	user_dn = wmputilities::getUserDN();

	// Check Proxy Path
	if (wmputilities::fileExists(WMPDelegation::getDelegatedProxyPath(delegation_id))){
		edglog(critical)<<"Unable to complete New Proxy request: previous proxy already exists"<<endl;
		free(delegation_id);
		free(user_dn);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getNewProxyRequest()", wmputilities::WMS_PROXY_ERROR,
			"Unable to complete New Proxy request: previous proxy already exists");
	}

	// Check Termination time
	time_t *start = (time_t*) malloc(sizeof(time_t));
	time_t *finish = (time_t*) malloc(sizeof(time_t));
	if (GRSTx509ProxyGetTimes((char*) getProxyDir().c_str(),
			delegation_id, user_dn, start, finish) != GRST_RET_OK) {
		edglog(critical)<<"Unable to get termination time"<<endl;
		free(delegation_id);
		free(user_dn);
		free(start);
		free(finish);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getTerminationTime()", wmputilities::WMS_PROXY_ERROR,
			"Unable to get termination time");
	}
	edglog(debug)<<"Termination Time: "<<*finish <<endl;
	free(user_dn);
	free(start);
	free(finish);

	// Make Actual Proxy Request
	char * request = NULL;
	if (GRSTx509MakeProxyRequest(&request, (char*) getProxyDir().c_str(), 
			delegation_id, user_dn) != 0) {
		edglog(critical)<<"Unable to complete New Proxy request"<<endl;
		free(delegation_id);
		free(user_dn);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getNewProxyRequest()", wmputilities::WMS_PROXY_ERROR,
			"Unable to complete New Proxy request");
	}
	
	pair<string, string> retpair;
	retpair.first = string(delegation_id);
	retpair.second = string(request);
	
	free(delegation_id);
	free(user_dn);
	free(request);
	
	return retpair;

	GLITE_STACK_CATCH();
}

void
WMPDelegation::putProxy(const string &delegation_id, const string &proxy_req)
{
	GLITE_STACK_TRY("putProxy()");
	edglog_fn("WMPDelegation::putProxy");
	
	char * user_dn = NULL;
  	user_dn = wmputilities::getUserDN();

  	edglog(debug)<<"Proxy dir: "<<getProxyDir()<<endl;
  	edglog(debug)<<"delegation id: "<<delegation_id<<endl;
  	edglog(debug)<<"User DN: "<<string(user_dn)<<endl;
  	
	if (GRSTx509CacheProxy((char*) getProxyDir().c_str(),
			(char*) delegation_id.c_str(), user_dn,
			(char*) proxy_req.c_str()) != GRST_RET_OK) {
		edglog(critical)<<"Unable to store client Proxy"<<endl;
		free(user_dn);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"putProxy()", wmputilities::WMS_PROXY_ERROR,
			"Unable to store client Proxy");
    }
    
    free(user_dn);
    
    GLITE_STACK_CATCH();
}

void
WMPDelegation::destroyProxy(const string &delegation_id)
{
	GLITE_STACK_TRY("destroyProxy()");
	edglog_fn("WMPDelegation::destroyProxy");
	
	char * user_dn = NULL;
  	user_dn = wmputilities::getUserDN();
  	
  	edglog(debug)<<"Proxy dir: "<<getProxyDir()<<endl;
  	edglog(debug)<<"delegation id: "<<delegation_id<<endl;
  	edglog(debug)<<"User DN: "<<string(user_dn)<<endl;
  	
	if (GRSTx509ProxyDestroy((char*) getProxyDir().c_str(),
			(char*) delegation_id.c_str(), user_dn) != GRST_RET_OK) {
		edglog(critical)<<"Unable to destroy Proxy"<<endl;
		free(user_dn);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"destroyProxy()", wmputilities::WMS_PROXY_ERROR,
			"Unable to destroy Proxy");
    }
    
    free(user_dn);
    
    GLITE_STACK_CATCH();
}

time_t
WMPDelegation::getTerminationTime(const string &delegation_id) {
	GLITE_STACK_TRY("getTerminationTime()");
	edglog_fn("WMPDelegation::getTerminationTime");

	char * user_dn = NULL;
  	user_dn = wmputilities::getUserDN();

  	time_t *start = (time_t*) malloc(sizeof(time_t));
  	time_t *finish = (time_t*) malloc(sizeof(time_t));

  	edglog(debug)<<"Proxy dir: "<<getProxyDir()<<endl;
  	edglog(debug)<<"delegation id: "<<delegation_id<<endl;
  	edglog(debug)<<"User DN: "<<string(user_dn)<<endl;

	if (GRSTx509ProxyGetTimes((char*) getProxyDir().c_str(),
			(char*) delegation_id.c_str(), user_dn, start, finish) != GRST_RET_OK) {
		edglog(critical)<<"Unable to get termination time"<<endl;
		free(user_dn);
		free(start);
		free(finish);
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getTerminationTime()", wmputilities::WMS_PROXY_ERROR,
			"Unable to get termination time");
	}
	time_t time = *finish;
	free(user_dn);
	free(start);
	free(finish);

    return time;

    GLITE_STACK_CATCH();
}

string
WMPDelegation::getDelegatedProxyPath(const string &delegation_id)
{
	GLITE_STACK_TRY("getDelegatedProxyPath()");
	edglog_fn("WMPDelegation::getDelegatedProxyPath");

	char * user_dn = NULL;
	user_dn = wmputilities::getUserDN();
	
	edglog(debug)<<"Proxy dir: "<<getProxyDir()<<endl;
  	edglog(debug)<<"delegation id: "<<delegation_id<<endl;
  	edglog(debug)<<"User DN: "<<string(user_dn)<<endl;
  	
  	char * user_dn_enc = NULL;
  	user_dn_enc = GRSThttpUrlEncode(user_dn);
  	free(user_dn);
  	
  	char * filename = NULL;
  	asprintf(&filename, "%s/%s/%s/userproxy.pem", (char*) getProxyDir().c_str(),
  		user_dn_enc, (char*) delegation_id.c_str());
  	free(user_dn_enc);
  	
  	string proxypath = string(filename);
  	free(filename);
  	
  	return proxypath;
	
	GLITE_STACK_CATCH();
}


} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


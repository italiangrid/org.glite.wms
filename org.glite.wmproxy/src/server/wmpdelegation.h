/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPDELEGATION_H
#define GLITE_WMS_WMPROXY_WMPDELEGATION_H

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

/**
 * WMPDelegation class provides a set of utility methods to work on
 * Proxy delegation.
 *
 * @brief utility methods to work on Proxy delegation
 * @version 1.0
 * @date 2004
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
*/
class WMPDelegation {
public:
	
	/**
	 * Gets the user Distinguished Name of the user. This is the DN of the request
	 * environment
	 * @return the user DN
	 */
	//static char* getUserDN();
	
	/**
	 * Gets a delegated Proxy request. A Proxy request must be done in order
	 * to obtain a request to create a delegated Proxy. To obtain a Proxy
	 * request a delegation identifier must be provided
	 * @param delegation_id the delegation id identifing the delegation session
	 * @return the Proxy request
	 * @see putProxy
	 */
	static std::string getProxyRequest(const std::string &delegation_id);
	
	/**
	 * Creates the delegated Proxy. The Proxy is created inside the delegated
	 * Proxy path
	 * @param delegation_id the delegation id identifing the delegation session
	 * @param proxy_req the request obtained with the method getProxyReq
	 * @see getProxyRequest
	 */
	static void putProxy(const std::string &delegation_id,
		const std::string &proxy_req);
		
	/**
	 * Gets the delegated proxy path. This is the Proxy corresponding to the 
	 * delegation identifier passed as argument and to the specific user. The
	 * user is represented by the Distinguished Name contained inside the request
	 * environment
	 * @param the delegation id identifing the delegated session
	 * @return the user DN
	 */
	static std::string getDelegatedProxyPath(const std::string &delegation_id);
	
	
private:
	/**
	 * Gets the path of the delegated Proxy. This is the directory where all the
	 * delegated Proxies are saved
	 * @return the delegated Proxy path
	 */
	static char* getProxyDir();
	
	// The proxy cache directory environment variable name
	static const char* GRST_PROXYCACHE;
	// The document root environment variable name. The directory where the
	// delegated Proxy are saved is: $DOCUMENT_ROOT/$GRST_PROXYCACHE
	static const char* DOCUMENT_ROOT;
	// the Distinguished Name environment variable name
	static const char* SSL_CLIENT_DN;
	
	
}; // END WMPDelegation class
 
} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPDELEGATION_H

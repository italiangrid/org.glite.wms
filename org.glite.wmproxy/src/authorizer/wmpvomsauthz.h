/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpvomsauthz.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPVOMSAUTHZ_H
#define GLITE_WMS_WMPROXY_WMPVOMSAUTHZ_H

// edglog /usr/include/unistd.h compilation problem workaround:
// declaration of `char* crypt(const char*, const char*) throw ()' 
// throws different exceptions
#if defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ == 3
#include <sys/cdefs.h>
#undef __THROW
#define __THROW
#endif

// API VOMS
extern "C" {
	#include "glite/security/voms/voms_apic.h"
}

#include "server/wmpresponsestruct.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

class VOMSAuthZ {
	
	public:
	
		VOMSAuthZ(const std::string &proxypath);
		virtual ~VOMSAuthZ();
		
		bool hasVOMSExtension();
		
		char * getDN();
		
		std::string getDefaultFQAN();
		
		std::string getDefaultVO();
		
		VOProxyInfoStructType * getDefaultVOProxyInfo();
		
		ProxyInfoStructType * getProxyInfo();
		
		static time_t ASN1_UTCTIME_get(const ASN1_UTCTIME *s);
		
		static const long convASN1Date(const std::string &date);

	private:
	
		X509 * cert;
		struct vomsdata * data;
		
		int parseVoms(char * proxypath);
		
		std::string errormessage(int error);
};

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPVOMSAUTHZ_H

/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

//
// File: wmpvomsauthz.h
// Author: Giuseppe Avellino <egee@datamat.it>
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
#include "server/wmpresponsestruct.h"

// API VOMS
extern "C" {
	#include "voms/voms_apic.h"
}

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

class VOMSAuthN {
public:
	
	VOMSAuthN(const std::string &proxypath);
	virtual ~VOMSAuthN();
	bool hasVOMSExtension();
	char* getDN();
	std::string getDefaultFQAN();
	std::vector<std::string> getFQANs();
	VOProxyInfoStructType* getDefaultVOProxyInfo();
	ProxyInfoStructType* getProxyInfo();
private:
	int parseVoms(char* proxypath);
	X509* cert_;
	struct vomsdata* data_;
};

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPVOMSAUTHZ_H

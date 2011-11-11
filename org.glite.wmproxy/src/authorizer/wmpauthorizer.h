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
limitations under the License.  */

#ifndef GLITE_WMS_WMPROXY_WMPAUTHORIZER_H
#define GLITE_WMS_WMPROXY_WMPAUTHORIZER_H

#include <string>
#include <vector>

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

/**
 * WMPAuthorizer class provides a set of utility methods for performing
 * Grid Users authorization and mapping to local users. It also relies on 
 * the functions provided by the security lcmaps component.
*/

enum FQANFields {
	FQAN_VO,
	FQAN_GROUP,
	FQAN_SUBGROUP,
	FQAN_ROLE,
	FQAN_CAPABILITY
};

void checkProxy(const std::string &proxypath);
void checkProxyExistence(const std::string &proxypath, const std::string &jobid);
std::vector<std::pair<std::string, std::string> > parseFQAN(const std::string &fqan);
bool checkGridsiteJobDrain();
void setGridsiteJobGacl(const std::string &jobid);
void setGridsiteJobGacl(std::vector<std::string> &jobids);

class WMPAuthorizer {
#ifndef GLITE_WMS_WMPROXY_TOOLS
public:
	WMPAuthorizer(std::string const& operation);
	WMPAuthorizer(std::string const& operation, std::string const& proxycert);
	~WMPAuthorizer();
   
	/**
	* Calls Gacl to check if the user is authorized to submit requests to WMProxy. 
	* Check is done on the basis of user's credential.
	*/
	void checkGaclUserAuthZ(std::string const& fqan, std::string const& dn);
    
	/**
	* Returns the user name
	* @return a string containing the local user name
	*/
	std::string getUserName();
    
	/**
	* Returns the user identifier
	* @return a uid_t type representing the local user id
	*/
	uid_t getUserId();
    
	/**
	* Returns the user group
	* @return a uid_t type representing the local user group
	*/
	uid_t getUserGroup();
    
	/**
	* Does authorization via FQAN
	* @param certfqan user certificate FQAN
	* @param jobid the job identifier
	*/
	void authorize(const std::string &certfqan = "", const std::string &jobid = "");
private:
	void mapUser();
	std::string username_;
	uid_t userid_;
	uid_t usergroup_;
	bool mapdone_;
	std::string fqan_;
	std::string action_;
	std::string userproxypath_;
    
#endif // #ifndef GLITE_WMS_WMPROXY_TOOLS
};

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPAUTHORIZER_H

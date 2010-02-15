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
// File: wmpauthorizer.h
// Author: Giuseppe Avellino <egee@datamat.it>
//

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

class WMPAuthorizer {
public:

#ifndef GLITE_WMS_WMPROXY_TOOLS
	/**
	 * Constructor
	 */
	WMPAuthorizer(char * lcmaps_logfile = NULL);
	
	/**
	 * Destructor
	 */
    virtual ~WMPAuthorizer() throw();

   
   	/**
     * Calls Gacl to check if the user is authorized to submit requests to WMProxy. 
     * Check is done on the basis of user's credential.
     */
    void checkGaclUserAuthZ();
    
    //LCAS CHECK bool checkLCASUserAuthZ(const std::string &dn);
    
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
    void authorize(const std::string &certfqan = "",
    	const std::string &jobid = "");
    	
 #endif // #ifndef GLITE_WMS_WMPROXY_TOOLS
 
    /**
     * Returns a vector containing the fields of the input fqan
     * @param fqan input fqan
     * @return  the fqan vector
     */
	static std::vector<std::pair<std::string,std::string> >
		parseFQAN(const std::string &fqan);
	/**
	 * Compares two user Distinguished Names in order to see if they are equal.
	 * different OpenSSL library versions use a different way to represent DN;
	 * in some version you have the e-mail address specified as "Email", in others
	 * with "emailAddress". Two different DN with only this difference in e-mail
	 * are considered egual.
	 * @param dn1 the first DN to compare
	 * @param dn2 the second DN to compare
	 * @return 0 if the two DN are different
	 */
	static bool compareDN(char * dn1, char * dn2);

    /**
     * Checks if the input fqan is contained in the referring fqan
     * @param infqan input fqan
     * @param reffqan referring fqan
     * @return true if the in_ fqan is contained in ref_fqan
     */
    static bool compareFQAN(const std::string &infqan,
    	const std::string &reffqan);

    static bool compareFQANAuthN(const std::string &infqan,
        const std::string &reffqan);
   
 	
#ifndef GLITE_WMS_WMPROXY_TOOLS

    /**
     * Creates the gacl files in the user job directories for the job identified
     * by jobid
     * @param jobid the identifier of the job
     */
    static void setJobGacl(const std::string &jobid);
    
    static void setJobGacl(std::vector<std::string> &jobids);

    /**
     * Checks if exec permission is denied for any user
     * in the gacl of the "DOCUMENT_ROOT" (drain configuration)
     * @return true if "drain" is set (exec is denied)
     */
    static bool checkJobDrain();
    
    /**
     * Returns the Proxy time left
     * @param proxypath the Proxy file path to get time left from
     * @return the Proxy time left in minutes
     */
    static const long getProxyTimeLeft(const std::string &proxypath);
    
    /**
     * Returns the Proxy not before time
     * @param proxypath the Proxy file path to get not before time from
     * @return the Proxy not before time
     */
    static const long getNotBefore(const std::string &proxypath);
    
    /**
     * Checks Proxy not before time and validity returning exception in case of
     * Proxy check fails
     * @param proxypath the Proxy file path to check
     */
    static void checkProxy(const std::string &proxypath);
    	
    /**
     * Checks if Job delegated Proxy or its copy exist. It also makes copy of
     * the Proxy if needed
     * @param proxypath the Proxy file path to check
     * @param jobid the job id to get the Proxy path from
     */
    static void checkProxyExistence(const std::string &proxypath,
    	const std::string &jobid);

private:
	std::string username;
	uid_t userid;
	uid_t usergroup;
	std::string lcmaps_logfile;
	bool mapdone;
	std::string certfqan;

	// job directories
	static const char* INPUT_SB_DIRECTORY ;
	static const char* OUTPUT_SB_DIRECTORY ;
	static const char* PEEK_DIRECTORY ;
	
	// Document root variable
	static const char * DOCUMENT_ROOT ;
	
	static const char* VOMS_GACL_VAR;
	static const std::string VOMS_GACL_FILE;
	
	/**
     * Calls LCMAPS to map the grid user to a local user.
     * Mapping is done on the basis of user's credential.
     */
    void mapUser(const std::string &certfqan = "");
    
  #endif // #ifndef GLITE_WMS_WMPROXY_TOOLS
};

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_WMPAUTHORIZER_H

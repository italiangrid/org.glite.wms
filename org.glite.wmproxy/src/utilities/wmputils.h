/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmputils.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H
#define GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H

#include <vector>

#include "glite/wmsutils/jobid/JobId.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace utilities {

#ifndef GLITE_WMS_WMPROXY_TOOLS
enum FQANFields {
	FQAN_VO,
	FQAN_GROUP,
	FQAN_SUBGROUP,
	FQAN_ROLE,
	FQAN_CAPABILITY
};


//
// File utility methods
//

/**
 * Checks if a file exists
 * @param path the file to check for existence
 * @return 0 if file exists 1 otherwise
 */
int fileExists(const std::string &path);

/**
 * Computes the size of the file represented by path
 * @param path file path
 * @return a long value representing the size
 */
long computeFileSize(const std::string & path);

/**
 * Returns the file name from a path
 * @param path file path
 * @return file name
 */
std::string getFileName(const std::string &path);

/**
 * Copies a file
 * @param source the source file
 * @param target the target file
 */
void fileCopy(const std::string &source, const std::string &target);

void uncompressFile(const std::string &filename, const std::string &startingpath);

void setFlagFile(const std::string &file, bool flag);

int operationLock(const std::string &lockfile, const std::string &opname);

void operationUnlock(int fd);

bool isOperationLocked(const std::string &lockfile);

void createSuidDirectory(const std::string &directory);

void untarFile(const std::string &file, const std::string &untar_starting_path,
	uid_t userid, uid_t groupid);

void writeTextFile(const std::string &path, const std::string &text);

std::string readTextFile(const std::string &path);



//
// Service & service host methods
//

/**
 * Returns the service host address
 * @return the service host address
 */
std::string getServerHost();

//std::string retrieveHostName();

/**
 * Returns the service endpoint as an URL format
 * @return the URL of the service endpoint
 */
std::string getEndpoint();

void parseAddressPort(const std::string &addressport,
	std::pair<std::string, int> &addresspair);



//
// Job specific methods
//

/**
 * Returns the destination URI reading the protocol to use from configuration
 * (default protocol is used)
 * @param jobid the job id from which calculate the job directory
 * @return the destination URI
 */
std::string getDestURI(const std::string &jobid, const std::string &protocol,
	const std::string &port);

std::vector<std::string> * getDestURIsVector(std::vector<std::pair<std::string,
	int> > protocols, int httpsport, const std::string &jid, bool addhttps = true);


std::vector<std::string> * getJobDirectoryURIsVector(
	std::vector<std::pair<std::string, int> > protocols,
	const std::string &defaultprotocol, int defaultport, int httpsport, 
	const std::string &jid, const std::string &protocol,
	const std::string &extradir = "");
	
/**
 * Returns the job reduced absolute path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return job reduced absolute path
 */
std::string getJobReducedPath(glite::wmsutils::jobid::JobId jid, int level = 0);

/**
 * Returns the job directory absolute path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return job directory absolute path
 */
std::string getJobDirectoryPath(glite::wmsutils::jobid::JobId jid, int level = 0);

/**
 * Returns the  job input directory relative path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return  job input directory relative path
 */
std::string getJobInputSBRelativePath(glite::wmsutils::jobid::JobId jid, int level = 0);
/**
 * Returns the job start lock file path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return job directory absolute path
 */
std::string getJobStartLockFilePath(glite::wmsutils::jobid::JobId jid,
	int level = 0);
	
std::string getGetOutputFileListLockFilePath(glite::wmsutils::jobid::JobId jid,
	int level = 0);

/** 
 * Returns the job Input Sandbox directory path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return input sandbox directory absolute path
 */
std::string getInputSBDirectoryPath(glite::wmsutils::jobid::JobId jid,
	int level = 0);
	
/** 
 * Returns the job Output Sandbox directory path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return output sandbox directory absolute path
 */
std::string getOutputSBDirectoryPath(glite::wmsutils::jobid::JobId jid,
	int level = 0);

/** 
 * Returns the job peek directory path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return output sandbox directory absolute path
 */
std::string getPeekDirectoryPath(glite::wmsutils::jobid::JobId jid,
	int level = 0, bool docroot = true);

/** 
 * Returns the job delegated Proxy path
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return delegated Proxy absolute path
 */
std::string getJobDelegatedProxyPath(glite::wmsutils::jobid::JobId jid,
	int level = 0);
	
/** 
 * Returns the job delegated Proxy path backup file
 * @param jid the job identifier of the job
 * @param level level, default value is 0
 * @return backup delegated Proxy absolute path
 */
std::string getJobDelegatedProxyPathBak(glite::wmsutils::jobid::JobId jid,
	int level = 0);

std::string getJobJDLOriginalPath(glite::wmsutils::jobid::JobId jid,
	bool isrelative = false, int level = 0);
	
std::string getJobJDLToStartPath(glite::wmsutils::jobid::JobId jid,
	bool isrelative = false, int level = 0);
	
std::string getJobJDLStartedPath(glite::wmsutils::jobid::JobId jid,
	bool isrelative = false, int level = 0);

std::string getJobJDLExistingStartPath(glite::wmsutils::jobid::JobId jid,
	bool isrelative = false, int level = 0);
	
/**
 * Returns the destination URI reading the protocol to use from configuration
 * (default protocol is used)
 * @param jobid the job id from which calculate the job directory
 * @return the destination URI
 */
std::string getDestURI(const std::string &jobid, const std::string &protocol,
	int port);
 
/**
 * Transforms a JobId into a valid filename
 * @param jobid the JobId instance
 * @param level
 * @param extended_path
 */
std::string to_filename(glite::wmsutils::jobid::JobId j, int level = 0,
	bool extended_path = true);



//
// OutputSandbox manipulation methods
//
std::vector<std::string> computeOutputSBDestURIBase(std::vector<std::string>
	outputsb, const std::string &baseuri);
	
std::vector<std::string> computeOutputSBDestURI(std::vector<std::string>
	osbdesturi, const std::string &dest_uri);



//
// Proxy & Proxy info
//

/*
 * Returns the Virtual Organisation contained in the requesting client Proxy.
 * The value is get from an environment variable set by GridSite
 * @return the Virtual Organisation
 */
std::string getEnvVO();

/*
 * Returns the FQAN contained in the requesting client Proxy.
 * The value is get from an environment variable set by GridSite
 * @return the FQAN
 */
std::string getEnvFQAN();

/**
 * Gets the user Distinguished Name of the user. This is the DN of the request
 * environment
 * @return the user DN
 */
char * getUserDN();

std::string convertDNEMailAddress(const std::string &dn);

/**
 * Parses the FQAN to get a vector containing the different elements
 * @param fqan the fqan to parse
 * @return a string vector containing the elements
 */
std::vector<std::string> parseFQAN(const std::string &fqan);

/**
 * Parses the FQAN to get a vector containing the different elements
 * @param fqan the fqan to parse
 * @return a string vector containing the elements
 */
std::vector<std::pair<std::string, std::string> >
	parseFQANPair(const std::string &fqan);

	

//
// "External" methods
//

/**
 * Calls the purger library method to purge job directory
 * @param dg_jobid the job identifier representing the job
 * @return true if the method was successfully, false otherwise
 */
bool doPurge(std::string dg_jobid);

/**
 * Gets the user total available disk quota
 * @param result reference variable where to put the results
 * @param uname the local user name
 * @return true if the operation was successfully, false otherwise
 */
bool getUserQuota(std::pair<long, long>& result, std::string uname);

/**
 * Gets the user free available disk quota
 * @param result reference variable where to put the results
 * @param uname the local user name
 * @return true if the operation was successfully, false otherwise
 */
bool getUserFreeQuota(std::pair<long, long>& result, std::string uname);

/**
 * Generate / Manage directory properties
 * @param dir name of the dir to be generated
 * @param userid the id of the user
 */
void managedir(const std::string &dir, uid_t userid, uid_t jobuserid,
	std::vector<std::string> jobids);

/**
 * Checks if the input string (representing an attribute name=value)
 * ends with "=NULL" substring
 * @param filed input string
 * @return true if the attribute is NULL
 */
bool isNull(std::string field);
/**
   * Generates randomic integer value in the range [lowerlimit, upperlimit]
   * @param lowerlimit the integer value lower limit
   * @param upperlimit the integer value upper limit
   * @param the randomic generated value
 */
int generateRandomNumber(int lowerlimit, int upperlimit);

/**
 * Server debugging method to wait for a while
 * @param seconds number of seconds to wait
 */
void waitForSeconds(int seconds);

#endif // #ifndef GLITE_WMS_WMPROXY_TOOLS

/**
   * Removes white spaces at the beginning and 
   * at the end of the input string if there is any 
   * @param the inputs string to be cleaned 
   * @return the string, with white space removed from the front and end. 
   */
const std::string cleanString(std::string str);

/**
   * Converts all of the characters in this String to lower case
   * @param src the input string 
   * @return the string, converted to lowercase.
 */
const std::string toLower(const std::string &src);

/**
   * Cuts the input string in two pieces (label and value) according to
   * the separator character "="
   * @param field the input string which format has to be label=value 
   * @param label returns the "label" part of the string 
   * @param value returns the "value" part of the string 
 */
void split(const std::string &field, std::string &label, std::string &value);

/**
* Checks whether a string is contained into a vector
* @param vect the vector of string
* @param elem the string to be looked for
* @returns true whether the vector contained the specified string
*/
bool hasElement(const std::vector<std::string> &vect, const std::string &elem);
/*
* Gets the absolute path of the file
* @param file the input file
* @return the absolute file path
*/
const std::string getAbsolutePath(const std::string &file);

/**
 * Removes '/' characters at the end of the of the input pathname
 * @param fpath the path to be normalized
 * @return the normalized path
 */
const std::string normalizePath( const std::string &fpath ) ;

} // namespace utilities
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_WMPROXY_UTILITIES_WMPUTILS_H

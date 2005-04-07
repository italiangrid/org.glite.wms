#ifndef  GLITE_WMS_WMPROXYAPICPP_H
#define GLITE_WMS_WMPROXYAPICPP_H

/**
* \file wmproxy_api.h
* \brief wsdl wmproxy service wrapper
* A wrapper around wmproxy Web Service. It provides primitive or simple structure to access more complicated service methods
*/

#include <iostream>
#include <string>
#include <vector>

namespace glite {
namespace wms {
namespace wmproxyapi {

/**  Base exception wrap */
struct BaseException {
	std::string                          methodName  ;
	/// Element Timestamp of type xs:dateTime
	time_t                               Timestamp     ;
	/// Element ErrorCode of type xs:string
	std::string*                         ErrorCode   ;
	/// Element Description of type xs:string
	std::string*                         Description ;
	/// Vector of std::string with length 0..unbounded
	std::vector<std::string           > *FaultCause   ;
};

struct AuthenticationException:BaseException{};
struct AuthorizationException:BaseException{};
struct InvalidArgumentException:BaseException{};
struct GetQuotaManagementException:BaseException{};
struct NoSuitableResourcesException:BaseException{};
struct JobUnknownException:BaseException{};
struct OperationNotAllowedException:BaseException{};
struct GenericException:BaseException{};

/**
* Used to specify the jobtype. multiple jobtype can be specified togheter by the bitwise (|) or operation
*/
enum jobtype {
	/*** Normal Jobs */
	JOBTYPE_NORMAL=1,
	/*** Parametric Jobs */
	JOBTYPE_PARAMETRIC =2,
	/***  Interactive Jobs */
	JOBTYPE_INTERACTIVE=4,
	/*** Mpi Jobs */
	JOBTYPE_MPICH=8,
	/*** PArtitionable Jobs */
	JOBTYPE_PARTITIONABLE=16,
	/*** Checkpointable Jobs */
	JOBTYPE_CHECKPOINTABLE =32
};
/**
* Used to define the the structure of a dag
* the name of the node might be NULL for the first instance (if it's representing the dag itself) while all the children have to be properly initialised.
*/
struct NodeStruct {
	/** The name of the node (might be NULL for the first instance) */
	std::string* nodeName ;
	/** A list of all the children for this node*/
	std::vector< NodeStruct* > childrenNodes ;
};
/**
* Used to define the jobid hierarchy of a job or a dag. the nodeName is mandatory only when the jobid refers to a dag node
*/
struct JobIdApi {
	/** */
	std::string jobid ;
	std::string* nodeName ;
	std::vector< JobIdApi* > children ;
};
/**
* Used to configure non-default properties such as:
* A proxy file different from the default one
* An endpoint service different from the one specified in the wsdl
* A further directory where to look for CA certificates */
struct ConfigContext{
	ConfigContext( std::string p , std::string s, std::string t);
	virtual ~ConfigContext() throw();
	std::string proxy_file;
	std::string endpoint;
	std::string trusted_cert_dir;
};
/**
* Retrieve service version information
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return an alpha-numeric version representation
*/
std::string getVersion(ConfigContext *cfs=NULL);
/**
* Register the job to the LB server. Once a job has been registered,
* a jobStart has to be issued in order to let it be properly submitted to the networkserver
* @param jdl the jdl string representation of the job
* @param delegationID the identification of the delegated proxy asociated to that job
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the structure associated to the registered job, with its jobid(s)
*/
JobIdApi jobRegister (const std::string &jdl, const std::string &delegationId, ConfigContext *cfs=NULL);
/**
* Register the job and than submit it to the networkserver
* @param jdl the jdl string representation of the job
* @param delegationID the identification of the delegated proxy asociated to that job
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the structure associated to the registered job, with its jobid(s)
*/
JobIdApi jobSubmit(const std::string &jdl, const std::string &delegationId, ConfigContext *cfs=NULL);
/**
* Submit the job to the networkserver.Before being submitted,a jobRegister must be issued
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
*@param jobid the string identification of the job (as returned from jobRegister method)
*/
void jobStart(const std::string &jobid, ConfigContext *cfs=NULL);
/**
* Stop the job from beeing executed.
*@param jobid the string identification of the job 
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
*/
void jobCancel(const std::string &jobid, ConfigContext *cfs=NULL);
/**
* Retrieve the maximum Input sandbox size a user can count-on for a job submission if using the space managed by the WM.
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the input sandbox maximum size in bytes
*/
long getMaxInputSandboxSize(ConfigContext *cfs=NULL);
/**
* Create a unique URI associated to the job
* @param jobid the string identification of the job
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the uniqe URI associated to the provided jobid
*/
std::vector<std::string>  getSandboxDestURI(const std::string &jobid, ConfigContext *cfs=NULL);
/**
* Retrieve the total amount of user free space quota
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
*@return a pair containing the soft and the hard limit quota
**/
std::pair<long, long> getFreeQuota(ConfigContext *cfs=NULL);
/**
* Retrieve the total amount of user space quota
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
*@return a pair containing the soft and the hard limit quota
**/
std::pair<long, long> getTotalQuota(ConfigContext *cfs=NULL);
/**
* Retrieve the avaliable user space quota, still to be possibly used
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @param jobid the string identification of the job
*@return a pair containing the soft and the hard limit quota
**/
void jobPurge(const std::string &jobid, ConfigContext *cfs=NULL);
/**
* Retrieve the list of URIs where the output files created during job execution have been stored in the WM managed space
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @param jobid the string identification of the job
* @return a vector containing, for each element, the URI of the output file and corresponding size in bytes
*/
std::vector <std::pair<std::string , long> > getOutputFileList (const std::string &jobid, ConfigContext *cfs=NULL);
/**
* Retrieve all resources matching with the provided jdl
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return a vector containing, for each recource found, its full name and its rank
*/
std::vector <std::pair<std::string , long> > jobListMatch (const std::string &jdl, ConfigContext *cfs=NULL);
/**
* Create a valid template ready for submission for a job
* @param type the jobtype of the job. Multiple jobtype can be specified toghether through the bitwise '|' operator
* @param executable the name of the executable to be executed
* @param arguments the arguments needed by the executable (empty string if not necessary)
* @param requirements a string representing the expression describing the requirements (which is an attribute of boolean type) for the job
* @param rank a string representing the expression for the rank (which is an attribute of double type) for the job
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the JDL string representation of the job
*/
std::string getJobTemplate (int type, const std::string &executable,const std::string &arguments,const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
/**
* Create a valid template ready for submission for a job
* @param dependencies the dependency structure of the dag: each node must list all the nodes that depends on it.
* @param requirements a string representing the expression describing the requirements (which is an attribute of boolean type) for all the nodes of the dag
* @param rank a string representing the expression for the rank (which is an attribute of double type) for all the nodes of the dag
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the JDL string representation of the dag
*/
std::string getDAGTemplate(NodeStruct dependencies, const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
/**
* Create a valid template JDL for a Collection of jobs
* @param jobNumber  the number of jobs to be created for the collection
* @param requirements a string representing the expression describing the requirements (which is an attribute of boolean type) for all the jobs in the collection
* @param rank a string representing the expression for the rank (which is an attribute of double type) for all the jobs in the collection
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the JDL string representation of the collection of jobs
*/
std::string getCollectionTemplate(int jobNumber, const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
/**
* Create a valid template JDL for a parametric job
* @param attributes all the attributes that contains reference to a parameter. Multiple attributes can be specified toghegher through the bitwise '|' operator ( as specified in attribute)
* @param parameters  the number of different parameters to be created
* @param start (default value is 0) the starting point where to begin to parametrise
* @param step (default value is 1) the step between one parameter and the next one among param_start
* @param requirements a string representing the expression describing all the Job requirements (which is an attribute of boolean type)
* @param rank a string representing the expression for the rank (which is an attribute of double type) of the resource
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the JDL string representation of the parametric job
*/
std::string getIntParametricJobTemplate (std::vector<std::string> attributes , int parameters , int start , int step , const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
/**
* Create a valid template JDL for a parametric job
* @param attributes all the attributes that contains reference to a parameter. Multiple attributes can be specified toghegher through the bitwise '|' operator ( as specified in attribute)
* @param parametersa vector containing all the parameters
* @param requirementsa string representing the expression describing all the Job requirements (which is an attribute of boolean type)
* @param rank a string representing the expression for the rank (which is an attribute of double type) of the resource
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the JDL string representation of the parametric job
*/
std::string getStringParametricJobTemplate (std::vector<std::string>attributes, std::vector<std::string> parameters, const std::string &requirements,const std::string &rank, ConfigContext *cfs=NULL);
/**
* Create a delegation identifier for the current proxy certificate. This method must be followed by a putProxy call
* @param delegationId the id of the delegation to be created
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @return the string representing the request, which has to be used as input while performing a putProxy for the created delegation Id
*/
std::string getProxyReq(const std::string &delegationId, ConfigContext *cfs=NULL);

/**
* Actually associate the current proxy certificate file with a previously created delegation id.This method must be called after a getProxyReq call
* @param delegationId the id of the delegation created previously (by a getProxyReq call)
* @param cfs define configuration context if non-default parameter(s) used (NULL otherwise)
* @param request the request output of a getProxyReq
*/
void putProxy(const std::string &delegationId, const std::string &request, ConfigContext *cfs=NULL);
} // wmproxy namespace
} // wms namespace
} // glite namespace
#endif
//EOF

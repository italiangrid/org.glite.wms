/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpoperations.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <fstream>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

// Boost
#include <boost/lexical_cast.hpp>
#include "boost/filesystem/path.hpp"

#include "wmpcommon.h"
#include "wmpoperations.h"
#include "wmpconfiguration.h"

#include "wmpstructconverter.h"

#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

// Utilities
#include "utilities/wmputils.h"

// Eventlogger
#include "eventlogger/wmpexpdagad.h"
#include "eventlogger/wmpeventlogger.h"

// Authorizer
#include "authorizer/wmpauthorizer.h"
#include "authorizer/wmpgaclmanager.h"
#include "authorizer/wmpvomsauthz.h"

//Logger
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "commands/listfiles.h"

// Delegation
#include "wmpdelegation.h"

// WMP Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

// RequestAd
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/jdl/PrivateAttributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/DAGAdManipulation.h"
#include "glite/jdl/RequestAdExceptions.h"

// Logging and Bookkeeping
#include "glite/lb/JobStatus.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"


//namespace glite {
//namespace wms {
//namespace wmproxy {
//namespace server {
	
// Global variables for configuration
extern WMProxyConfiguration conf;

// Global variables for configuration attributes (ENV dependant)
extern std::string sandboxdir_global;

// WMProxy software version
const std::string WMP_MAJOR_VERSION = "2";
const std::string WMP_MINOR_VERSION = "2";
const std::string WMP_RELEASE_VERSION = "0";
const std::string WMP_POINT_VERSION = ".";
const std::string WMP_VERSION = WMP_MAJOR_VERSION
	+ WMP_POINT_VERSION + WMP_MINOR_VERSION
	+ WMP_POINT_VERSION + WMP_RELEASE_VERSION;

// DONE job output file
const std::string MARADONA_FILE = "Maradona.output";

// Perusal functionality
const std::string PERUSAL_FILE_2_PEEK_NAME = "files2peek";
const std::string TEMP_PERUSAL_FILE_NAME = "tempperusalfile";
const std::string PERUSAL_DATE_INFO_SEPARATOR = "-";

// File size limit of globus URL copy
const long FILE_TRANSFER_SIZE_LIMIT = 2147000000; 
// 2 Giga = 2 * 1.073.741.824 = 2.147.483.648


// Defining File Separator
#ifdef WIN 
   // Windows File Separator 
   const std::string FILE_SEPARATOR = "\\"; 
#else 
   // Linux File Separator 
   const std::string FILE_SEPARATOR = "/";
#endif

const std::string ALL_PROTOCOLS = "all";
const std::string DEFAULT_PROTOCOL = "default";


using namespace std;
using namespace glite::lb; // JobStatus
using namespace glite::jdl; // DagAd, AdConverter
using namespace boost::details::pool; //singleton
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

using namespace glite::wms::wmproxy::server;  //Exception codes
using namespace glite::wms::wmproxy::utilities; //Exception
using namespace glite::wms::wmproxy::eventlogger;
using namespace glite::wms::common::configuration; // Configuration

namespace logger         = glite::wms::common::logger;
namespace configuration  = glite::wms::common::configuration;

namespace wmputilities	 = glite::wms::wmproxy::utilities;
namespace authorizer 	 = glite::wms::wmproxy::authorizer;
namespace eventlogger    = glite::wms::wmproxy::eventlogger;



//
// WM Web Service available operations
//

// To get more information see WMProxy service wsdl file
void
getVersion(getVersionResponse &getVersion_response)
{
	GLITE_STACK_TRY("getVersion()");
	edglog_fn("wmpoperations::getVersion");
	logRemoteHostInfo();
	
	getVersion_response.version = WMP_VERSION;
	edglog(info)<<"Version retrieved: "<<getVersion_response.version<<endl;
	
	GLITE_STACK_CATCH();
}

void
getJDL(const std::string &job_id, JdlType jdltype,
	getJDLResponse &getJDL_response)
{
	GLITE_STACK_TRY("getJDL()");
	edglog_fn("wmpoperations::getJDL");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::getJDL()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	getJDL_response.jdl = "";
	switch (jdltype) {
		case WMS_JDL_ORIGINAL:
			getJDL_response.jdl = wmputilities::readTextFile(
				wmputilities::getJobJDLOriginalPath(*jid));
			break;
		case WMS_JDL_REGISTERED:
			getJDL_response.jdl = wmputilities::readTextFile(
				wmputilities::getJobJDLToStartPath(*jid));
			break;
		default:
			break;
	}
	
	delete jid;
	
	edglog(info)<<"JDL retrieved: "<<getJDL_response.jdl<<endl;
	
	GLITE_STACK_CATCH();
}

void
getMaxInputSandboxSize(getMaxInputSandboxSizeResponse
	&getMaxInputSandboxSize_response)
{
	GLITE_STACK_TRY("getMaxInputSandboxSize()");
	edglog_fn("wmpoperations::getMaxInputSandboxSize");
	logRemoteHostInfo();

	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	try {
		getMaxInputSandboxSize_response.size =
		// WARNING: Temporal cast TBD
		// WARNING: double temporarely casted into long (soon long will be returned directly
		(long)singleton_default<WMProxyConfiguration>::instance()
			.wmp_config->max_input_sandbox_size();
	} catch (exception &ex) {
		edglog(severe)<<"Unable to get max input sandbox size: "
			<<ex.what()<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getMaxInputSandboxSize()", wmputilities::WMS_IS_FAILURE,
			"Unable to get max input sandbox size");
	}

	GLITE_STACK_CATCH();
}

void
getSandboxDestURI(getSandboxDestURIResponse &getSandboxDestURI_response,
	const string &jid, const string &protocol)
{
	GLITE_STACK_TRY("getSandboxDestURI()");
	edglog_fn("wmpoperations::getSandboxDestURI");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	

	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	getSandboxDestURI_response.path = new StringList;
	getSandboxDestURI_response.path->Item = new vector<string>(0);
	
	vector<string> *jobdiruris = getJobDirectoryURIsVector(conf.getProtocols(),
		conf.getDefaultProtocol(), conf.getDefaultPort(), conf.getHTTPSPort(),
		jid, protocol, "input");
	
	getSandboxDestURI_response.path->Item = jobdiruris;
	
	GLITE_STACK_CATCH();
}

void
getSandboxBulkDestURI(getSandboxBulkDestURIResponse &getSandboxBulkDestURI_response,
	const string &jid, const string &protocol)
{
	GLITE_STACK_TRY("getSandboxBulkDestURI()");
	edglog_fn("wmpoperations::getSandboxBulkDestURI");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	// Initializing logger
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jobid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);
	
	DestURIsStructType *destURIsStruct = new DestURIsStructType();
	destURIsStruct->Item = new vector<DestURIStructType*>(0);
	
	// Getting job status to check if cancellation is possible
	JobStatus status = wmplogger.getStatus(true);
	vector<string> jids = status.getValStringList(JobStatus::CHILDREN);
	edglog(debug)<<"Children count: "<<status.getValInt(JobStatus::CHILDREN_NUM)<<endl;
	vector<string>::iterator jidsiterator = jids.begin();
 	jids.insert(jidsiterator, 1, jid);
 	
	vector<string>::iterator iter = jids.begin();
	vector<string>::iterator const end = jids.end();
	for (; iter != end; ++iter) {
		vector<string> *uris = getJobDirectoryURIsVector(conf.getProtocols(),
			conf.getDefaultProtocol(), conf.getDefaultPort(), conf.getHTTPSPort(),
			*iter, protocol, "input");
		
		DestURIStructType *destURIStruct = new DestURIStructType();
		destURIStruct->id = *iter;
		destURIStruct->destURIs = uris;
		destURIsStruct->Item->push_back(destURIStruct);
	}
	getSandboxBulkDestURI_response.destURIsStruct = destURIsStruct;
	
	GLITE_STACK_CATCH();
}

void
getQuota(getQuotaResponse &getQuota_response)
{
	GLITE_STACK_TRY("getQuota()");
	edglog_fn("wmpoperations::getQuota");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize(wmputilities::getEnvFQAN());
	edglog(debug)<<"User Name: "<<auth->getUserName()<<endl;
	
	pair<long, long> quotas;
	if (!wmputilities::getUserQuota(quotas, auth->getUserName())) {
		edglog(severe)<<"Unable to get total quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__,
			"getQuota()", wmputilities::WMS_IS_FAILURE,
			"Unable to get total quota");
	}
	delete auth;
	getQuota_response.softLimit = quotas.first;
	getQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
getFreeQuota(getFreeQuotaResponse &getFreeQuota_response)
{
	GLITE_STACK_TRY("getFreeQuota()");
	edglog_fn("wmpoperations::getFreeQuota");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize(wmputilities::getEnvFQAN());
	edglog(debug)<<"User Name: "<<auth->getUserName()<<endl;
	
	pair<long, long> quotas;
	if (!wmputilities::getUserFreeQuota(quotas, auth->getUserName())) {
		edglog(severe)<<"Unable to get free quota"<<endl;
		throw FileSystemException(__FILE__, __LINE__, "getFreeQuota()",
			wmputilities::WMS_IS_FAILURE, "Unable to get free quota");
	}
	delete auth;
	getFreeQuota_response.softLimit = quotas.first;
	getFreeQuota_response.hardLimit = quotas.second;
	
	GLITE_STACK_CATCH();
}

void
getOutputFileList(getOutputFileListResponse &getOutputFileList_response,
	const string &jid, const string &protocol)
{
	GLITE_STACK_TRY("getOutputFileList()");
	edglog_fn("wmpoperations::getOutputFileList");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<jid<<endl;
	
	JobId *jobid = new JobId(jid);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jobid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, jid);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), jid);
	} else {
		auth->authorize("", jid);
	}
	delete auth;
	
	string jobdirectory = wmputilities::getJobDirectoryPath(*jobid);

	// Checking for maradona file, created if and only if the job is in DONE state
	edglog(debug)<<"Searching for file: "<<jobdirectory + FILE_SEPARATOR
		+ MARADONA_FILE<<endl;
	if (!wmputilities::fileExists(jobdirectory + FILE_SEPARATOR + MARADONA_FILE)) {
		// Initializing logger
		WMPEventLogger wmplogger(wmputilities::getEndpoint());
		std::pair<std::string, int> lbaddress_port
			= conf.getLBLocalLoggerAddressPort();
		wmplogger.init(lbaddress_port.first, lbaddress_port.second, jobid,
			conf.getDefaultProtocol(), conf.getDefaultPort());
		wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
		
		// Setting user proxy
		wmplogger.setUserProxy(delegatedproxy);
		
		// Getting job status to check if is a job and is done success
		JobStatus status = wmplogger.getStatus(false);
	
		if (status.getValInt(JobStatus::CHILDREN_NUM) != 0) {
			string msg = "getOutputFileList operation not allowed for dag or "
				"collection type";
			edglog(error)<<msg<<": "<<jobid<<endl;
			throw JobOperationException(__FILE__, __LINE__, "getOutputFileList()", 
				wmputilities::WMS_OPERATION_NOT_ALLOWED, msg);
		}
		
		if (((status.status == JobStatus::DONE)
				&& (status.getValInt(JobStatus::DONE_CODE)
					== JobStatus::DONE_CODE_FAILED))
				|| ((status.status != JobStatus::DONE)
					&& (status.status != JobStatus::ABORTED))) {
			edglog(error)<<
				"Job current status doesn't allow getOutputFileList operation"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"getOutputFileList()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Job current status doesn't allow getOutputFileList operation");
		}
	}
	
	delete jobid;
	
	string outputpath = wmputilities::getOutputSBDirectoryPath(jid);
	vector<string> *jobdiruris = getJobDirectoryURIsVector(conf.getProtocols(),
		conf.getDefaultProtocol(), conf.getDefaultPort(), conf.getHTTPSPort(),
		jid, protocol, "output");
	unsigned int jobdirsize = jobdiruris->size();
	
	// Searching files inside directory
	const boost::filesystem::path p(outputpath, boost::filesystem::native);
	std::vector<std::string> found;
	glite::wms::wmproxy::commands::list_files(p, found);
	edglog(debug)<<"List size is (hidden files included): "
		<<found.size()<<endl;
	
	// Creating the list
	StringAndLongList *list = new StringAndLongList();
	vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
	StringAndLongType *item = NULL;
	string filename;
	long filesize;
	
	vector<string>::iterator iter = found.begin();
	vector<string>::iterator const end = found.end();
	for (; iter != end; ++iter) {
		// Checking for hidden files
		filename = wmputilities::getFileName(string(*iter));
		if ((filename != MARADONA_FILE) && (filename
				!= authorizer::GaclManager::WMPGACL_DEFAULT_FILE)) {
			filesize = wmputilities::computeFileSize(*iter);
			edglog(debug)<<"Inserting file name: " <<filename<<endl;
			edglog(debug)<<"Inserting file size: " <<filesize<<endl;
			for (unsigned int i = 0; i < jobdirsize; i++) {
				item = new StringAndLongType();
				item->name = (*jobdiruris)[i] + FILE_SEPARATOR + filename;
				item->size = filesize;
				file->push_back(item);
			}
		}
	}
	list->file = file;
	getOutputFileList_response.OutputFileAndSizeList = list;
	
	edglog(info)<<"Successfully retrieved files: "<<found.size()<<endl;
	
	GLITE_STACK_CATCH();
}
	
void
getJobTemplate(getJobTemplateResponse &getJobTemplate_response,
	JobTypeList jobType, const string &executable,
	const string &arguments, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getJobTemplate()");
	edglog_fn("wmpoperations::getJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getJobTemplate_response.jdl =
		(AdConverter::createJobTemplate(convertJobTypeListToInt(jobType),
		executable, arguments, requirements, rank))->toString();
		
	edglog(info)<<"Job Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getDAGTemplate(getDAGTemplateResponse &getDAGTemplate_response,
	GraphStructType dependencies, const string &requirements,
	const string &rank)
{
	GLITE_STACK_TRY("getDAGTemplate()");
	edglog_fn("wmpoperations::getDAGTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getDAGTemplate_response.jdl = AdConverter::createDAGTemplate(
		convertGraphStructTypeToNodeStruct(dependencies),
		requirements, rank)->toString();
		
	edglog(info)<<"DAG Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getCollectionTemplate(getCollectionTemplateResponse
	&getCollectionTemplate_response, int jobNumber,
	const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getCollectionTemplate()");
	edglog_fn("wmpoperations::getCollectionTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;

	getCollectionTemplate_response.jdl =
		AdConverter::createCollectionTemplate(jobNumber, requirements,
			rank)->toString();
			
	edglog(info)<<"Collection Template retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}

void
getIntParametricJobTemplate(getIntParametricJobTemplateResponse 
	&getIntParametricJobTemplate_response, StringList *attributes, int param,
	int parameterStart, int parameterStep, const string &requirements,
	const string &rank)
{
	GLITE_STACK_TRY("getIntParametricJobTemplate()");
	edglog_fn("wmpoperations::getIntParametricJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getIntParametricJobTemplate_response.jdl =
		AdConverter::createIntParametricTemplate(*(attributes->Item), param,
			parameterStart, parameterStep, requirements, rank)->toString();
			
	edglog(info)<<"Int Parametric Job Template retrieved successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getStringParametricJobTemplate(getStringParametricJobTemplateResponse
	&getStringParametricJobTemplate_response, StringList *attributes,
	StringList *param, const string &requirements, const string &rank)
{
	GLITE_STACK_TRY("getStringParametricJobTemplate()");
	edglog_fn("wmpoperations::getStringParametricJobTemplate");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getStringParametricJobTemplate_response.jdl =
		AdConverter::createStringParametricTemplate(*(attributes->Item),
		*(param->Item), requirements, rank)->toString();
		
	edglog(info)<<"String Parametric Job Template retrieved successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getProxyReq(getProxyReqResponse &getProxyReq_response,
	const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyReq()");
	edglog_fn("wmpoperations::getProxyReq");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyReq()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	getProxyReq_response.request =
		WMPDelegation::getProxyRequest(delegation_id);
	edglog(info)<<"Proxy requested successfully"<<endl;
	
	GLITE_STACK_CATCH();
}
/*
void
getNewProxyReq(pair<string, string> &retpair)
{
	GLITE_STACK_TRY("getNewProxyReq()");
	edglog_fn("wmpoperations::getNewProxyReq");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	retpair =
		WMPDelegation::getNewProxyRequest();
	edglog(debug)<<"____ retpair.1: "<<retpair.first<<endl;
	edglog(debug)<<"____ retpair.2: "<<retpair.second<<endl;
	
	edglog(info)<<"Proxy requested successfully"<<endl;
	
	GLITE_STACK_CATCH();
}
*/
void
putProxy(putProxyResponse &putProxyReq_response, const string &delegation_id,
	const string &proxy)
{
	GLITE_STACK_TRY("putProxy()");
	edglog_fn("wmpoperations::putProxy");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"putProxy()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	WMPDelegation::putProxy(delegation_id, proxy);
	edglog(info)<<"Proxy put successfully"<<endl;
	
	GLITE_STACK_CATCH();
}
/*
void
destroyProxy(const string &delegation_id)
{
	GLITE_STACK_TRY("destroyProxy()");
	edglog_fn("wmpoperations::destroyProxy");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"destroyProxy()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	WMPDelegation::destroyProxy(delegation_id);
	edglog(info)<<"destroyProxy successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getProxyTerminationTime(getProxyTerminationTimeResponse &getProxyTerminationTime_response,
	const string &delegation_id)
{
	GLITE_STACK_TRY("getProxyTerminationTime()");
	edglog_fn("wmpoperations::getProxyTerminationTime");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	if (delegation_id == "") {
		edglog(error)<<"Provided delegation id not valid"<<endl;
  		throw ProxyOperationException(__FILE__, __LINE__,
			"getProxyTerminationTime()", wmputilities::WMS_DELEGATION_ERROR,
			"Provided delegation id not valid");
	}
	
	getProxyTerminationTime_response = WMPDelegation::getTerminationTime(delegation_id);
	edglog(info)<<"getProxyTerminationTime successfully"<<endl;
	
	GLITE_STACK_CATCH();
}
*/
void
getACLItems(getACLItemsResponse &getACLItems_response, const string &job_id)
{
	GLITE_STACK_TRY("getACLItems()");
	edglog_fn("wmpoperations::getACLItems");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
		
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::addACLItems()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	string jobpath = wmputilities::getJobDirectoryPath(*jid);
	
	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	getACLItems_response =
		gaclmanager.getItems(authorizer::GaclManager::WMPGACL_PERSON_TYPE);
	
	edglog(info)<<"getACLItems successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
addACLItems(addACLItemsResponse &addACLItems_response, const string &job_id,
	StringList * dnlist)
{
	GLITE_STACK_TRY("addACLItems()");
	edglog_fn("wmpoperations::addACLItems");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"
			<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::addACLItems()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	string jobpath = wmputilities::getJobDirectoryPath(*jid);

	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
		
	vector<pair<authorizer::GaclManager::WMPgaclCredType, string> > gaclvect;
	pair<authorizer::GaclManager::WMPgaclCredType, string> gaclpair;
	for (unsigned int i = 0; i < dnlist->Item->size(); i++) {
		gaclpair.first = authorizer::GaclManager::WMPGACL_PERSON_TYPE;
		edglog(info)<<"Item to add: "<<(*(dnlist->Item))[i]<<endl;
		gaclpair.second = (*(dnlist->Item))[i];
		gaclvect.push_back(gaclpair);
	}
	gaclmanager.addEntries(gaclvect);
	gaclmanager.saveGacl();
	
	edglog(info)<<"addACLItems successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
removeACLItem(removeACLItemResponse &removeACLItem_response,
	const string &job_id, const string &item)
{
	GLITE_STACK_TRY("removeACLItem()");
	edglog_fn("wmpoperations::removeACLItem");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	string errors = "";
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::removeACLItem()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	// TBD change test in: item == owner
	if (item == wmputilities::getUserDN()) {
		edglog(error)<<"Removal of the item representing user that has "
			"registered the job is not allowed"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"removeACLItem()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
			"Removal of the item representing user that has registered the "
			"job is not allowed");
	}
	string jobpath = wmputilities::getJobDirectoryPath(*jid);
	
	edglog(debug)<<"GACL File: "<<jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE<<endl;
	authorizer::GaclManager gaclmanager(jobpath + FILE_SEPARATOR
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	gaclmanager.removeEntry(authorizer::GaclManager::WMPGACL_PERSON_TYPE,
		item, errors);
		
	if (errors.size( )>0) { 
       edglog(error)<<"Removal of the gacl item failed: " << errors << "\n"; 
       throw JobOperationException(__FILE__, __LINE__, 
               "removeACLItem()", wmputilities::WMS_AUTHZ_ERROR, 
               "Removal of the gacl item failed:\n" + errors); 
	} 
	
		
	gaclmanager.saveGacl();
	
	edglog(info)<<"removeACLItem successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

// getDelegatedProxyInfo and getJobProxyInfo
void
getProxyInfo(getProxyInfoResponse &getProxyInfo_response, const string &id,
	bool isjobid)
{
	GLITE_STACK_TRY("getProxyInfo()");
	edglog_fn("wmpoperations::getProxyInfo");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	string proxy;
	if (isjobid) {
		edglog(info)<<"Job Id: "<<id<<endl;
		// Checking delegation id
		if (id == "") {
			edglog(error)<<"Provided Job Id not valid"<<endl;
	  		throw JobOperationException(__FILE__, __LINE__,
				"getProxyInfo()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Provided Job Id not valid");
		}
		auth->authorize("", id);
		proxy = wmputilities::getJobDelegatedProxyPath(id);
		authorizer::WMPAuthorizer::checkProxyExistence(proxy, id);
		edglog(debug)<<"Job proxy: "<<proxy<<endl;
	} else {
		edglog(info)<<"Delegation Id: "<<id<<endl;
		auth->authorize();
		// Checking delegation id
		if (id == "") {
			edglog(error)<<"Provided Delegation Id not valid"<<endl;
	  		throw JobOperationException(__FILE__, __LINE__,
				"getProxyInfo()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Provided Delegation Id not valid");
		}
		proxy = WMPDelegation::getDelegatedProxyPath(id);
		edglog(debug)<<"Delegated Proxy: "<<proxy<<endl;
	}
	delete auth;
	
	
	authorizer::VOMSAuthZ vomsproxy = authorizer::VOMSAuthZ(proxy);
	getProxyInfo_response.items = vomsproxy.getProxyInfo();
	
	edglog(info)<<"getProxyInfo successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
checkPerusalFlag(JobId *jid, string &delegatedproxy, bool checkremotepeek)
{
	GLITE_STACK_TRY("checkPerusalFlag()");
	edglog_fn("wmpoperations::checkPerusalFlag");
	
	WMPEventLogger wmplogger(wmputilities::getEndpoint());
	std::pair<std::string, int> lbaddress_port = conf.getLBLocalLoggerAddressPort();
	wmplogger.init(lbaddress_port.first, lbaddress_port.second, jid,
		conf.getDefaultProtocol(), conf.getDefaultPort());
	wmplogger.setLBProxy(conf.isLBProxyAvailable(), wmputilities::getUserDN());
	
	// Setting user proxy
	wmplogger.setUserProxy(delegatedproxy);
	
	string jdlpath = wmplogger.retrieveRegJobEvent(jid->toString()).jdl;
	edglog(debug)<<"jdlpath: "<<jdlpath<<endl;
	if (jdlpath == "") {
		edglog(critical)<<"No Register event found quering LB; unable to get "
			"registered jdl"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"checkPerusalFlag()", wmputilities::WMS_IS_FAILURE,
			"Unable to check perusal availability"
			"\n(please contact server administrator)");
	}
	
	string jdl = wmputilities::readTextFile(jdlpath);
	edglog(debug)<<"Jdl: "<<jdl<<endl;
	
	int type = getType(jdl);
	if (type == TYPE_JOB) {
		JobAd * jad = new JobAd(jdl);
		jad->setLocalAccess(false);
		
		if (jad->hasAttribute(JDL::PU_FILE_ENABLE)) {
			if (!jad->getBool(JDL::PU_FILE_ENABLE)) {
				edglog(debug)<<"Perusal not enabled for this job"<<endl;
				throw JobOperationException(__FILE__, __LINE__,
			    	"checkPerusalFlag()",
			    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
			    	"Perusal not enabled for this job");
			}
		} else {
			edglog(debug)<<"Perusal not enabled for this job"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
		    	"checkPerusalFlag()",
		    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
		    	"Perusal not enabled for this job");
		}
		if (checkremotepeek) {
			if (jad->hasAttribute(JDL::PU_FILES_DEST_URI)) {
				string protocol = conf.getDefaultProtocol();
				string port = (conf.getDefaultPort() != 0)
					? (":" + boost::lexical_cast<std::string>(
						conf.getDefaultPort()))
					: "";
				string serverhost = getServerHost();
				string uri = jad->getString(JDL::PU_FILES_DEST_URI);
				
				string tofind = "://" + serverhost;
				unsigned int pos = uri.find(tofind);
				if (pos == string::npos) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Remote perusal peek URI set:\n" + uri);
				}
				string uriprotocol = uri.substr(0, pos);
				edglog(debug)<<"URI protocol: "<<uriprotocol<<endl;
				string uripath = uri.substr(pos + tofind.length(),
					uri.length() - 1);
				edglog(debug)<<"URI path: "<<uripath<<endl;
				pos = uripath.find("/");
				// Looking for port
				if (pos == string::npos) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Remote perusal peek URI set:\n" + uri);
				}
				uripath = uripath.substr(pos, uripath.length() - 1);
				edglog(debug)<<"URI path: "<<uripath<<endl;
				
				string serverpath;
				if ((uriprotocol == "https") || (uriprotocol == "http")) {
					// No document root added
					serverpath = wmputilities::getPeekDirectoryPath(*jid, false);
				} else {
					serverpath = wmputilities::getPeekDirectoryPath(*jid);	
				}
				edglog(debug)<<"Server path: "<<serverpath<<endl;
				if (uripath != serverpath) {
					edglog(debug)<<"Remote perusal peek URI set:\n"
						<<uri<<endl;
					throw JobOperationException(__FILE__, __LINE__,
				    	"checkPerusalFlag()",
				    	wmputilities::WMS_OPERATION_NOT_ALLOWED, 
				    	"Perusal peek URI refers to server host, but the path "
				    		"is not manged by WMProxy:\n" + uri);
				}
			}
		}
		
		delete jad;
	} else {
		edglog(debug)<<"Perusal service not available for dag or collection type"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
	    	"checkPerusalFlag()", wmputilities::WMS_OPERATION_NOT_ALLOWED, 
	    	"Perusal service not available for dag or collection type");
	}
	
	GLITE_STACK_CATCH();
}

void
enableFilePerusal(enableFilePerusalResponse &enableFilePerusal_response,
	const string &job_id, StringList * fileList)
{
	GLITE_STACK_TRY("enableFilePerusal()");
	edglog_fn("wmpoperations::enableFilePerusal");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if (authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::enableFilePerusal()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	checkPerusalFlag(jid, delegatedproxy, false);
	
	string filename = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR
		+ PERUSAL_FILE_2_PEEK_NAME;
	
	unsigned int size = fileList->Item->size();
	if (size != 0) {
		fstream file2peek(filename.c_str(), ios::out);
		for (unsigned int i = 0; i < size; i++) {
	    	file2peek << (*(fileList->Item))[i] + "\n";
		}
	    file2peek.close();
	} else {
		// Removing file2peek file if needed
		if (wmputilities::fileExists(filename)) {
			remove(filename.c_str());
		}
	}

	edglog(info)<<"enableFilePerusal successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getPerusalFiles(getPerusalFilesResponse &getPerusalFiles_response,
	const string &job_id, const string &fileName, bool allChunks,
	const string &protocol)
{
	GLITE_STACK_TRY("getPerusalFiles()");
	edglog_fn("wmpoperations::getPerusalFiles");
	logRemoteHostInfo();
	edglog(info)<<"Operation requested for job: "<<job_id<<endl;
	
	JobId *jid = new JobId(job_id);
	
	//** Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	
	// Getting delegated proxy inside job directory
	string delegatedproxy = wmputilities::getJobDelegatedProxyPath(*jid);
	edglog(debug)<<"Job delegated proxy: "<<delegatedproxy<<endl;
	
	authorizer::WMPAuthorizer::checkProxyExistence(delegatedproxy, job_id);
	authorizer::VOMSAuthZ vomsproxy(delegatedproxy);
	if (vomsproxy.hasVOMSExtension()) {
		auth->authorize(vomsproxy.getDefaultFQAN(), job_id);
	} else {
		auth->authorize("", job_id);
	}
	delete auth;

	// GACL Authorizing
	edglog(debug)<<"Checking for drain..."<<endl;
	if ( authorizer::WMPAuthorizer::checkJobDrain ( ) ) {
		edglog(error)<<"Unavailable service (the server is temporarily drained)"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_AUTHZ_ERROR, 
	    	"Unavailable service (the server is temporarily drained)");
	} else {
		edglog(debug)<<"No drain"<<endl;
	}
	//** END
	
	if (fileName == "") {
		throw JobOperationException(__FILE__, __LINE__,
	    	"wmpoperations::getPerusalFiles()", wmputilities::WMS_INVALID_ARGUMENT, 
	    	"Provided file name not valid");
	}
	
	checkPerusalFlag(jid, delegatedproxy, true);

	string peekdir = wmputilities::getPeekDirectoryPath(*jid) + FILE_SEPARATOR;
	const boost::filesystem::path p(peekdir, boost::filesystem::native);
	vector<string> found;
	glite::wms::wmproxy::commands::list_files(p, found);
	
	vector<string> *jobdiruris = getJobDirectoryURIsVector(conf.getProtocols(),
		conf.getDefaultProtocol(), conf.getDefaultPort(), conf.getHTTPSPort(),
		job_id, protocol, "peek/");

	unsigned int jobdirsize = jobdiruris->size();
		
	vector<string> good;
	vector<string> returnvector;
	string currentfilename;
	
	vector<string>::iterator iter = found.begin();
	vector<string>::iterator const end = found.end();
	for (; iter != end; ++iter) {
		currentfilename = wmputilities::getFileName(*iter);
		if (currentfilename.find(fileName + ".") == 0) {
			edglog(debug)<<"Good perusal file: "<<*iter<<endl;
			good.push_back(*iter);
		}
		if (allChunks) {
			if (currentfilename.find(fileName + PERUSAL_DATE_INFO_SEPARATOR)
					== 0) {
				edglog(debug)<<"Good old global perusal file: "<<*iter<<endl;
				for (unsigned int i = 0; i < jobdirsize; i++) {
					returnvector.push_back((*jobdiruris)[i] + currentfilename);
				}
			}
		}
	}
	
	//unsigned int size = good.size();
	if (good.size()) {
		// Sorting vector
		sort(good.begin(), good.end());
		
		string tempfile = peekdir + TEMP_PERUSAL_FILE_NAME;
		
		long filesize = 0;
		long totalfilesize = 0;
		
		string startdate = good[0].substr(good[0].rfind(".") + 1,
			good[0].length() - 1);
		string enddate = startdate;
			
		fstream outfile(tempfile.c_str(), ios::out);
		if (!outfile.good()) {
			edglog(severe)<<tempfile<<": !outfile.good()"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
				"temporary file\n(please contact server administrator)");
		}
		
		string filetoreturn;
		vector<string>::iterator iter = good.begin();
		vector<string>::iterator const end = good.end();
		for (; iter != end; ++iter) {
			filesize = wmputilities::computeFileSize(*iter);
			filetoreturn = fileName
				+ PERUSAL_DATE_INFO_SEPARATOR + startdate
				+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
			if ((totalfilesize + filesize) > FILE_TRANSFER_SIZE_LIMIT) {
				outfile.close();
				rename(tempfile.c_str(), (peekdir + filetoreturn).c_str());
				
				for (unsigned int i = 0; i < jobdirsize; i++) {
					returnvector.push_back((*jobdiruris)[i] + filetoreturn);
				}
				
				outfile.open(tempfile.c_str(), ios::out);
				if (!outfile.good()) {
					edglog(severe)<<tempfile<<": !outfile.good()"<<endl;
					throw FileSystemException(__FILE__, __LINE__,
						"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
						"temporary file\n(please contact server administrator)");
				}
				totalfilesize = 0;
				enddate = (*iter).substr((*iter).rfind(".") + 1,
                	(*iter).length() - 1);
				startdate = enddate;
			} else {
				enddate = (*iter).substr((*iter).rfind(".") + 1,
                	(*iter).length() - 1);
			}
			ifstream infile((*iter).c_str());
			if (!infile.good()) {
				edglog(severe)<<*iter<<": !infile.good()"<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"getPerusalFiles()", WMS_IS_FAILURE, "Unable to open perusal "
					"input file\n(please contact server administrator)");
			}
			outfile << infile.rdbuf();
			infile.close();
			remove((*iter).c_str());
			totalfilesize += filesize;
		}
		outfile.close();
		filetoreturn = fileName
			+ PERUSAL_DATE_INFO_SEPARATOR + startdate
			+ PERUSAL_DATE_INFO_SEPARATOR + enddate;
		rename(tempfile.c_str(), (peekdir + filetoreturn).c_str());
		
		for (unsigned int i = 0; i < jobdirsize; i++) {
			returnvector.push_back((*jobdiruris)[i] + filetoreturn);
		}
	}
	
	getPerusalFiles_response = returnvector;
	
	edglog(info)<<"getPerusalFiles successfully"<<endl;
	
	GLITE_STACK_CATCH();
}

void
getTransferProtocols(getTransferProtocolsResponse &getTransferProtocols_response)
{
	GLITE_STACK_TRY("getTransferProtocols()");
	edglog_fn("wmpoperations::getTransferProtocols");
	logRemoteHostInfo();
	
	// Authorizing user
	edglog(info)<<"Authorizing user..."<<endl;
	authorizer::WMPAuthorizer *auth = 
		new authorizer::WMPAuthorizer();
	auth->authorize();
	delete auth;
	
	getTransferProtocols_response.protocols = new StringList;
	vector<string> *protocols = new vector<string>();
	//getTransferProtocols_response.protocols->Item = new vector<string>(0);
	
	WMProxyConfiguration conf = singleton_default<WMProxyConfiguration>::instance();
	vector<pair<string, int> > serverprotocols = conf.getProtocols();
	unsigned int size = serverprotocols.size();
	for (unsigned int i = 0; i < size; i++) {
		protocols->push_back(serverprotocols[i].first);
	}
	protocols->push_back("https");
	getTransferProtocols_response.protocols->Item = protocols;
		
	edglog(info)<<"Transfer protocols retrieved successfully"<<endl;
	GLITE_STACK_CATCH();
}
//} // server
//} // wmproxy
//} // wms
//} // glite

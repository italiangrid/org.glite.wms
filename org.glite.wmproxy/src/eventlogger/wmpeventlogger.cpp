/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
#include "wmpeventlogger.h"
#include "glite/lb/producer.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/utilities/boost_fs_add.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
// edglog macro definitions
#include "utilities/logging.h"
#include "utilities/wmpexception_codes.h"
#include "utilities/wmpexceptions.h"
#include "utilities/wmputils.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/security/proxyrenewal/renewal.h"

// gethostbyname inclusion:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
namespace logger        = glite::wms::common::logger;
namespace jobid = glite::wmsutils::jobid;


using namespace std;
using namespace glite::wms::wmproxy::utilities ;  //Exception codes
using namespace glite::wms::jdl; // DagAd
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

const char *WMPEventLogger::GLITE_WMS_LOG_DESTINATION = "GLITE_WMS_LOG_DESTINATION";

std::string retrieveHostName() {
	struct hostent hent, *hp;
	size_t hstbuflen = 1024;
	char *tmphstbuf;
	char hostname[101];
	int res;
	int herr;

	edglog_fn("CFSI::retrHostName");
	if (gethostname(hostname, 100) == -1) {
		edglog(severe) << "Error while retrieving host name." << std::endl;
	}
	tmphstbuf = (char *)malloc (hstbuflen);
	while ( (res = gethostbyname_r(hostname, &hent, tmphstbuf, hstbuflen, &hp,&herr)) == ERANGE ) {
		hstbuflen *= 2;
		tmphstbuf = (char *)realloc (tmphstbuf, hstbuflen);
	}
	free(tmphstbuf);
	if (res || hp == NULL) {
		return "";
	}
	edglog(debug) << "Hostname: " << hp->h_name << std::endl;
	return std::string(hp->h_name);
}




WMPEventLogger::WMPEventLogger()
{
	id = NULL;
	if (edg_wll_InitContext(&ctx)
			|| (edg_wll_SetParam(ctx, EDG_WLL_PARAM_SOURCE,
			EDG_WLL_SOURCE_WM_PROXY))) {
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::WMPEventLogger()",
			WMS_IS_FAILURE, "LB initialisation failed");
	}
};

WMPEventLogger::~WMPEventLogger() throw()
{
	edg_wll_FreeContext(ctx);
}




void
WMPEventLogger::init(const string &lb_host, int lb_port, jobid::JobId *id)
{
	this->id = id;
	this->lb_host = lb_host;
	this->lb_port = lb_port;
	if (!getenv(GLITE_WMS_LOG_DESTINATION)) {
		if (edg_wll_SetParamString(ctx, EDG_WLL_PARAM_DESTINATION,
				lb_host.c_str())) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::init(const string& lb_host, int lb_port, "
					"jobid::JobId *id)",
				WMS_OPERATION_NOT_ALLOWED, "LB initialisation failed "
					"(set destination)");
		}
	}
}

std::string
WMPEventLogger::getSequence()
{
	return std::string(edg_wll_GetSequenceCode(ctx));
};


// Registering methods

void
WMPEventLogger::registerProxyRenewal(const string &proxy_path,
	const string &my_proxy_server)
{
	char *renewal_proxy_path = NULL;
	int i = 0;
	for (; i < LOG_RETRY_COUNT
		&& edg_wlpr_RegisterProxyExt((char*)proxy_path.c_str(),
	 			(char*)my_proxy_server.c_str(), LB_RENEWAL_PORT,
				id->getId(), EDG_WLPR_FLAG_UNIQUE,
      		    &renewal_proxy_path); i++);

 	if (i = LOG_RETRY_COUNT - 1) {
		for (int j = 0; j < LOG_RETRY_COUNT
	    	&& !edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY,
	    	renewal_proxy_path); j++);
	} else {
		for (int j = 0; j < LOG_RETRY_COUNT
	    	&& !edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY,
	    	proxy_path.c_str()); j++);
	   	throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerProxyRenewal(const string &proxy_path, const "
			"string &my_proxy_server)",
			WMS_LOGGING_ERROR, error_message("registerProxyRenewal"));
 	}
}

void
WMPEventLogger::unregisterProxyRenewal()
{
	char *renewal_proxy_path = NULL;
	cerr<<"---->> ID: "<<id->toString()<<endl;
	for (int i = 0; i < LOG_RETRY_COUNT
		&& edg_wlpr_UnregisterProxy(id->getId(), renewal_proxy_path); i++);
}

void
WMPEventLogger::registerJob(JobAd *jad)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	//jad->setAttribute(JDL::LB_SEQUENCE_CODE, getSequence());
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
		jad->toSubmissionString().c_str(), str_addr, 0, NULL, NULL)) {
		cerr<<"JobOperationException"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerJob(JobAd* jad)",
			WMS_OPERATION_NOT_ALLOWED,
			error_message("edg_wll_RegisterJobSync"));
	}
	if (jad->hasAttribute(JDL::USERTAGS)) {
		logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
	}
} 

void
WMPEventLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)
{
	char str_nsAddr[1024];
	sprintf(str_nsAddr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	vector<string> jdls = ad->getSubmissionStrings();
	
	char **jdls_char;
	char **zero_char;
	vector<string>::iterator iter;
	
	// Adding WMProxyDestURI and InputSandboxDestURI attributes
	JobId *jobid = NULL;
	string dest_uri;
	string jobidstring;
	int size = sizeof(subjobs) / sizeof(subjobs[0]);
	for (unsigned int i = 0; i < size; i++) {
		// Setting attribute for children jobs
		// method getting jobId, attr name, attr value
		// it should be recursive in the case the job is a dag!! //TBD
		jobidstring = edg_wlc_JobIdUnparse(subjobs[i]);
		jobid = new JobId(jobidstring);
	 	dest_uri = string(getenv("DOCUMENT_ROOT"))
			+ "/" + to_filename(*jobid);
		/*if (!ad->hasNodeAttribute(jobid, JDL::ISB_BASE_URI)) {
			ad->setNodeAttribute(jobid, JDL::ISB_BASE_URI, dest_uri);
		}				//TBD ** UNCOMMENT WHEN CODED **
		job_ad->setNodeAttribute(jobid, JDL::WMPROXY_BASE_URI, dest_uri);*/
	}
	
	jdls_char = (char**) malloc(sizeof(char*) * (jdls.size() + 1));
	zero_char = jdls_char;
	jdls_char[jdls.size()] = NULL;
	
	int i = 0;
	for (iter = jdls.begin(); iter != jdls.end(); iter++, i++) {
		*zero_char = (char*) malloc(iter->size() + 1);
		sprintf(*zero_char, "%s", iter->c_str());
		zero_char++;
	}
	if (edg_wll_RegisterSubjobs(ctx, id->getId(), jdls_char, str_nsAddr, 
			subjobs)) {
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)",
			WMS_OPERATION_NOT_ALLOWED,
			error_message("edg_wll_RegisterSubjobs"));
	}

	// Release Memory  REMOVE!!
	for (unsigned int i = 0 ; i < jdls.size() ; i++ ) {
		std::free(jdls_char[i]);
	}
    	std::free(jdls_char);
    	//delete []jdls_char;
}

vector<string>
WMPEventLogger::registerPartitionable(WMPExpDagAd *dag, int res_num)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	
	cerr<<"Registering partitionable job" << endl;
	dag->setAttribute(WMPExpDagAd::SEQUENCE_CODE, getSequence());
	edg_wlc_JobId *subjobs = NULL;
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_PARTITIONED,
				dag->toString(WMPExpDagAd::NO_NODES).c_str(), //TBD NO_NODES??
				str_addr, res_num, //TBD or dag->size()??
				NULL, &subjobs)) {
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerPartitionable(WMPExpDagAd *dag, int res_num)",
			WMS_OPERATION_NOT_ALLOWED,
			error_message("edg_wll_RegisterJobSync"));
	}
	logUserTags(dag->getSubAttributes(JDL::USERTAGS));
	
	registerSubJobs(dag, subjobs);
	vector<string> jobids;
	for (unsigned int i = 0; i < res_num; i++) {
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	dag->setJobIds(jobids);
	
	return jobids;
}

vector<string>
WMPEventLogger::registerDag(WMPExpDagAd *dag)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);

	cerr<<"Registering dag" << endl;
	edg_wlc_JobId *subjobs = NULL;
	dag->setAttribute(WMPExpDagAd::SEQUENCE_CODE, getSequence());
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
			dag->toString(WMPExpDagAd::NO_NODES).c_str(), str_addr, dag->size(),
			NULL, &subjobs)) {
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerDag(WMPExpDagAd *dag)",
			WMS_OPERATION_NOT_ALLOWED,
			error_message("edg_wll_RegisterJobSync"));
	}
	logUserTags(dag->getSubAttributes(JDL::USERTAGS));
	
	registerSubJobs(dag, subjobs);
	vector<string> jobids;
	for (unsigned int i = 0; i < dag->size(); i++) {
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	dag->setJobIds(jobids);
	
	return jobids;
}


// User Tags and JobId logs:
void
WMPEventLogger::logUserTag(string name, const string &value){
	cerr<<"name: "<<name<<endl;
	Ad *classad = new Ad();
	classad->setAttribute(JDL::JDL_ORIGINAL, value);
	logUserTags(classad->ad());
	delete classad;
}

void
WMPEventLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> >
	userTags)
{
	for (unsigned int i = 0; i < userTags.size(); i++) {
		if (userTags[i].second->GetKind() != classad::ExprTree::CLASSAD_NODE) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(std::vector<std::pair<std::string, "
				"classad::ExprTree*> > userTags)",
				WMS_OPERATION_NOT_ALLOWED, "Wrong UserTag value for "
				+ userTags[i].first);
		}
		setLoggingJob(userTags[i].first);
		logUserTags((classad::ClassAd*)(userTags[i].second));
	}
	edg_wll_SetLoggingJob(ctx, id->getId(), NULL, EDG_WLL_SEQ_NORMAL);
}

void
WMPEventLogger::logUserTags(classad::ClassAd* userTags)
{
	vector<pair<string, classad::ExprTree *> > vect;
	classad::Value val;
	string attrValue;
	userTags->GetComponents(vect);
	for (unsigned int i = 0; i< vect.size(); i++) {
		if (!userTags->EvaluateExpr(vect[i].second, val)) {
			// Unable to parse the attribute
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
				WMS_OPERATION_NOT_ALLOWED, "Unable to Parse Expression");
		}
 		if (val.IsStringValue(attrValue)) {
			if (edg_wll_LogUserTag(ctx, (vect[i].first).c_str(),
					attrValue.c_str())) {
				throw JobOperationException(__FILE__, __LINE__,
					"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
					WMS_OPERATION_NOT_ALLOWED,
					error_message("edg_wll_LogUserTag"));
			}
 		}
	}
}
void WMPEventLogger::setLoggingJob( const std::string &jid , const char* seq_code){
	glite::wmsutils::jobid::JobId jobid ( jid ) ;
	edg_wll_SetLoggingJob(ctx, jobid.getId(), seq_code ,EDG_WLL_SEQ_NORMAL);
}


/******************************************************************
Logging Events: Accepted,Aborted, Refused, Cancel, Purge
******************************************************************/
// Actual LB log call
bool WMPEventLogger::logEvent(event_name event, const char* reason){
		switch (event){
			case LOG_ACCEPT:
				edglog(debug) << "Logging Accept event" << endl ;
				return !edg_wll_LogAccepted(ctx,  EDG_WLL_SOURCE_WM_PROXY , (retrieveHostName().c_str()),"","");
				break;
			case LOG_CANCEL:
				edglog(debug) << "Logging Cancel event" << endl ;
				return !edg_wll_LogCancelREQ(ctx, reason);
				break;
			case LOG_CLEAR:
				edglog(debug) << "Logging Clear event" << endl ;
				return !edg_wll_LogClearUSER(ctx);
				break;
			case LOG_ABORT:
				edglog(debug) << "Logging Abort event" << endl ;
				return !edg_wll_LogAbort(ctx,reason);
				break;
			default:
				edglog(severe) << "Warning: no event caught, not Logging" << endl ;
				return true;
		}
}

void WMPEventLogger::logEvent(event_name event, const char* reason, bool retry){
	int i=0;
	edglog_fn("CFSI::logCancel(event, char* reason, bool retry)");
	edglog(fatal) << "Logging "<< event<<" Request." << std::endl;
	bool logged = false;
	for (; i < 3 && !logged && retry; i++) {
		// PERFORM the Requested operation (actual LB call)
		logged = logEvent (event , reason);
		if (!logged && (i<2) && retry) {
			edglog(info) << "Failed to log Cancelled Job. Sleeping 60 seconds before retry." << std::endl;
			sleep(60);
		}
	}
	if ((retry && (i>=3)) || (!retry && (i>0)) ) {
		edglog(severe) << "Error while logging Cancelled Job." << std::endl;
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::logEvent(event ,char* reason, bool retry)",
				WMS_OPERATION_NOT_ALLOWED,
				error_message("edg_wll_Log<Event>REQ"));
	}
	edglog(debug) << "Logged." << std::endl;
}

void WMPEventLogger::logEvent(event_name  event, const char* reason,bool retry, bool test) {
	if (!test) logEvent(event, reason, retry);
	edglog_fn("WMPEventLogger::logEvent(event_name event, bool retry, bool test) ");
	edglog(fatal) << "Logging "<< event<<" Job." << std::endl;
	int res;
	bool with_hp = false;
	int lap = 0;
	std::string host_cert , host_key ;

/*  TBD TBD GET PARAM
	std::string user_proxy;
	std::string host_proxy;
	cmd->getParam("X509UserProxy", user_proxy);
	reset_user_proxy(cmd, user_proxy);
	cmd->getParam("HostProxy", host_proxy);
*/
	do {
		// PERFORM the Requested operation (actual LB call)
		res = logEvent (event , reason);
		testAndLog( res, with_hp, lap, host_cert , host_key );
	} while( res != 0 );
	// return true    TBD.... this means failure??
}


void WMPEventLogger::logEnqueuedJob(std::string jdl, const std::string &file_queue, bool mode, const char *reason, bool retry ){
	int i=0;
	edglog_fn("WMPEventLogger::logEnqueuedJobN");
	edglog(fatal) << "Logging Enqueued Job." << std::endl;
	bool logged = false;
	for (; i < 3 && !logged && retry; i++) {
		logged = !edg_wll_LogEnQueued (ctx,
			file_queue.c_str(),
			jdl.c_str(),
			(mode ? "OK" : "FAIL"),
			reason);
		if (!logged && (i<2) && retry) {
			edglog(info) << "Failed to log Enqueued Job. Sleeping 60 seconds before retry." << std::endl;
			sleep(60);
		}
	}
	if ((retry && (i>=3)) || (!retry && (i>0)) ) {
		edglog(severe) << "Error while logging Enqueued Job." << std::endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::logEnqueuedJob(std::string jdl, const std::string &file_queue, bool mode,"
			"const char *reason, bool retry )",
			WMS_OPERATION_NOT_ALLOWED, "Error while logging Enqueued Job.");
	}
	edglog(debug) << "Logged." << std::endl;
	edg_wll_FreeContext(ctx);
}


void WMPEventLogger::logEnqueuedJob(std::string jdl, const std::string &proxy_path,
			const std::string &host_cert, const std::string &host_key, const std::string &file_queue,
			bool mode, const char *reason, bool retry, bool test){
	if (!test) logEnqueuedJob(jdl, file_queue, mode, reason, retry);
	edglog_fn("WMPEventLogger::logEnqueuedJobE");
	edglog(fatal) << "Logging Enqueued Job." << std::endl;
	int res;
	bool with_hp = false;
	int lap = 0;
	reset_user_proxy( proxy_path);
	do {
		res = edg_wll_LogEnQueued (ctx,
				file_queue.c_str(),
				jdl.c_str(),
				(mode ? "OK" : "FAIL"),
				reason);
		testAndLog( res, with_hp, lap, host_cert, host_key );
	}while( res != 0 );
}


int WMPEventLogger::setX509Param(const std::string &name , const std::string &value ,  edg_wll_ContextParam lbX509code ){
	if(value.length() == 0 ){
		edglog(warning) <<name<< " not set inside configuration file." << std::endl
			<< "Trying with a default NULL and hoping for the best." << std::endl;
		return edg_wll_SetParam( ctx, lbX509code, NULL );
	}else {
		edglog(info) << name <<" found = \"" << value << "\"." << std::endl;
		return  edg_wll_SetParam( ctx, lbX509code, value.c_str() );
	}
}

void
WMPEventLogger::testAndLog( int &code, bool &with_hp, int &lap, const std::string &host_cert, const std::string &host_key)
{
	edglog_fn("NS2WM::test&Log");
	if( code ) {
		switch( code ) {
			case EINVAL:
				edglog(critical) << "Critical error in L&B calls: EINVAL." << std::endl;
				code = 0; // Don't retry...
				break;
			case EDG_WLL_ERROR_GSS:
				edglog(severe) << "Severe error in SSL layer while communicating with L&B daemons." << std::endl;
				if( with_hp ) {
					edglog(severe) << "The log with the host certificate has just been done. Giving up." << std::endl;
					code = 0; // Don't retry...
				}else {
					edglog(info) << "Retrying using cert and key certificate..." << std::endl;
					// unsetting proxy parameters:
					edg_wll_SetParam( ctx, EDG_WLL_PARAM_X509_PROXY , NULL );
					// Setting usercert and userkey instead:
					if   ( setX509Param("Host Cert" , host_cert , EDG_WLL_PARAM_X509_CERT)
						| setX509Param("Host Key" , host_key , EDG_WLL_PARAM_X509_KEY) )  {
					edglog(severe) << "Cannot set some host credential inside the context. Giving up." << std::endl;
					code = 0; // Don't retry.
					} else with_hp = true; // Set and retry (code is still != 0)
				}
				break;
			default:
				if( ++lap > 3 ) {
					edglog(error) << "L&B call retried " << lap << " times always failed." << std::endl
						<< "Ignoring." << std::endl;
					code = 0; // Don't retry anymore
				} else {
					edglog(warning) << "L&B call got a transient error. Waiting 60 seconds and trying again." << std::endl;
					edglog(info) << "Try n. " << lap << "/3" << std::endl;
					sleep( 60 );
				}
				break;
			}
	}   // end of "if( code ) {"
	else // The logging call worked fine, do nothing
		edglog(debug) << "L&B call succeeded." << std::endl;
	return;
}

void
WMPEventLogger::reset_user_proxy( const std::string &proxy_path ){
	edglog_fn("WMPEventLogger::resetUserProxy");
	bool    erase = false;
	int     res;
	if( proxy_path.size() ) {
		boost::filesystem::path pf( boost::filesystem::normalize_path(proxy_path), boost::filesystem::system_specific );
		if( boost::filesystem::exists(pf) ) {
			res = edg_wll_SetParam( ctx, EDG_WLL_PARAM_X509_PROXY, proxy_path.c_str() );
			if( res ) edglog(severe) << "Cannot set proxyfile path inside context." << std::endl;
		}
		else erase = true;
	}
	else if( proxy_path.size() == 0 ) erase = true;
	if( erase ) {
		res = edg_wll_SetParam( ctx, EDG_WLL_PARAM_X509_PROXY, NULL );
		if( res ) edglog(severe) << "Cannot reset proxyfile path inside context." << std::endl;;
	}
}


// Error Message Parsing
const char*
WMPEventLogger::error_message(const char *api){
	char *error_message = (char*) malloc(1024);
	char *msg;
	char *dsc;
	edg_wll_Error(ctx, &msg, &dsc);
	sprintf(error_message, "%s %s %s%s%s%s%s", api,
		getenv(GLITE_WMS_LOG_DESTINATION), "\n", msg, " (", dsc, ")");
	return error_message;
}

} // eventlogger
} // wmproxy
} // wms
} // glite

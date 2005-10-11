/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpeventlogger.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpeventlogger.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

// Logging & Bookkeeping
#include "glite/lb/producer.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/lb/context-int.h" // edg_wll_SetSequenceCode

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "utilities/wmpexception_codes.h"
#include "utilities/wmpexceptions.h"
#include "utilities/wmputils.h"

#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "authorizer/wmpauthorizer.h"

const bool DEFAULT_USER_PROXY = false;

const char * GLITE_HOST_KEY = "GLITE_HOST_KEY";
const char * GLITE_HOST_CERT = "GLITE_HOST_CERT";

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
namespace logger = glite::wms::common::logger;
namespace jobid  = glite::wmsutils::jobid;
namespace authorizer = glite::wms::wmproxy::authorizer;
namespace wmputilities = glite::wms::wmproxy::utilities;

using namespace std;
using namespace glite::wms::wmproxy::utilities;  //Exception codes
using namespace glite::wms::jdl; // DagAd
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

const char *WMPEventLogger::GLITE_WMS_LOG_DESTINATION 
	= "GLITE_WMS_LOG_DESTINATION";


WMPEventLogger::WMPEventLogger(const string &endpoint)
{
	edglog_fn("WMPEventLogger::WMPEventLogger");
	
	id = NULL;
	subjobs = NULL;
	this->delegatedproxy = "";
	lbProxy_b = DEFAULT_USER_PROXY;
	this->server = endpoint;
	if (edg_wll_InitContext(&ctx)
		|| (edg_wll_SetParam(ctx, EDG_WLL_PARAM_SOURCE, EDG_WLL_SOURCE_WM_PROXY))
		|| (endpoint.c_str()
			? edg_wll_SetParamString(ctx, EDG_WLL_PARAM_INSTANCE, endpoint.c_str())
			: false)
		) {
			edglog(critical)<<"LB initialisation failed"<<endl;
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
WMPEventLogger::init(const string &lb_host, int lb_port,
	jobid::JobId *id, const string &desturiprotocol, int desturiport)
{
	GLITE_STACK_TRY("init()");
	edglog_fn("WMPEventlogger::init");
	
	this->id = id;
	setLoggingJob(id->toString());
	this->lb_host = lb_host;
	this->lb_port = lb_port;
	this->desturiprotocol = desturiprotocol;
	this->desturiport = desturiport;
	if (!getenv(GLITE_WMS_LOG_DESTINATION)) {
		if (edg_wll_SetParamString(ctx, EDG_WLL_PARAM_DESTINATION,
				lb_host.c_str())) {
			edglog(critical)<<"LB initialisation failed (set destination)"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::init(const string& lb_host, int lb_port, "
					"jobid::JobId *id)",
				WMS_OPERATION_NOT_ALLOWED, "LB initialisation failed "
					"(set destination)");
		}
	}
	
	GLITE_STACK_CATCH();
}

char *
WMPEventLogger::getSequence()
{
	GLITE_STACK_TRY("getSequence()");
	return edg_wll_GetSequenceCode(ctx);
	GLITE_STACK_CATCH();
};

void
WMPEventLogger::setSequenceCode(const string &seqcode)
{
	GLITE_STACK_TRY("setSequenceCode()");
	edg_wll_SetSequenceCode(ctx, seqcode.c_str(), EDG_WLL_SEQ_NORMAL);
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::incrementSequenceCode()
{
	GLITE_STACK_TRY("incrementSequenceCode()");
	edg_wll_IncSequenceCode(ctx);
	GLITE_STACK_CATCH();
}

char *
WMPEventLogger::registerProxyRenewal(const string &proxy_path,
	const string &my_proxy_server)
{
	GLITE_STACK_TRY("registerProxyRenewal()");
	edglog_fn("WMPEventLogger::registerProxyRenewal");
	
	edglog(debug)<<"Proxy path: "<<proxy_path<<endl;
	edglog(debug)<<"My Proxy Server: "<<my_proxy_server<<endl;
	
	char *renewal_proxy_path = NULL;
	int i = LOG_RETRY_COUNT;
	for (; i > 0
		&& edg_wlpr_RegisterProxyExt((char*)proxy_path.c_str(),
	 			(char*)my_proxy_server.c_str(), LB_RENEWAL_PORT,
				id->getId(), EDG_WLPR_FLAG_UNIQUE,
      		    &renewal_proxy_path); i--);
	
 	if (i > 0) {
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
 	return renewal_proxy_path;
 	
 	GLITE_STACK_CATCH();
}

void
WMPEventLogger::unregisterProxyRenewal()
{
	GLITE_STACK_TRY("unregisterProxyRenewal()");
	
	char *renewal_proxy_path = NULL;
	for (int i = 0; i < LOG_RETRY_COUNT
		&& edg_wlpr_UnregisterProxy(id->getId(), renewal_proxy_path); i++);
		
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::registerJob(JobAd *jad)
{
	GLITE_STACK_TRY("registerJob()");
	edglog_fn("WMPEventlogger::registerJob");
	
	char str_addr[1024];
	sprintf(str_addr, "%s", server.c_str());
	edglog(debug)<<"str_addr: "<<str_addr<<endl;
	char * seqcode = getSequence();
	jad->setAttribute(JDL::LB_SEQUENCE_CODE, string(seqcode));
	int register_result;
#ifdef HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Registering normal job to LB Proxy"<<endl;
		register_result = edg_wll_RegisterJobProxy(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
			jad->toSubmissionString().c_str(),
			str_addr, 0, NULL, NULL) ;
		edglog(debug)<<"edg_wll_RegisterJobProxy() exit code: "<<register_result<<endl;
	}else {
#endif  //HAVE_LBPROXY
		edglog(debug)<<"Registering normal job to LB"<<endl;
		register_result = edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
			jad->toSubmissionString().c_str(),
			str_addr, 0, NULL, NULL) ;
		edglog(debug)<<"edg_wll_RegisterJobSync() exit code: "<<register_result<<endl;
#ifdef HAVE_LBPROXY
	}
#endif  //HAVE_LBPROXY
	if (register_result) {
		string msg = error_message("edg_wll_RegisterJob");
		edglog(severe)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerJob(JobAd* jad)",
			WMS_OPERATION_NOT_ALLOWED, msg);
	}
	//logUserTag(JDL::LB_SEQUENCE_CODE, string(seqcode));
	if (jad->hasAttribute(JDL::USERTAGS)) {
		logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
	}
	
	GLITE_STACK_CATCH();
} 

void
WMPEventLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)
{
	GLITE_STACK_TRY("registerSubJobs()");
	edglog_fn("WMPEventlogger::registerSubJobs()");
	
	char str_nsAddr[1024];
	sprintf(str_nsAddr, "%s", server.c_str());
	
	char **jdls_char;
	char **zero_char;
	vector<string>::iterator iter;
	vector<string> jdls = ad->getSubmissionStrings();
	jdls_char = (char**) malloc(sizeof(char*) * (jdls.size() + 1));
	zero_char = jdls_char;
	jdls_char[jdls.size()] = NULL;
	
	int i = 0;
	for (iter = jdls.begin(); iter != jdls.end(); iter++, i++) {
		*zero_char = (char*) malloc(iter->size() + 1);
		sprintf(*zero_char, "%s", iter->c_str());
		zero_char++;
	}
        edglog(debug)<<"Registering DAG subjobs to LB"<<endl;
	if (edg_wll_RegisterSubjobs(ctx, id->getId(), jdls_char, str_nsAddr,
			subjobs)) {
		string msg = error_message("edg_wll_RegisterSubjobs");
		edglog(critical)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerSubJobs()",
			WMS_OPERATION_NOT_ALLOWED, msg);
	}
	for (unsigned int i = 0; i < jdls.size(); i++ ) {
		std::free(jdls_char[i]);
	}
    std::free(jdls_char);
    
    GLITE_STACK_CATCH();
}


vector<string>
WMPEventLogger::generateSubjobsIds(int res_num)
{
	GLITE_STACK_TRY("generateSubjobsIds()");
	edglog_fn("WMPEventlogger::generateSubjobsIds");
	
	subjobs = NULL;
	edg_wll_GenerateSubjobIds(ctx, id->getId(), res_num, NULL, &subjobs);
	vector<string> jobids;
	for (int i = 0; i < res_num; i++) {
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
		edglog(debug)<<"Subjob generated: "
			<<string(edg_wlc_JobIdUnparse(subjobs[i]))<<endl;
	}
	return jobids;
	
	GLITE_STACK_CATCH();
}


vector<string>
WMPEventLogger::registerDag(WMPExpDagAd *dag)
{
	GLITE_STACK_TRY("registerDag()");
	edglog_fn("WMPEventlogger::registerDag");
	
	char str_addr[1024];
	sprintf(str_addr, "%s", server.c_str());
	
	int size;
	if (subjobs) {
		size = 0;
	} else {
		size = dag->size();
	}
	dag->setAttribute(WMPExpDagAd::SEQUENCE_CODE, string(getSequence())); //TBC needed???
	
    vector<string> jobids;

    int register_result;
#ifdef HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Registering DAG to LB Proxy"<<endl;
		register_result = edg_wll_RegisterJobProxy(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
			dag->toString().c_str(), str_addr, size,
			NULL, &subjobs) ;
	} else {
#endif  //HAVE_LBPROXY
		edglog(debug)<<"Registering DAG to LB"<<endl;
		register_result = edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
			dag->toString().c_str(), str_addr, size,
			NULL, &subjobs);
#ifdef HAVE_LBPROXY
	}
#endif  //HAVE_LBPROXY
	if (register_result) {
		string msg = error_message("edg_wll_RegisterJob");
		edglog(critical)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerDag(WMPExpDagAd *dag)",
			WMS_OPERATION_NOT_ALLOWED, msg);
	}
	
	// Logging parent user tags
	if (dag->hasAttribute(JDL::USERTAGS)) {
		classad::ClassAd * parentclassad = dag->getAttributeAd(JDL::USERTAGS).ad();
		logUserTags(parentclassad);
	}
	
	// Logging children user tags
	logUserTags(dag->getSubAttributes(JDL::USERTAGS));
	
	registerSubJobs(dag, subjobs);
	
	return jobids;
	
	GLITE_STACK_CATCH();
}

// User Tags and JobId logs:
void
WMPEventLogger::logUserTag(string name, const string &value)
{
	GLITE_STACK_TRY("logUserTag()");
        edglog_fn("WMPEventlogger::logUserTag()");
	
	Ad *classad = new Ad();
	classad->setAttribute(name, value);
        //edglog(debug)<<"Logging user tags to LB"<<endl;
	logUserTags(classad->ad());
	delete classad;
	
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> >
	userTags)
{
	GLITE_STACK_TRY("logUserTags()");
	edglog_fn("WMPEventlogger::logUserTags()");
	
	for (unsigned int i = 0; i < userTags.size(); i++) {
		if (userTags[i].second->GetKind() != classad::ExprTree::CLASSAD_NODE) {
			string msg = "Wrong UserTag value for " + userTags[i].first;
			edglog(error)<<msg<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(std::vector<std::pair<std::string, "
				"classad::ExprTree*> > userTags)",
				WMS_OPERATION_NOT_ALLOWED, msg);
		}
		setLoggingJob(userTags[i].first);
		logUserTags((classad::ClassAd*)(userTags[i].second));
	}
#ifdef HAVE_LBPROXY
	if (lbProxy_b) { 
		edg_wll_SetLoggingJobProxy(ctx, id->getId(), NULL, getUserDN(),
			EDG_WLL_SEQ_NORMAL); 
	} else {
#endif  //HAVE_LBPROXY
		edg_wll_SetLoggingJob(ctx, id->getId(), NULL, EDG_WLL_SEQ_NORMAL);
#ifdef HAVE_LBPROXY
	}
#endif  //HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logUserTags(classad::ClassAd* userTags)
{
	GLITE_STACK_TRY("logUserTags()");
	edglog_fn("WMPEventlogger::logUserTags()");
	
	vector<pair<string, classad::ExprTree *> > vect;
	classad::Value val;
	string attrValue;
	userTags->GetComponents(vect);
	for (unsigned int i = 0; i< vect.size(); i++) {
		if (!userTags->EvaluateExpr(vect[i].second, val)) {
			edglog(error)<<"Unable to Parse Expression"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
				WMS_OPERATION_NOT_ALLOWED, "Unable to Parse Expression");
		}
 		if (val.IsStringValue(attrValue)) {
            edglog(debug)<<"Logging user tag to LB: "<<vect[i].first<<endl;
			if (edg_wll_LogUserTag(ctx, (vect[i].first).c_str(),
					attrValue.c_str())) {
				string msg = error_message("edg_wll_LogUserTag");
				edglog(critical)<<msg<<endl;
				throw JobOperationException(__FILE__, __LINE__,
					"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
					WMS_OPERATION_NOT_ALLOWED, msg);
			}
 		}
	}
	
	GLITE_STACK_CATCH();
}

void 
WMPEventLogger::setLoggingJob(const std::string &jid, const char* seq_code)
{
	GLITE_STACK_TRY("setLoggingJob()");
        edglog_fn("WMPEventlogger::setLoggingJob()");
	
	glite::wmsutils::jobid::JobId jobid(jid);
#ifdef HAVE_LBPROXY
	if (lbProxy_b) {
                edglog(debug)<<"Setting job for logging to LBProxy"<<endl;
		edg_wll_SetLoggingJobProxy(ctx, jobid.getId(), seq_code, 
			getUserDN(), EDG_WLL_SEQ_NORMAL);
	} else {
#endif  //HAVE_LBPROXY
                edglog(debug)<<"Setting job for logging to LB"<<endl;
		edg_wll_SetLoggingJob(ctx, jobid.getId(), seq_code, EDG_WLL_SEQ_NORMAL);
#ifdef HAVE_LBPROXY
	}
#endif  //HAVE_LBPROXY

	GLITE_STACK_CATCH();
}


/******************************************************************
	Logging Events: Accepted,Aborted, Refused, Cancel, Purge
******************************************************************/
// Actual LB log call

bool
WMPEventLogger::logListener(const char* host, int port)
{
	GLITE_STACK_TRY("logListener()");
	edglog_fn("WMPEventlogger::logListener");
	
	edglog(debug) << "Logging Listener event" << endl;
#ifdef HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LBProxy" << endl;
		return edg_wll_LogListenerProxy(ctx, "InteractiveListener", 
			host, (uint16_t) port);
	} else {
#endif  //HAVE_LBPROXY
		edglog(debug) << "Logging to LB" << endl;
		return edg_wll_LogListener(ctx, "InteractiveListener", 
			host, (uint16_t) port);
#ifdef HAVE_LBPROXY
	} // end switch LB normal
#endif  //HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::logCheckpointable(const char* current_step, const char* state)
{
	GLITE_STACK_TRY("logCheckpointable()");
	edglog_fn("WMPEventlogger::logCheckpointable");
	
	edglog(debug) << "Logging Checkpointable event" << endl;
#ifdef HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LBProxy" << endl;
		return edg_wll_LogEventProxy(ctx, EDG_WLL_EVENT_CHKPT,
				  EDG_WLL_FORMAT_CHKPT, current_step, state);
	} else {
#endif  //HAVE_LBPROXY
		edglog(debug) << "Logging to LB" << endl;
		return edg_wll_LogEventSync(ctx, EDG_WLL_EVENT_CHKPT,
				  EDG_WLL_FORMAT_CHKPT, current_step, state);
#ifdef HAVE_LBPROXY
	} // end switch LB normal
#endif  //HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::logAbortEventSync(char* reason)
{
        edglog_fn("WMPEventlogger::logAbortEventSync");
	edglog(debug)<<"Logging Abort event (sync)"<<endl;
	//TBC Checks possibility to do it with LBProxy
#ifdef HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Logging to LBProxy"<<endl;
		return edg_wll_LogEventProxy(ctx, EDG_WLL_EVENT_ABORT, EDG_WLL_FORMAT_ABORT,
			reason);
	} else { // end switch LB PROXY
#endif  //HAVE_LBPROXY
		edglog(debug) << "Logging to LB" << endl;
		return edg_wll_LogEventSync(ctx, EDG_WLL_EVENT_ABORT, EDG_WLL_FORMAT_ABORT,
			reason);
#ifdef HAVE_LBPROXY
	} // end switch LB normal
#endif  //HAVE_LBPROXY

}

bool 
WMPEventLogger::logEvent(event_name event, const char* reason, 
	const char* file_queue, const char* jdl)
{
	GLITE_STACK_TRY("logEvent()");
	edglog_fn("WMPEventlogger::logEvent");
	
#ifdef HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LBProxy" << endl ;
		switch (event){
			case LOG_ACCEPT:
				edglog(debug) << "Logging Accept event" << endl ;
				return edg_wll_LogAcceptedProxy(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(),"","");
				break;
			case LOG_CANCEL:
				edglog(debug) << "Logging Cancel event" << endl ;
				return edg_wll_LogCancelREQProxy(ctx, reason);
				break;
			case LOG_CLEAR:
				edglog(debug) << "Logging Clear event" << endl ;
				//return edg_wll_LogClearUSERProxy(ctx);
				return edg_wll_LogClearProxy(ctx, reason);
				break;
			case LOG_ABORT:
				edglog(debug) << "Logging Abort event" << endl ;
				return edg_wll_LogAbortProxy(ctx,reason);
				break;
			case LOG_ENQUEUE_OK:
				edglog(debug) << "Logging Enqueue OK event" << endl ;
				return edg_wll_LogEnQueuedProxy(ctx, file_queue, jdl, "OK", "");
				break;
			case LOG_ENQUEUE_FAIL:
				edglog(debug) << "Logging Enqueue FAIL event" << endl ;
				return edg_wll_LogEnQueuedProxy (ctx, file_queue, jdl, "FAIL", reason);
				break;
			default:
				edglog(severe) << "Warning: no event caught, not Logging" << endl ;
				return true;
		}
	} else { // end switch LB PROXY
#endif  //HAVE_LBPROXY
		edglog(debug) << "Logging to LB" << endl ;
		switch (event){
			case LOG_ACCEPT:
				edglog(debug) << "Logging Accept event" << endl ;
				return edg_wll_LogAccepted(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(), "", "");
				break;
			case LOG_CANCEL:
				edglog(debug) << "Logging Cancel event" << endl ;
				return edg_wll_LogCancelREQ(ctx, reason);
				break;
			case LOG_CLEAR:
				edglog(debug) << "Logging Clear event" << endl ;
				//return edg_wll_LogClearUSER(ctx);
				return edg_wll_LogClear(ctx, reason);
				break;
			case LOG_ABORT:
				edglog(debug) << "Logging Abort event" << endl ;
				return edg_wll_LogAbort(ctx, reason);
				break;
			case LOG_ENQUEUE_OK:
				edglog(debug) << "Logging Enqueue OK event" << endl ;
				return edg_wll_LogEnQueued (ctx, file_queue, jdl, "OK", "");
				break;
			case LOG_ENQUEUE_FAIL:
				edglog(debug) << "Logging Enqueue FAIL event" << endl ;
				return edg_wll_LogEnQueued (ctx, file_queue, jdl, "FAIL", reason);
				break;
			default:
				edglog(severe) << "Warning: no event caught, not Logging" << endl ;
				return true;
		}
#ifdef HAVE_LBPROXY
	} // end switch LB normal
#endif  //HAVE_LBPROXY
	// Unpredictable line: ERROR!
	edglog(fatal) << "logEvent Unpredictable line found!!!" << endl ;
	return true;
	
	GLITE_STACK_CATCH();
}

void 
WMPEventLogger::logEvent(event_name event, const char* reason, bool retry, 
	const char* file_queue, const char* jdl)
{
	GLITE_STACK_TRY("logEvent()");
	edglog_fn("WMPEventLogger::logEvent");
	edglog(debug) << "Logging "<< event<<" request" << std::endl;
	
	int i=0;
	bool logged = false;
	for (; i < 3 && !logged && retry; i++) {
		// PERFORM the Requested operation (actual LB call)
		logged = logEvent (event , reason);
		if (!logged && (i<2) && retry) {
			edglog(debug) << "Failed to log. Sleeping 15 seconds before retry..."
				<< std::endl;
			sleep(15);
		}
	}
	if ((retry && (i>=3)) || (!retry && (i>0)) ) {
		string msg = error_message("edg_wll_Log<Event>REQ");
		edglog(severe)<<msg<<std::endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::logEvent(event ,char* reason, bool retry)",
			WMS_OPERATION_NOT_ALLOWED, msg);
	}
	edglog(debug) << "Logged" << std::endl;
	
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logEvent(event_name event, const char* reason, bool retry,
	bool test, const char* file_queue, const char* jdl)
{
	GLITE_STACK_TRY("logEvent()");
	edglog_fn("WMPEventlogger::logEvent()");
	
	if (!test) {
		logEvent(event, reason, retry);
	}
	
	edglog(fatal) << "Logging "<< event<<" request" << std::endl;
	int res;
	bool with_hp = false;
	int lap = 0;
	do {
		// PERFORM the Requested operation (actual LB call)
		res = logEvent(event, reason);
		testAndLog(res, with_hp, lap);
	} while( res != 0 );
	// return true    TBD.... this means failure??
	
	GLITE_STACK_CATCH();
}

int
WMPEventLogger::setUserProxy(const std::string &proxy)
{
	GLITE_STACK_TRY("setUserProxy()");
	edglog_fn("WMPEventlogger::setUserProxy");
	
	this->delegatedproxy = proxy;
	// Checking proxy validity
	if (proxy != "") {
		try {
			authorizer::WMPAuthorizer::checkProxy(proxy);
			return edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY, proxy.c_str());
		} catch (Exception &ex) {
			if (ex.getCode() != wmputilities::WMS_PROXY_EXPIRED) {
				throw ex;
			}
		}
	}
	// Setting host proxy
	if (!getenv(GLITE_HOST_KEY) || !getenv(GLITE_HOST_CERT)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setJobLoggingProxy()", WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	} else {
		return (edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY, NULL)
			|| edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_KEY, getenv(GLITE_HOST_KEY))
			|| edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_CERT, getenv(GLITE_HOST_CERT)));
	}
	return 0;
	
	GLITE_STACK_CATCH();
}


void
WMPEventLogger::testAndLog(int &code, bool &with_hp, int &lap)
{
	GLITE_STACK_TRY("testAndLog()");
	edglog_fn("WMPEventlogger::testAndLog");
	
	if (code) {
		switch (code) {
			case EINVAL:
				edglog(critical)<<"Critical error in LB calls: EINVAL"
					<<std::endl;
				code = 0; // Don't retry...
				break;
			case EDG_WLL_ERROR_GSS:
				edglog(severe)<<"Severe error in SSL layer while communicating "
					"with LB daemons"<<std::endl;
				if (with_hp) {
					edglog(severe)<<"The log with the host certificate has "
						"just been done. Giving up"<<std::endl;
					code = 0; // Don't retry...
				} else {
					code = 0; // Don't retry.
				}
				break;
			default:
				if(++lap > 3) {
					edglog(error)<<"LB call retried "<<lap
						<<" times always failed. "<<std::endl
						<< "Ignoring"<<std::endl;
					code = 0; // Don't retry anymore
				} else {
					edglog(debug)<<"LB call got a transient error. "
						"Waiting 15 seconds before trying again..." << std::endl;
					edglog(debug)<<"Try n. "<<lap<<"/3"<<std::endl;
					sleep(15);
				}
				break;
			}
	} else { // The logging call worked fine, do nothing
		edglog(debug)<<"LB call succeeded"<<std::endl;
	}
	return;
	
	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::retrieveEvent(const std::string &jobid_str, event_name eventname)
{
	GLITE_STACK_TRY("retrieveEvent()");
	edglog_fn("WMPEventlogger::retrieveEvent");
	
	edg_wlc_JobId jobid;
  	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	
  	regJobEvent event;
  	event.instance = "";
  	event.jdl = "";
  	
  	// parse the jobID string
  	if (edg_wlc_JobIdParse(jobid_str.c_str(), &jobid)) {
  		edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw JobOperationException(__FILE__, __LINE__,
			"std::string WMPEventLogger::retrieveEvent()",
			WMS_OPERATION_NOT_ALLOWED, "Error during edg_wlc_JobIdParse");
  	}

  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  
  	ec[0].attr = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	switch (eventname) {
  		case LOG_ENQUEUE:
  			ec[0].value.i = EDG_WLL_EVENT_ENQUEUED;
  			break;
  		case LOG_ABORT:
  			ec[0].value.i = EDG_WLL_EVENT_ABORT;
  			break;
  		default:
  			break;
  	}

  	int error = edg_wll_QueryEvents(ctx, jc, ec, &events);
  	if (error == ENOENT) { // no events found
   		return false;
  	}
  	if (error) {
  		//TBC throw an exception???
   		//log_error("Query failed");
    	return false;
	}
  	return true;
  	
  	GLITE_STACK_CATCH();
}

regJobEvent
WMPEventLogger::retrieveRegJobEvent(const std::string &jobid_str)
{
	GLITE_STACK_TRY("retrieveRegJobEvent()");
	edglog_fn("WMPEventlogger::retrieveRegJobEvent");
	
	edg_wlc_JobId jobid;
  	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	
  	regJobEvent event;
  	event.instance = "";
  	event.jdl = "";
  	event.parent = "";
  	
  	// parse the jobID string
  	if (edg_wlc_JobIdParse(jobid_str.c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw JobOperationException(__FILE__, __LINE__,
			"std::string WMPEventLogger::retrieveEvent()",
			WMS_OPERATION_NOT_ALLOWED, "Error during edg_wlc_JobIdParse");
  	}

  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  
  	// event condition: Event type = REGJOB
  	ec[0].attr = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.i = EDG_WLL_EVENT_REGJOB;
  	
  	int error = edg_wll_QueryEvents(ctx, jc, ec, &events);
  	if (error == ENOENT) { // no events found
   		return event;
  	}
  	if (error) {
   		///TBC throw an exception???
   		//log_error("Query failed");
    	return event;
	}
  	
  	// TBC regJobs.ns, is it correct??? I'm getting "instance"
  	// Getting first event found
  	if (events) {
  		if (events[0].regJob.src_instance) {
	  		event.instance = events[0].regJob.src_instance;
  		}
	  	if (events[0].regJob.jdl) {
	  		event.jdl = events[0].regJob.jdl;
	  	}
	  	if (events[0].regJob.parent) {
	  		event.parent = string(edg_wlc_JobIdUnparse(events[0].regJob.parent));
	  	}
  	}
  	return event;
  	
  	GLITE_STACK_CATCH();
}

void
setJobLoggingProxy(glite::lb::Job &lbjob, const string &proxy)
{
	GLITE_STACK_TRY("setJobLoggingProxy()");
	edglog_fn("WMPEventLogger::setJobLoggingProxy");
	
	if (proxy != "") {
		try {
			authorizer::WMPAuthorizer::checkProxy(proxy);
			//lbjob.setParam(EDG_WLL_PARAM_X509_KEY, 0);
			//lbjob.setParam(EDG_WLL_PARAM_X509_CERT, 0);
			lbjob.setParam(EDG_WLL_PARAM_X509_PROXY, proxy);
			return;
		} catch (Exception &ex) {
			if (ex.getCode() != wmputilities::WMS_PROXY_EXPIRED) {
				throw ex;
			}
		}
	}
	// Setting host proxy
	if (!getenv(GLITE_HOST_KEY) || !getenv(GLITE_HOST_CERT)) {
		edglog(severe)<<"Unable to set User Proxy for LB context"<<endl;
		throw AuthenticationException(__FILE__, __LINE__,
			"setJobLoggingProxy()", WMS_AUTHENTICATION_ERROR,
			"Unable to set User Proxy for LB context");
	} else {
		//lbjob.setParam(EDG_WLL_PARAM_X509_PROXY, 0);
		lbjob.setParam(EDG_WLL_PARAM_X509_KEY, string(getenv(GLITE_HOST_KEY)));
		lbjob.setParam(EDG_WLL_PARAM_X509_CERT, string(getenv(GLITE_HOST_CERT)));
	}
	
	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::isRegisterEventOnly()
{
	GLITE_STACK_TRY("isRegisterEventOnly()");
	edglog_fn("WMPEventLogger::isRegisterEventOnly");
	
	glite::lb::Job lbjob(*id);
	setJobLoggingProxy(lbjob, this->delegatedproxy);
	
	std::vector<glite::lb::Event> events = lbjob.log();
	for (unsigned int i = 0; i < events.size(); i++) {
		if ((events[i].type != glite::lb::Event::REGJOB)	
				&& (events[i].type != glite::lb::Event::USERTAG)) {
			return false;	
		}
	}
	return true;
	
	GLITE_STACK_CATCH();
}

string
WMPEventLogger::getUserTagSequenceCode() 
{
	GLITE_STACK_TRY("getUserTagSequenceCode()");
	edglog_fn("WMPEventLogger::getUserTagSequenceCode");
	
	glite::lb::Job lbjob(*id);
	setJobLoggingProxy(lbjob, this->delegatedproxy);
	
	std::vector<glite::lb::Event> events = lbjob.log();
	glite::lb::Event event;
	for (unsigned int i = 0; i < events.size(); i++) {
		event = events[i];
		if (event.type == glite::lb::Event::USERTAG) {
			if (event.getValString(glite::lb::Event::NAME) == "lb_sequence_code") {
				return event.getValString(glite::lb::Event::VALUE);	
			}
		}
	}
	return "";
	
	GLITE_STACK_CATCH();
}

string
WMPEventLogger::getUserTagJDLOriginal() 
{
	GLITE_STACK_TRY("getUserJDLOriginal()");
	edglog_fn("WMPEventLogger::getUserJDLOriginal");
	
	glite::lb::Job lbjob(*id);
	setJobLoggingProxy(lbjob, this->delegatedproxy);
	
	std::vector<glite::lb::Event> events = lbjob.log();
	glite::lb::Event event;
	for (unsigned int i = 0; i < events.size(); i++) {
		event = events[i];
		if (event.type == glite::lb::Event::USERTAG) {
			if (event.getValString(glite::lb::Event::NAME) == "jdl_original") {
				return event.getValString(glite::lb::Event::VALUE);	
			}
		}
	}
	return "";
	
	GLITE_STACK_CATCH();
}

glite::lb::JobStatus
WMPEventLogger::getStatus(JobId *jid, const string &delegatedproxy,
	bool childreninfo)
{
	GLITE_STACK_TRY("WMPEventLogger::getStatus()");
	
	glite::lb::Job lb_job(*jid);
	setJobLoggingProxy(lb_job, delegatedproxy);
	int flag = glite::lb::Job::STAT_CLASSADS;
	if (childreninfo) {
		flag = flag | glite::lb::Job::STAT_CHILDREN;
	}
	return lb_job.status(flag); // to get also jdl
	// lb_job.status(0) minimal information about the job
	
	GLITE_STACK_CATCH();
}

// Error Message Parsing
const char*
WMPEventLogger::error_message(const char *api)
{
	GLITE_STACK_TRY("error_message()");
	
	char *error_message = (char*) malloc(1024);
	char *msg;
	char *dsc;
	edg_wll_Error(ctx, &msg, &dsc);
	sprintf(error_message, "%s %s %s%s%s%s%s", api,
		getenv(GLITE_WMS_LOG_DESTINATION), "\n", msg, " (", dsc, ")");
	return error_message;
	
	GLITE_STACK_CATCH();
}



} // eventlogger
} // wmproxy
} // wms
} // glite

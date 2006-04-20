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

// Boost
#include <boost/lexical_cast.hpp>

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

#include "glite/jdl/PrivateAttributes.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/jdl_attributes.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "authorizer/wmpauthorizer.h"

const bool DEFAULT_USER_PROXY = false;

const char * GLITE_HOST_KEY = "GLITE_HOST_KEY";
const char * GLITE_HOST_CERT = "GLITE_HOST_CERT";

namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
namespace logger       = glite::wms::common::logger;
namespace jobid        = glite::wmsutils::jobid;
namespace authorizer   = glite::wms::wmproxy::authorizer;
namespace wmputilities = glite::wms::wmproxy::utilities;

using namespace std;
using namespace glite::wms::wmproxy::utilities;  //Exception codes
using namespace glite::jdl; // DagAd
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

const char *WMPEventLogger::GLITE_WMS_LOG_DESTINATION 
	= "GLITE_WMS_LOG_DESTINATION";

const string WMPEventLogger::QUERY_SEQUENCE_CODE = "lb_sequence_code";
const string WMPEventLogger::QUERY_JDL_ORIGINAL = "jdl_original";

// Randomic value lower and upper limits for LB[Proxy] log retry on failure
const int LB_LOG_RETRY_LOWER_LIMIT = 30;
const int LB_LOG_RETRY_UPPER_LIMIT = 60;
const int LB_PROXY_LOG_RETRY_LOWER_LIMIT = 5;
const int LB_PROXY_LOG_RETRY_UPPER_LIMIT = 15;

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
			throw LBException(__FILE__, __LINE__,
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
			throw LBException(__FILE__, __LINE__,
				"WMPEventLogger::init(const string& lb_host, int lb_port, "
					"jobid::JobId *id)",
				WMS_IS_FAILURE, "LB initialisation failed "
					"(set destination)");
		}
	}
	
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::randomsleep()
{
	if (this->lbProxy_b) {
		int randomvalue = generateRandomNumber(LB_PROXY_LOG_RETRY_LOWER_LIMIT,
			LB_PROXY_LOG_RETRY_UPPER_LIMIT);
		edglog(debug)<<"Failed to log. Sleeping for "<<randomvalue
			<<" seconds before retry..."<<endl;
		sleep(randomvalue);
	} else {
		int randomvalue = generateRandomNumber(LB_LOG_RETRY_LOWER_LIMIT,
			LB_LOG_RETRY_UPPER_LIMIT);
		edglog(debug)<<"Failed to log. Sleeping for "<<randomvalue
			<<" seconds before retry..."<<endl;
		sleep(randomvalue);
	}
}

void
WMPEventLogger::setLBProxy(bool value, char * userdn)
{	GLITE_STACK_TRY("setLBProxy()");
	this->lbProxy_b = value;
	if (value) {
		edg_wll_SetParam(ctx, EDG_WLL_PARAM_LBPROXY_USER, userdn);
	} else {
		edg_wll_SetParam(ctx, EDG_WLL_PARAM_LBPROXY_USER, NULL);
	}
	GLITE_STACK_CATCH();
}

bool 
WMPEventLogger::getLBProxy()
{
	return this->lbProxy_b;
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
	for (; (i > 0) 
				&& glite_renewal_RegisterProxy((char*)proxy_path.c_str(),
		 		(char*)my_proxy_server.c_str(), LB_RENEWAL_PORT,
				id->toString().c_str(), EDG_WLPR_FLAG_UNIQUE,
				&renewal_proxy_path);
			i--);
	
 	if (i > 0) {
		for (int j = 0;
			j < LOG_RETRY_COUNT
		    	&& !edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY,
		    	renewal_proxy_path);
	    	j++);
	} else {
		for (int j = 0;
			j < LOG_RETRY_COUNT
		    	&& !edg_wll_SetParam(ctx, EDG_WLL_PARAM_X509_PROXY,
		    	proxy_path.c_str());
	    	j++);
	   	throw LBException(__FILE__, __LINE__,
			"WMPEventLogger::registerProxyRenewal(const string &proxy_path, "
			"const string &my_proxy_server)", WMS_LOGGING_ERROR,
			error_message("registerProxyRenewal"));
 	}
 	return renewal_proxy_path;
 	
 	GLITE_STACK_CATCH();
}

void
WMPEventLogger::unregisterProxyRenewal()
{
	GLITE_STACK_TRY("unregisterProxyRenewal()");
	
	char *renewal_proxy_path = NULL;
	for (int i = 0;
		i < LOG_RETRY_COUNT
			&& glite_renewal_UnregisterProxy(id->toString().c_str(),
			renewal_proxy_path);
		i++);
		
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::registerJob(JobAd *jad, const string &path)
{
	GLITE_STACK_TRY("registerJob()");
	edglog_fn("WMPEventlogger::registerJob");
	
	char str_addr[1024];
	sprintf(str_addr, "%s", server.c_str());
	edglog(debug)<<"str_addr: "<<str_addr<<endl;
	char * seqcode = getSequence();
	jad->setAttribute(JDL::LB_SEQUENCE_CODE, string(seqcode));
	
	int register_result = 1;
	int i = LOG_RETRY_COUNT;
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Registering normal job to LB Proxy..."<<endl;
		/*for (; (i > 0) && register_result; i--) {
			register_result = edg_wll_RegisterJobProxy(ctx, id->getId(),
				EDG_WLL_JOB_SIMPLE, path.c_str(), str_addr, 0, NULL, NULL);
			if (register_result) {
				randomsleep();				
			}
		}*/
		register_result = edg_wll_RegisterJobProxy(ctx, id->getId(),
			EDG_WLL_JOB_SIMPLE, path.c_str(), str_addr, 0, NULL, NULL);
		edglog(debug)<<"edg_wll_RegisterJobProxy() exit code: "
			<<register_result<<endl;
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<<"Registering normal job to LB..."<<endl;
		/*for (; (i > 0) && register_result; i--) {
			register_result = edg_wll_RegisterJobSync(ctx, id->getId(),
				EDG_WLL_JOB_SIMPLE, path.c_str(), str_addr, 0, NULL, NULL);
			if (register_result) {
				randomsleep();				
			}
		}*/
		register_result = edg_wll_RegisterJobSync(ctx, id->getId(),
			EDG_WLL_JOB_SIMPLE, path.c_str(), str_addr, 0, NULL, NULL);
		edglog(debug)<<"edg_wll_RegisterJobSync() exit code: "
			<<register_result<<endl;
#ifdef GLITE_WMS_HAVE_LBPROXY
	}
#endif  //GLITE_WMS_HAVE_LBPROXY

	if (register_result) {
		string msg = error_message("edg_wll_RegisterJob");
		edglog(severe)<<msg<<endl;
		throw LBException(__FILE__, __LINE__,
			"WMPEventLogger::registerJob(JobAd* jad)",
			WMS_LOGGING_ERROR, msg);
	}
	if (jad->hasAttribute(JDL::USERTAGS)) {
		logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
	}
	
	GLITE_STACK_CATCH();
} 

void
WMPEventLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)
{
	GLITE_STACK_TRY("registerSubJobs()");
	edglog_fn("WMPEventlogger::registerSubJobs");

	char str_nsAddr[1024];
	sprintf(str_nsAddr, "%s", server.c_str());

	// Prepare both jdls and subjobs for registering:
	vector<string> jobids;
	vector<string> jdls = ad->getSubmissionStrings(&jobids);
	unsigned int jdlssize = jdls.size();
	if (jdlssize!= jobids.size()){
		// This is a fatal Exception and sholud never be raised
		string msg = "Number of nodes do not correspond to number of inserted jobids";
		edglog(critical)<<msg<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPEventLogger::registerSubJobs()",
			WMS_OPERATION_NOT_ALLOWED, msg);
	}

	// Define useful structures:
	char **jdls_char;
	char **zero_char;
	jdls_char = (char**) malloc(sizeof(char*) * (jdlssize + 1)); // same size for both arrays
	zero_char = jdls_char;
	jdls_char[jdlssize] = NULL;

	edg_wlc_JobId jids_id [jdlssize];
	// Create needed structures
	vector<string>::iterator iter = jdls.begin();
	vector<string>::iterator const end = jdls.end();
	vector<string>::iterator iterId    = jobids.begin();
	for (unsigned int jid_i = 0; iter != end; ++iter, ++iterId, jid_i++) {
		*zero_char = (char*) malloc(iter->size() + 1);
		sprintf(*zero_char, "%s", iter->c_str());
		zero_char++;
		jids_id[jid_i]=glite::wmsutils::jobid::JobId (*iterId).getId();
	}
	int register_result;
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Registering DAG subjobs to LB Proxy..."<<endl;
		register_result = edg_wll_RegisterSubjobsProxy(ctx, id->getId(), jdls_char,
			str_nsAddr, jids_id);
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<<"Registering DAG subjobs to LB..."<<endl;
		register_result = edg_wll_RegisterSubjobs(ctx, id->getId(), jdls_char,
			str_nsAddr, jids_id);
#ifdef GLITE_WMS_HAVE_LBPROXY
	}
#endif  //GLITE_WMS_HAVE_LBPROXY

	if (register_result) {
		string msg = error_message("edg_wll_RegisterSubjobs");
		edglog(critical)<<msg<<endl;
		throw LBException(__FILE__, __LINE__,
			"WMPEventLogger::registerSubJobs()",
			WMS_LOGGING_ERROR, msg);
	}
	
	for (unsigned int i = 0; i < jdlssize; i++) {
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
	edg_wll_GenerateSubjobIds(ctx, id->getId(), res_num, "WMPROXY", &subjobs);
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
WMPEventLogger::registerDag(WMPExpDagAd *dag, const string &path)
{
	GLITE_STACK_TRY("registerDag()");
	edglog_fn("WMPEventlogger::registerDag");
	
	char str_addr[1024];
	sprintf(str_addr, "%s", server.c_str());
	
	dag->setAttribute(WMPExpDagAd::SEQUENCE_CODE, string(getSequence())); //TBC needed???
    vector<string> jobids;
    
    int dagsize = dag->size();   
    
	// Setting LB log sync timeout   
    int timeout = 120 + dagsize;   
    if (edg_wll_SetParamInt(ctx, EDG_WLL_PARAM_LOG_SYNC_TIMEOUT, timeout)) {   
    	edglog(error)<<"Unable to set LB log sync timeout"<<endl;   
   	}

    int register_result;
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Registering DAG to LB Proxy..."<<endl;
		register_result = edg_wll_RegisterJobProxy(ctx, id->getId(),
			EDG_WLL_REGJOB_DAG, path.c_str(), str_addr, dagsize,
			"WMPROXY", &subjobs) ;
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<<"Registering DAG to LB..."<<endl;
		register_result = edg_wll_RegisterJobSync(ctx, id->getId(),
			EDG_WLL_REGJOB_DAG, path.c_str(), str_addr, dagsize,
			"WMPROXY", &subjobs);
#ifdef GLITE_WMS_HAVE_LBPROXY
	}
#endif  //GLITE_WMS_HAVE_LBPROXY

	if (register_result) {
		string msg = error_message("edg_wll_RegisterJob");
		edglog(critical)<<msg<<endl;
		throw LBException(__FILE__, __LINE__,
			"WMPEventLogger::registerDag(WMPExpDagAd *dag)",
			WMS_LOGGING_ERROR, msg);
	}
	
	// Logging parent user tags
	if (dag->hasAttribute(JDL::USERTAGS)) {
		classad::ClassAd * parentclassad = dag->getAttributeAd(JDL::USERTAGS).ad();
		logUserTags(parentclassad);
	}
	
	// Logging children user tags
	logUserTags(dag->getSubAttributes(JDL::USERTAGS));
	
	//registerSubJobs(dag, subjobs);
	
	return jobids;
	
	GLITE_STACK_CATCH();
}

// User Tags and JobId logs:
void
WMPEventLogger::logUserTag(string name, const string &value)
{
	GLITE_STACK_TRY("logUserTag()");
    edglog_fn("WMPEventlogger::logUserTag");
	
	Ad *classad = new Ad();
	classad->setAttribute(name, value);
	logUserTags(classad->ad());
	delete classad;
	
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> >
	userTags)
{
	GLITE_STACK_TRY("logUserTags()");
	edglog_fn("WMPEventlogger::logUserTags");
	
	unsigned int size = userTags.size();
	for (unsigned int i = 0; i < size; i++) {
		if (userTags[i].second->GetKind() != classad::ExprTree::CLASSAD_NODE) {
			string msg = "Wrong UserTag value for " + userTags[i].first;
			edglog(error)<<msg<<endl;
			throw LBException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(std::vector<std::pair<std::string, "
				"classad::ExprTree*> > userTags)",
				WMS_LOGGING_ERROR, msg);
		}
		setLoggingJob(userTags[i].first);
		logUserTags((classad::ClassAd*)(userTags[i].second));
	}
	setLoggingJob(id->toString());

	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logUserTags(classad::ClassAd* userTags)
{
	GLITE_STACK_TRY("logUserTags()");
	edglog_fn("WMPEventlogger::logUserTags");
	
	vector<pair<string, classad::ExprTree *> > vect;
	classad::Value val;
	string attrValue;
	userTags->GetComponents(vect);
	
	// Pointer to a function
	int (*fp) (_edg_wll_Context*, const char*, const char*);
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Setting job to log to LB Proxy..."<<endl;
		fp = &edg_wll_LogUserTagProxy;
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
        edglog(debug)<<"Setting job to log to LB..."<<endl;
		fp = &edg_wll_LogUserTag;
#ifdef GLITE_WMS_HAVE_LBPROXY
	}
#endif  //GLITE_WMS_HAVE_LBPROXY

	unsigned int size = vect.size();
	for (unsigned int i = 0; i < size; i++) {
		if (!userTags->EvaluateExpr(vect[i].second, val)) {
			edglog(error)<<"Unable to Parse Expression"<<endl;
			throw LBException(__FILE__, __LINE__,
				"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
				WMS_LOGGING_ERROR, "Unable to Parse Expression");
		}
 		if (val.IsStringValue(attrValue)) {
            edglog(debug)<<"Logging user tag to LB: "<<vect[i].first<<endl;
            int j = LOG_RETRY_COUNT;
            int outcome = 1;
			for (; (j > 0) && outcome; j--) {
				outcome = fp(ctx, (vect[i].first).c_str(), attrValue.c_str());
				if (outcome) {
					randomsleep();
				}
			}
			if (outcome) {
				string msg = error_message("edg_wll_LogUserTag");
				edglog(critical)<<msg<<endl;
				throw LBException(__FILE__, __LINE__,
					"WMPEventLogger::logUserTags(classad::ClassAd* userTags)",
					WMS_LOGGING_ERROR, msg);
			}
 		}
	}
	
	GLITE_STACK_CATCH();
}

void 
WMPEventLogger::setLoggingJob(const std::string &jid, const char* seq_code)
{
	GLITE_STACK_TRY("setLoggingJob()");
	edglog_fn("WMPEventlogger::setLoggingJob");
	
	glite::wmsutils::jobid::JobId jobid(jid);
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
        edglog(debug)<<"Setting job for logging to LB Proxy..."<<endl;
		edg_wll_SetLoggingJobProxy(ctx, jobid.getId(), seq_code,
			getUserDN(), EDG_WLL_SEQ_NORMAL);
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
        edglog(debug)<<"Setting job for logging to LB..."<<endl;
		edg_wll_SetLoggingJob(ctx, jobid.getId(), seq_code, EDG_WLL_SEQ_NORMAL);
#ifdef GLITE_WMS_HAVE_LBPROXY
	}
#endif  //GLITE_WMS_HAVE_LBPROXY

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
	
	edglog(debug) << "Logging Listener event..." << endl;
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LB Proxy..." << endl;
		return edg_wll_LogListenerProxy(ctx, "InteractiveListener", 
			host, (uint16_t) port);
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug) << "Logging to LB..." << endl;
		return edg_wll_LogListener(ctx, "InteractiveListener", 
			host, (uint16_t) port);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::logCheckpointable(const char* current_step, const char* state)
{
	GLITE_STACK_TRY("logCheckpointable()");
	edglog_fn("WMPEventlogger::logCheckpointable");
	
	edglog(debug) << "Logging Checkpointable event..." << endl;
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LB Proxy..." << endl;
		return edg_wll_LogEventProxy(ctx, EDG_WLL_EVENT_CHKPT,
			EDG_WLL_FORMAT_CHKPT, current_step, state);
	} else {
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug) << "Logging to LB..." << endl;
		return edg_wll_LogEventSync(ctx, EDG_WLL_EVENT_CHKPT,
			EDG_WLL_FORMAT_CHKPT, current_step, state);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::logAbortEventSync(char* reason)
{
	GLITE_STACK_TRY("logAbortEventSync()");
	
    edglog_fn("WMPEventlogger::logAbortEventSync");
	edglog(debug)<<"Logging Abort event (sync)"<<endl;
	//TBC Checks possibility to do it with LBProxy
	
#ifdef HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Logging to LB Proxy..."<<endl;
		return edg_wll_LogEventProxy(ctx, EDG_WLL_EVENT_ABORT,
			EDG_WLL_FORMAT_ABORT, reason);
	} else { // end switch LB PROXY
#endif  //HAVE_LBPROXY
		edglog(debug) << "Logging to LB..." << endl;
		return edg_wll_LogEventSync(ctx, EDG_WLL_EVENT_ABORT,
			EDG_WLL_FORMAT_ABORT, reason);
#ifdef HAVE_LBPROXY
	} // end switch LB normal
#endif  //HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool
WMPEventLogger::logAcceptEventSync()
{
	GLITE_STACK_TRY("logAcceptEventSync()");
	
    edglog_fn("WMPEventlogger::logAcceptEventSync");
	edglog(debug)<<"Logging Accept event (sync)"<<endl;
	
  	char * s_from = edg_wll_SourceToString(EDG_WLL_SOURCE_NETWORK_SERVER);
 
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Logging to LB Proxy..."<<endl;
		return edg_wll_LogEventProxy(ctx, EDG_WLL_EVENT_ACCEPTED,
			EDG_WLL_FORMAT_ACCEPTED, s_from, "", "", "");
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug) << "Logging to LB..." << endl;
		return edg_wll_LogEventSync(ctx, EDG_WLL_EVENT_ACCEPTED,
			EDG_WLL_FORMAT_ACCEPTED, s_from, "", "", "");
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

	GLITE_STACK_CATCH();
}

bool 
WMPEventLogger::logEvent(event_name event, const char* reason, 
	const char* file_queue, const char* jdl)
{
	GLITE_STACK_TRY("logEvent()");
	edglog_fn("WMPEventlogger::logEvent");
	
#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b){
		edglog(debug) << "Logging to LB Proxy..." << endl ;
		switch (event){
			case LOG_ACCEPT:
				edglog(debug) << "Logging Accept event..." << endl ;
				return edg_wll_LogAcceptedProxy(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(),"","");
				break;
			case LOG_ENQUEUE_START:
				edglog(debug) << "Logging Enqueue START event..." << endl ;
				return edg_wll_LogEnQueuedProxy(ctx, file_queue, jdl, "START", "");
				break;
			case LOG_ENQUEUE_OK:
				edglog(debug) << "Logging Enqueue OK event..." << endl ;
				return edg_wll_LogEnQueuedProxy(ctx, file_queue, jdl, "OK", "");
				break;
			case LOG_ENQUEUE_FAIL:
				edglog(debug) << "Logging Enqueue FAIL event..." << endl ;
				return edg_wll_LogEnQueuedProxy (ctx, file_queue, jdl, "FAIL",
					reason);
				break;
			case LOG_ABORT:
				edglog(debug) << "Logging Abort event..." << endl ;
				return edg_wll_LogAbortProxy(ctx,reason);
				break;
			case LOG_REFUSE:
				edglog(debug) << "Logging Refuse event..." << endl ;
				return edg_wll_LogRefusedProxy(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(),"","");
				break;
			case LOG_CANCEL:
				edglog(debug) << "Logging Cancel event..." << endl ;
				return edg_wll_LogCancelREQProxy(ctx, reason);
				break;
			case LOG_CLEAR:
				edglog(debug) << "Logging Clear event..." << endl ;
				//return edg_wll_LogClearUSERProxy(ctx);
				return edg_wll_LogClearProxy(ctx, reason);
				break;
			case LOG_USER_TAG:
				edglog(debug) << "Logging User Tag event..." << endl ;
				return edg_wll_LogCancelREQProxy(ctx, reason);
				break;
			
			default:
				edglog(severe) << "Warning: no event caught, not Logging" << endl ;
				return true;
		}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug) << "Logging to LB..." << endl ;
		switch (event){
			case LOG_ACCEPT:
				edglog(debug) << "Logging Accept event..." << endl ;
				return edg_wll_LogAccepted(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(), "", "");
				break;
			case LOG_REFUSE:
				edglog(debug) << "Logging Refuse event..." << endl ;
				return edg_wll_LogRefused(ctx, EDG_WLL_SOURCE_WM_PROXY,
					server.c_str(),"","");
				break;
			case LOG_CANCEL:
				edglog(debug) << "Logging Cancel event..." << endl ;
				return edg_wll_LogCancelREQ(ctx, reason);
				break;
			case LOG_CLEAR:
				edglog(debug) << "Logging Clear event..." << endl ;
				//return edg_wll_LogClearUSER(ctx);
				return edg_wll_LogClear(ctx, reason);
				break;
			case LOG_ABORT:
				edglog(debug) << "Logging Abort event..." << endl ;
				return edg_wll_LogAbort(ctx, reason);
				break;
			case LOG_ENQUEUE_START:
				edglog(debug) << "Logging Enqueue START event..." << endl ;
				return edg_wll_LogEnQueued(ctx, file_queue, jdl, "START", "");
				break;
			case LOG_ENQUEUE_OK:
				edglog(debug) << "Logging Enqueue OK event..." << endl ;
				return edg_wll_LogEnQueued (ctx, file_queue, jdl, "OK", "");
				break;
			case LOG_ENQUEUE_FAIL:
				edglog(debug) << "Logging Enqueue FAIL event..." << endl ;
				return edg_wll_LogEnQueued (ctx, file_queue, jdl, "FAIL",
					reason);
				break;
			default:
				edglog(severe) << "Warning: no event caught, not Logging" << endl ;
				return true;
		}
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY
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
	edglog(debug)<<"Logging event "<<event<<" request..."<<endl;
	
	int i = 0;
	bool logged = false;
	for (; i < 3 && !logged && retry; i++) {
		// PERFORM the Requested operation (actual LB call)
		logged = logEvent(event, reason, file_queue, jdl);
		if (!logged && (i < 2) && retry) {
			randomsleep();
		}
	}
	if ((retry && (i >= 3)) || (!retry && (i > 0)) ) {
		string msg = error_message("edg_wll_Log<Event>REQ");
		edglog(severe)<<msg<<endl;
		throw LBException(__FILE__, __LINE__,
			"WMPEventLogger::logEvent()",
			WMS_LOGGING_ERROR, msg);
	}
	edglog(debug)<<"Logged"<<endl;
	
	GLITE_STACK_CATCH();
}

void
WMPEventLogger::logEvent(event_name event, const char* reason, bool retry,
	bool test, const char* file_queue, const char* jdl)
{
	GLITE_STACK_TRY("logEvent()");
	edglog_fn("WMPEventlogger::logEvent");
	
	if (!test) {
		logEvent(event, reason, retry, file_queue, jdl);
	}
	
	edglog(fatal)<<"Logging "<<event<<" request"<<endl;
	int res;
	bool with_hp = false;
	int lap = 0;
	do {
		// PERFORM the Requested operation (actual LB call)
		res = logEvent(event, reason, file_queue, jdl);
		testAndLog(res, with_hp, lap);
	} while (res != 0);
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
				edglog(critical)<<"Critical error in LB calls: EINVAL"<<endl;
				code = 0; // Don't retry...
				break;
			case EDG_WLL_ERROR_GSS:
				edglog(severe)<<"Severe error in SSL layer while communicating "
					"with LB daemons"<<endl;
				if (with_hp) {
					edglog(severe)<<"The log with the host certificate has "
						"just been done. Giving up"<<endl;
					code = 0; // Don't retry...
				} else {
					code = 0; // Don't retry.
				}
				break;
			default:
				if (++lap > 3) {
					string msg = "Unable to complete operation: LB call retried " 
						+ boost::lexical_cast<std::string>(lap - 1)
						+ " times, always failed";
					edglog(error)<<msg<<endl;
					//code = 0; // Don't retry anymore
					throw LBException(__FILE__, __LINE__,
						"WMPEventLogger::testAndLog()", WMS_LOGGING_ERROR,
						msg + "\n(please contact server administrator)");
				} else {
					randomsleep();
				}
				break;
			}
	} else { // The logging call worked fine, do nothing
		edglog(debug)<<"LB call succeeded"<<endl;
	}
	return;
	
	GLITE_STACK_CATCH();
}

regJobEvent
WMPEventLogger::retrieveRegJobEvent(const std::string &jobid_str)
{
	GLITE_STACK_TRY("retrieveRegJobEvent()");
	edglog_fn("WMPEventlogger::retrieveRegJobEvent");
	
	edg_wlc_JobId jobid;
  	// parse the jobID string
  	if (edg_wlc_JobIdParse(jobid_str.c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw LBException(__FILE__, __LINE__,
			"retrieveRegJobEvent()", WMS_OPERATION_NOT_ALLOWED,
			"Error during edg_wlc_JobIdParse");
  	}
	
	regJobEvent event;
  	event.instance = "";
  	event.jdl = "";
  	event.parent = "";
  	
	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  	jc[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  
  	// event condition: Event type = REGJOB
  	ec[0].attr = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.i = EDG_WLL_EVENT_REGJOB;
  	ec[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	int error;

#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Quering LB Proxy..."<<endl;
		error = edg_wll_QueryEventsProxy(ctx, jc, ec, &events);
		if (error == ENOENT) { // no events found
	   		edglog(debug)<< "No events found quering LB Proxy. Quering LB..."<<endl;
			error = edg_wll_QueryEvents(ctx, jc, ec, &events);
	  	}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<< "Quering LB..."<<endl;
		error = edg_wll_QueryEvents(ctx, jc, ec, &events);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

  	if (error == ENOENT) { // no events found
   		return event;
  	}
  	if (error) {
   		///TBC throw an exception???
   		//log_error("Query failed");
    	return event;
	}
	
  	// Getting last event found
    int i = 0;
    while (events[i].type) {
    	i++;
    }
    i--;
  	
  	if (events[i].regJob.src_instance) {
  		event.instance = events[i].regJob.src_instance;
	}
  	if (events[i].regJob.jdl) {
  		event.jdl = events[i].regJob.jdl;
  	}
  	if (events[i].regJob.parent) {
  		event.parent = string(edg_wlc_JobIdUnparse(events[i].regJob.parent));
  	}
  	
  	for (int i = 0; events[i].type; i++) {
		edg_wll_FreeEvent(&events[i]);
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

string
WMPEventLogger::getLastEventSeqCode()
{
	GLITE_STACK_TRY("getLastEventSeqCode()");
	edglog_fn("WMPEventlogger::getLastEventSeqCode");
	
	edg_wlc_JobId jobid;
  	// parse the jobID string
  	if (edg_wlc_JobIdParse((this->id->toString()).c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw LBException(__FILE__, __LINE__,
			"getLastEventSeqCode()", WMS_OPERATION_NOT_ALLOWED,
			"Error during edg_wlc_JobIdParse");
  	}
	
	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  	jc[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	// event condition: Event SOURCE = NetworkServer
  	ec[0].attr = EDG_WLL_QUERY_ATTR_SOURCE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.i = EDG_WLL_SOURCE_NETWORK_SERVER;
  	ec[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	int error;

#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Quering LB Proxy..."<<endl;
		error = edg_wll_QueryEventsProxy(ctx, jc, ec, &events);
		if (error == ENOENT) { // no events found
	   		edglog(debug)<< "No events found quering LB Proxy. Quering LB..."<<endl;
			error = edg_wll_QueryEvents(ctx, jc, ec, &events);
	  	}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<< "Quering LB..."<<endl;
		error = edg_wll_QueryEvents(ctx, jc, ec, &events);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

  	if (error) {
  		if (error == ENOENT) { // no events found
	   		edglog(critical)<<"No events found for job: "<<this->id->toString()
	   			<<endl;
	    	throw LBException(__FILE__, __LINE__, "getLastEventSeqCode()",
	    		WMS_LOGGING_ERROR, "Unable to complete operation: no events "
	    		"found for requested job");
	  	}
   		edglog(critical)<<"Unable to get events for job: "<<this->id->toString()
   			<<endl;
    	throw LBException(__FILE__, __LINE__, "getLastEventSeqCode()",
    		WMS_LOGGING_ERROR, "Unable to complete operation: unable to get "
    		"events for requested job");
	}
	
	int i = 0;
	while (events[i].type) {
		i++;
	}
	i--;
	
	string seqcode = events[i].any.seqcode;
    
    for (int i = 0; events[i].type; i++) {
		edg_wll_FreeEvent(&events[i]);
	}
	
  	return seqcode;
  
  	GLITE_STACK_CATCH();
}

string
WMPEventLogger::isStartAllowed()
{
	GLITE_STACK_TRY("isStartAllowed()");
	edglog_fn("WMPEventlogger::isStartAllowed");
	
	edg_wlc_JobId jobid;
  	// parse the jobID string
  	if (edg_wlc_JobIdParse((this->id->toString()).c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw LBException(__FILE__, __LINE__,
			"isStartAllowed()", WMS_OPERATION_NOT_ALLOWED,
			"Error during edg_wlc_JobIdParse");
  	}
	
	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  	jc[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	// event condition: Event SOURCE = NetworkServer
  	ec[0].attr = EDG_WLL_QUERY_ATTR_SOURCE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.i = EDG_WLL_SOURCE_NETWORK_SERVER;
  	ec[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	/*ec[0].attr = EDG_WLL_QUERY_ATTR_INSTANCE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.c = strdup((char*)this->server.c_str());
  	ec[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;*/
  	
  	int error;

#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Quering LB Proxy..."<<endl;
		error = edg_wll_QueryEventsProxy(ctx, jc, ec, &events);
		if (error == ENOENT) { // no events found
	   		edglog(debug)<< "No events found quering LB Proxy. Quering LB..."<<endl;
			error = edg_wll_QueryEvents(ctx, jc, ec, &events);
	  	}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<< "Quering LB..."<<endl;
		error = edg_wll_QueryEvents(ctx, jc, ec, &events);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

  	if (error) {
  		if (error == ENOENT) { // no events found
	   		edglog(critical)<<"No events found for job: "<<this->id->toString()
	   			<<endl;
	    	throw LBException(__FILE__, __LINE__, "isStartAllowed()",
	    		WMS_LOGGING_ERROR, "Unable to complete operation: no events "
	    		"found for requested job");
	  	}
   		edglog(critical)<<"Unable to get events for job: "<<this->id->toString()
   			<<endl;
    	throw LBException(__FILE__, __LINE__, "isStartAllowed()",
    		WMS_LOGGING_ERROR, "Unable to complete operation: unable to get "
    		"events for requested job");
	}
	
	bool flag = false;
	int i = 0;
	while (events[i].type) {
		edglog(debug)<<"Event type: "<<events[i].type<<endl;
		switch (events[i].type) {
			case EDG_WLL_EVENT_REGJOB: case EDG_WLL_EVENT_USERTAG:
				break;
			case EDG_WLL_EVENT_ACCEPTED: case EDG_WLL_EVENT_ENQUEUED:
				flag = true;
				break;
			case EDG_WLL_EVENT_ABORT:
				throw JobOperationException(__FILE__, __LINE__,
					"isStartAllowed()", WMS_OPERATION_NOT_ALLOWED,
					"job state (ABORTED) doesn't allow start operation");
				break;
			case EDG_WLL_EVENT_CANCEL:
				throw JobOperationException(__FILE__, __LINE__,
					"isStartAllowed()", WMS_OPERATION_NOT_ALLOWED,
					"job state (CANCELLED) doesn't allow start operation");
				break;
			default:
				throw JobOperationException(__FILE__, __LINE__,
					"isStartAllowed()", WMS_OPERATION_NOT_ALLOWED,
					"unable to complete the operation");
				break;
		}
		i++;
	}
	i--;
	
	// Only logs EDG_WLL_EVENT_REGJOB, EDG_WLL_EVENT_USERTAG,
	// EDG_WLL_EVENT_ACCEPTED & EDG_WLL_EVENT_ENQUEUED are present
	// Checking for last event type
	string seqcode = "";
    if (flag) {
    	if ((events[i].type == EDG_WLL_EVENT_ENQUEUED)
    			&& (events[i].enQueued.result == EDG_WLL_ENQUEUED_FAIL)) {
			seqcode = string(events[i].enQueued.seqcode);
    	} else {
    		throw JobOperationException(__FILE__, __LINE__,
				"isStartAllowed()", WMS_OPERATION_NOT_ALLOWED,
				"job started or start operation already in progress");
    	}
	} else {
		if (events[i].type == EDG_WLL_EVENT_REGJOB) {
			seqcode = string(events[i].regJob.seqcode);
		} else if (events[i].type == EDG_WLL_EVENT_USERTAG) {
			seqcode = string(events[i].userTag.seqcode);
		}
    }
    
    for (int i = 0; events[i].type; i++) {
		edg_wll_FreeEvent(&events[i]);
	}
	
  	return seqcode;
  
  	GLITE_STACK_CATCH();
}


string
WMPEventLogger::getUserTag(const string &tagname)
{
	GLITE_STACK_TRY("getUserTag()");
	edglog_fn("WMPEventlogger::getUserTag");
	
	edg_wlc_JobId jobid;
	string jobid_str = id->toString();
  	// parse the jobID string
  	if (edg_wlc_JobIdParse(jobid_str.c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw LBException(__FILE__, __LINE__,
			"getUserTag()", WMS_LOGGING_ERROR,
			"Error during edg_wlc_JobIdParse");
  	}

  	edg_wll_Event * events = NULL;
  	edg_wll_QueryRec jc[2];
  	edg_wll_QueryRec ec[2];
  	memset(jc, 0, sizeof jc);
  	memset(ec, 0, sizeof ec);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  	jc[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  
  	// event condition: Event type = REGJOB
  	ec[0].attr = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  	ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	ec[0].value.i = EDG_WLL_EVENT_USERTAG;
  	ec[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  	
  	int error;
  	string returnvalue = "";

#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Quering LB Proxy..."<<endl;
		error = edg_wll_QueryEventsProxy(ctx, jc, ec, &events);
		if (error == ENOENT) { // no events found
	   		edglog(debug)<< "No events found quering LB Proxy. Quering LB..."<<endl;
			error = edg_wll_QueryEvents(ctx, jc, ec, &events);
	  	}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<< "Quering LB..."<<endl;
		error = edg_wll_QueryEvents(ctx, jc, ec, &events);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

  	if (error == ENOENT) { // no events found
   		return returnvalue;
  	}
  	if (error) {
   		///TBC throw an exception???
   		//log_error("Query failed");
    	return returnvalue;
	}

	// Events array is set to a NULL event wich type = 0
	for (int i = 0; events[i].type; i++) {
		if (string(events[i].userTag.name) == tagname) {
			returnvalue = events[i].userTag.value;
			edglog(debug)<<"Found " + string(tagname) + " user tag, value: "
				<<returnvalue<<endl;
			break;
		}
	}
	
	for (int i = 0; events[i].type; i++) {
		edg_wll_FreeEvent(&events[i]);
	}
  	
  	return returnvalue;
  	
  	GLITE_STACK_CATCH();
}

glite::lb::JobStatus
WMPEventLogger::getStatus(bool childreninfo)
{
	GLITE_STACK_TRY("getStatus()");
	edglog_fn("WMPEventlogger::getStatus");
	
	edg_wlc_JobId jobid;
	string jobid_str = this->id->toString();
  	// parse the jobID string
  	if (edg_wlc_JobIdParse(jobid_str.c_str(), &jobid)) {
    	edglog(critical)<<"Error during edg_wlc_JobIdParse"<<endl;
    	throw LBException(__FILE__, __LINE__,
			"getStatus()", WMS_LOGGING_ERROR,
			"Error during edg_wlc_JobIdParse");
  	}

  	edg_wll_JobStat * states = NULL;
  	edg_wll_QueryRec jc[2];
  	memset(jc, 0, sizeof jc);
  
  	// job condition: JOBID = jobid
  	jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  	jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  	jc[0].value.j = jobid;
  	jc[1].attr = EDG_WLL_QUERY_ATTR_UNDEF;
  
  	int error;
  	int flag = EDG_WLL_STAT_CLASSADS;
	if (childreninfo) {
		flag = flag | EDG_WLL_STAT_CHILDREN;
	}

#ifdef GLITE_WMS_HAVE_LBPROXY
	if (lbProxy_b) {
		edglog(debug)<<"Quering LB Proxy..."<<endl;
		error = edg_wll_QueryJobsProxy(ctx, jc, flag, NULL, &states);
		if (error == ENOENT) { // no events found
	   		edglog(debug)<< "No status found quering LB Proxy. Quering LB..."<<endl;
			error = edg_wll_QueryJobs(ctx, jc, flag, NULL, &states);
	  	}
	} else { // end switch LB PROXY
#endif  //GLITE_WMS_HAVE_LBPROXY
		edglog(debug)<< "Quering LB..."<<endl;
		error = edg_wll_QueryJobs(ctx, jc, flag, NULL, &states);
#ifdef GLITE_WMS_HAVE_LBPROXY
	} // end switch LB normal
#endif  //GLITE_WMS_HAVE_LBPROXY

  	if (error == ENOENT) { // no events found
   		return glite::lb::JobStatus();
  	}
  	if (error) {
   		///TBC throw an exception???
   		//log_error("Query failed");
    	return glite::lb::JobStatus();
	}

	// Searching last state
	int i = 0;
	while (states[i].state) {
		i++;
	}
	i--;
	
	/*for (int i = 0; events[i].type; i++) {
		edg_wll_FreeStatus(&states[i]);
	}*/
  	
  	return glite::lb::JobStatus(states[i]);
  	
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
const char *
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

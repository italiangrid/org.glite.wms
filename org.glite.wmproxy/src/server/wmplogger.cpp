/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmplogger.h"

#include "wmpexception_codes.h"
#include "wmpexceptions.h"
#include "wmpoperations.h"

#include "glite/lb/producer.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"

#include "glite/security/proxyrenewal/renewal.h"

namespace jobid = glite::wmsutils::jobid;

using namespace std;
using namespace wmproxyname;
using namespace glite::wms::jdl; // DagAd
using namespace glite::wmsutils::jobid; //JobId
using namespace glite::wmsutils::exception; //Exception

const char *WMPLogger::GLITE_WMS_LOG_DESTINATION = "GLITE_WMS_LOG_DESTINATION";

//namespace glite {
//namespace wms {
//namespace wmproxy {

WMPLogger::WMPLogger()
{
	id = NULL;
	if ((edg_wll_InitContext(&ctx))
			|| (edg_wll_SetParam(ctx, EDG_WLL_PARAM_SOURCE,
			EDG_WLL_SOURCE_USER_INTERFACE))) {
		throw JobOperationException(__FILE__, __LINE__,
			"WMPLogger::WMPLogger()",
			WMS_IS_FAILURE, "LB initialisation failed");
	}
};

WMPLogger::~WMPLogger() throw()
{
	edg_wll_FreeContext(ctx);
}

void
WMPLogger::init(const string &lb_host, int lb_port, jobid::JobId *id)
{
	this->id = id;
	this->lb_host = lb_host;
	this->lb_port = lb_port;
	if (!getenv(GLITE_WMS_LOG_DESTINATION)) {
		if (edg_wll_SetParamString(ctx, EDG_WLL_PARAM_DESTINATION,
				lb_host.c_str())) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPLogger::init(const string& lb_host, int lb_port, "
					"jobid::JobId *id)",
				WMS_OPERATION_NOT_ALLOWED, "LB initialisation failed "
					"(set destination)");
		}
	}
}

std::string
WMPLogger::getSequence()
{
	return std::string(edg_wll_GetSequenceCode(ctx));
};


// Registering methods

void
WMPLogger::registerProxyRenewal(const string &proxy_path,
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
			"WMPLogger::registerProxyRenewal(const string &proxy_path, const "
			"string &my_proxy_server)",
			WMS_LOGGING_ERROR, error_message("registerProxyRenewal"));
 	}
}

void
WMPLogger::unregisterProxyRenewal()
{
	char *renewal_proxy_path = NULL;
	cerr<<"---->> ID: "<<id->toString()<<endl;
	for (int i = 0; i < LOG_RETRY_COUNT
		&& edg_wlpr_UnregisterProxy(id->getId(), renewal_proxy_path); i++);
}

void
WMPLogger::registerJob(JobAd *jad)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	cerr<<"JOBAD: "<<jad->toString()<<endl;
	cerr<<"ctx: "<<ctx<<endl;
	cerr<<"id->getId(): "<<id->getId()<<endl;
	cerr<<"jad->toSubmissionString().c_str(): "<<jad->toSubmissionString().c_str()<<endl;
	cerr<<"str_addr: "<<str_addr<<endl;
	/*string proxy = "/tmp/x509up_u503";
	edg_wll_SetParamString(ctx, EDG_WLL_PARAM_X509_PROXY, proxy.c_str());
	string proxyout;*/
	cerr<<"BEFORE edg_wll_RegisterJobSync"<<endl;
	
	//jad->setAttribute(JDL::LB_SEQUENCE_CODE, getSequence());
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
			jad->toSubmissionString().c_str(), str_addr, 0, NULL, NULL)) {
		cerr<<"JobOperationException"<<endl;
		throw JobOperationException(__FILE__, __LINE__,
			"WMPLogger::registerJob(JobAd* jad)",
			WMS_OPERATION_NOT_ALLOWED,
			error_message("edg_wll_RegisterJobSync"));
	}
	if (jad->hasAttribute(JDL::USERTAGS)) {
		logUserTags((classad::ClassAd*) jad->delAttribute(JDL::USERTAGS));
	}
	
	cerr<<"----------- After if"<<endl;
} 

void
WMPLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)
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
	getSandboxDestURIResponse getSandboxDestURI_response;
	int size = sizeof(subjobs) / sizeof(subjobs[0]);
	for (unsigned int i = 0; i < size; i++) {
		// Setting attribute for children jobs
		// method getting jobId, attr name, attr value
		// it should be recursive in the case the job is a dag!! //TBD
		jobidstring = edg_wlc_JobIdUnparse(subjobs[i]);
		jobid = new JobId(jobidstring);
		try {
			getSandboxDestURI(getSandboxDestURI_response, jobidstring);
		} catch (Exception &exc) {
			throw exc;
		} catch (exception &ex) {
			throw ex;
		}
	 	dest_uri = getSandboxDestURI_response.path;
		/*if (!ad->hasNodeAttribute(jobid, JDL::ISB_DEST_URI)) {
			ad->setNodeAttribute(jobid, JDL::ISB_DEST_URI, dest_uri);
		}				//TBD ** UNCOMMENT WHEN CODED **
		job_ad->setNodeAttribute(jobid, JDL::WMPROXY_DEST_URI, dest_uri);*/
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
			"WMPLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)",
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

void
WMPLogger::registerPartitionable(WMPExpDagAd *dag, int res_num)
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
			"WMPLogger::registerPartitionable(WMPExpDagAd *dag, int res_num)",
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
}

void
WMPLogger::registerDag(WMPExpDagAd *dag)
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
			"WMPLogger::registerDag(WMPExpDagAd *dag)",
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
}


// Loggin methods

void
WMPLogger::logAccepted(const std::string &jid)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	edg_wll_LogAccepted(ctx, EDG_WLL_SOURCE_NETWORK_SERVER, lb_host.c_str(),
		str_addr, jid.c_str());
}

void
WMPLogger::logRefused(const std::string &jid)
{
	/*char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	edg_wll_LogAccepted(ctx, EDG_WLL_SOURCE_NETWORK_SERVER, lb_host.c_str(),
		str_addr, jid.c_str());*/
}

void
WMPLogger::logAbort(const char *reason)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", lb_host.c_str(), ":", lb_port);
	edg_wll_LogAbort(ctx, reason);
}

void
WMPLogger::logUserTag(string name, const string &value)
{
	cerr<<"name: "<<name<<endl;
	Ad *classad = new Ad();
	classad->setAttribute(JDL::JDL_ORIGINAL, value);
	logUserTags(classad->ad());
	delete classad;
}

void
WMPLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> > 
	userTags)
{
	for (unsigned int i = 0; i < userTags.size(); i++) {
		if (userTags[i].second->GetKind() != classad::ExprTree::CLASSAD_NODE) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPLogger::logUserTags(std::vector<std::pair<std::string, "
				"classad::ExprTree*> > userTags)",
				WMS_OPERATION_NOT_ALLOWED, "Wrong UserTag value for "
				+ userTags[i].first);
		}
		glite::wmsutils::jobid::JobId sub_jobid(userTags[i].first);
		edg_wll_SetLoggingJob(ctx, sub_jobid.getId(), NULL, EDG_WLL_SEQ_NORMAL);
		logUserTags((classad::ClassAd*)(userTags[i].second));
	}
	edg_wll_SetLoggingJob(ctx, id->getId(), NULL, EDG_WLL_SEQ_NORMAL);
}

void 
WMPLogger::logUserTags(classad::ClassAd* userTags)
{
	vector<pair<string, classad::ExprTree *> > vect;
	classad::Value val;
	string attrValue;
	userTags->GetComponents(vect);
	for (unsigned int i = 0; i< vect.size(); i++) {
		if (!userTags->EvaluateExpr(vect[i].second, val)) {
			// Unable to parse the attribute
			throw JobOperationException(__FILE__, __LINE__,
				"WMPLogger::logUserTags(classad::ClassAd* userTags)",
				WMS_OPERATION_NOT_ALLOWED, "Unable to Parse Expression");
		}
 		if (val.IsStringValue(attrValue)) {
			if (edg_wll_LogUserTag(ctx, (vect[i].first).c_str(), 
					attrValue.c_str())) {
				throw JobOperationException(__FILE__, __LINE__,
					"WMPLogger::logUserTags(classad::ClassAd* userTags)",
					WMS_OPERATION_NOT_ALLOWED,
					error_message("edg_wll_LogUserTag"));
			}
 		}
	}
}

// Error Message Parsing
const char* 
WMPLogger::error_message(const char *api)
{
	char *error_message = (char*) malloc(1024);
	char *msg;
	char *dsc;
	edg_wll_Error(ctx, &msg, &dsc);
	sprintf(error_message, "%s %s %s%s%s%s%s", api,
		getenv(GLITE_WMS_LOG_DESTINATION), "\n", msg, " (", dsc, ")");
	return error_message;
}

//} // wmproxy
//} // wms
//} // glite

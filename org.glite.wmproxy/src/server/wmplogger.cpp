/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmplogger.h"

#include "exception_codes.h"
#include "wmpexceptions.h"
#include "wmpoperations.h"

#include "glite/wmsui/partitioner/Partitioner.h" // REMOVE UI DEPENDENCY

#include "glite/lb/producer.h"

#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/jdl_attributes.h"

#define GLITE_WMS_LOG_DESTINATION "GLITE_WMS_LOG_DESTINATION"

using namespace std ;
using namespace glite::wmsutils::exception ; //Exception
using namespace glite::wmsutils::jobid ; //JobId
using namespace glite::wms::jdl ; // DagAd

namespace jobid = glite::wmsutils::jobid;

using namespace wmproxyname;

//namespace glite {
//namespace wms {
//namespace wmproxy {

WMPLogger::WMPLogger()
{
	id = NULL;
	if ((edg_wll_InitContext(&ctx)) ||
		(edg_wll_SetParam(ctx, EDG_WLL_PARAM_SOURCE, EDG_WLL_SOURCE_USER_INTERFACE))) {
		throw JobOperationException(__FILE__, __LINE__, "WMPLogger::WMPLogger()",
			WMS_IS_FAILURE, "LB initialisation failed");
	}
};

WMPLogger::~WMPLogger() throw()
{
	edg_wll_FreeContext(ctx);
}

void
WMPLogger::init(const string &nsHost, int nsPort, jobid::JobId *id)
{
	this->id = id;
	this->nsHost = nsHost;
	this->nsPort = nsPort;
	if (!getenv(GLITE_WMS_LOG_DESTINATION)) {
		if (edg_wll_SetParamString(ctx, EDG_WLL_PARAM_DESTINATION, nsHost.c_str())) {
			throw JobOperationException(__FILE__, __LINE__,
					"WMPLogger::init(const string& nsHost, int nsPort, jobid::JobId* id)",
					WMS_OPERATION_NOT_ALLOWED, "LB initialisation failed (set destination)");
		}
	}
}

std::string
WMPLogger::getSequence()
{
	return std::string(edg_wll_GetSequenceCode(ctx));
};

void
WMPLogger::setDestinationURI(string dest_uri)
{
	this->dest_uri = dest_uri;
}

void
WMPLogger::logOriginalJdl(const string &jdl) {
	Ad *classad = new Ad();
	classad->setAttribute(JDL::JDL_ORIGINAL, jdl);
	logUserTags(classad->ad());
	delete classad;
}

void
WMPLogger::registerJob(JobAd *jad)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
			jad->toSubmissionString().c_str(), str_addr, 0, NULL, NULL)) {
		throw JobOperationException(__FILE__, __LINE__, "WMPLogger::registerJob(JobAd* jad)",
			WMS_OPERATION_NOT_ALLOWED, error_message("edg_wll_RegisterJobSync"));
	}
}

void
WMPLogger::registerSubJobs(WMPExpDagAd *ad, edg_wlc_JobId *subjobs)
{
	char str_nsAddr[1024];
	sprintf(str_nsAddr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	vector<string> jdls = ad->getSubmissionStrings();
	
	// array of jdls
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
		// it should be recursive in the case the job is a dag!! TBD
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
		}				** UNCOMMENT WHEN CODED **
		job_ad->setNodeAttribute(jobid, JDL::WMPROXY_DEST_URI, dest_uri);*/
	}
	
	cerr<<"WMPLogger::Creating N Sub Jobs of size: "<<jdls.size()<<endl;
	jdls_char = (char**) malloc(sizeof(char*) * (jdls.size() + 1));
	zero_char = jdls_char;
	jdls_char[jdls.size()] = NULL;
	
	int i = 0;
	for (iter = jdls.begin(); iter != jdls.end(); iter++, i++) {
		*zero_char = (char*) malloc(iter->size() + 1);
		sprintf(*zero_char, "%s", iter->c_str());
		cerr<<"WMPLogger::Created Sub: "<<*zero_char<<endl;
		zero_char++;
	}
	cerr<<"WMPLogger::Performing edg_wll_RegisterSubjobs (Dag father is): "<<id->toString()<<endl;
	if (edg_wll_RegisterSubjobs(ctx, id->getId(), jdls_char, str_nsAddr, subjobs)) {
		throw JobOperationException(__FILE__, __LINE__, "WMPLogger::registerJob",
			WMS_OPERATION_NOT_ALLOWED, error_message("edg_wll_RegisterSubjobs"));
	}

	// Release Memory  REMOVE!!
	for ( unsigned int i = 0 ; i < jdls.size() ; i++ ) {
		std::free(jdls_char[i]);
	}
    	std::free(jdls_char);
    	//delete []jdls_char;
};

void
WMPLogger::registerDag(WMPExpDagAd *dag, int res_num)
{
	edg_wlc_JobId *subjobs = NULL;

	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	cerr<<"WMPLogger::registerDag Registering partitiobnable job" << endl;
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
		dag->toString(WMPExpDagAd::NO_NODES).c_str(), str_addr, dag->size(),
		NULL, &subjobs)) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPLogger::registerDag", WMS_OPERATION_NOT_ALLOWED,
				error_message("edg_wll_RegisterJobSync"));
	}
	registerSubJobs(dag, subjobs);
	vector<string> jobids;
	for (unsigned int i = 0; i < res_num; i++) {
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	dag->setJobIds(jobids);
};

void
WMPLogger::registerDag(WMPExpDagAd *ad)
{
	// array of subjob id
	edg_wlc_JobId *subjobs = NULL;

	// Writing the wmp address
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);

	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
		ad->toString(WMPExpDagAd::NO_NODES).c_str(), str_addr, ad->size(),
		NULL, &subjobs)) {
			throw JobOperationException(__FILE__, __LINE__,
				"WMPLogger::registerDag", WMS_OPERATION_NOT_ALLOWED,
				error_message("edg_wll_RegisterJobSync"));
	}
	registerSubJobs(ad, subjobs);
	vector<string> jobids;
	for (unsigned int i = 0; i < ad->size(); i++) {
		cerr << "Appending jobid..." <<  edg_wlc_JobIdUnparse(subjobs[i]) << endl;
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	// Inserting the sub-jobids into the DagAd
	ad->setJobIds(jobids);	
}

void
WMPLogger::transfer(txType tx, const std::string &jdl, const char *error)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	switch (tx) {
		case START:
			if (edg_wll_LogTransferSTART(ctx, EDG_WLL_SOURCE_NETWORK_SERVER,
					nsHost.c_str(), str_addr, jdl.c_str(), error, "")) {
				cerr << error_message("edg_wll_LogTransferSTART") << endl;
			}
			break;
		case OK:
			if (edg_wll_LogTransferOK(ctx, EDG_WLL_SOURCE_NETWORK_SERVER,
					nsHost.c_str(), str_addr, jdl.c_str(), error, "")) {
				cerr << error_message("edg_wll_LogTransferOK") << endl;
			}
			break;
		case FAIL:
			if (edg_wll_LogTransferFAIL(ctx, EDG_WLL_SOURCE_NETWORK_SERVER,
					nsHost.c_str(), str_addr, jdl.c_str(), error, "")) {
				cerr << error_message( "edg_wll_LogTransferFAIL") << endl;
			}
			edg_wll_LogAbort(ctx, error);
			break;
		default:
			break;
	}
}

void
WMPLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> > userTags) {
	for (unsigned int i = 0; i < userTags.size(); i++) {
		if (userTags[i].second->GetKind() != classad::ExprTree::CLASSAD_NODE) {
			throw JobOperationException(__FILE__, __LINE__, "WMPLogger::logUserTags",
				WMS_OPERATION_NOT_ALLOWED, "Wrong UserTags value for "
				+ userTags[i].first);
		}
		glite::wmsutils::jobid::JobId sub_jobid(userTags[i].first);
		cerr<<"logUserTags log for "<<sub_jobid.toString()<<endl;
		edg_wll_SetLoggingJob(ctx, sub_jobid.getId(), NULL, EDG_WLL_SEQ_NORMAL);
		logUserTags((classad::ClassAd*)(userTags[i].second));
	}
	edg_wll_SetLoggingJob(ctx, id->getId(), NULL, EDG_WLL_SEQ_NORMAL);
}

void WMPLogger::logUserTags(classad::ClassAd* userTags) {
	vector<pair<string, classad::ExprTree *> > vect;
	classad::Value val;
	string attrValue;
	userTags->GetComponents(vect);
	for (unsigned int i = 0; i< vect.size(); i++) {
		if (!userTags->EvaluateExpr(vect[i].second, val)) {
			// Unable to parse the attribute
			throw JobOperationException(__FILE__, __LINE__, "WMPLogger::logUserTags",
				WMS_OPERATION_NOT_ALLOWED, "Unable to Parse Expression");
		}
 		if (val.IsStringValue(attrValue)) {
			if (edg_wll_LogUserTag(ctx, (vect[i].first).c_str(), attrValue.c_str())) {
				throw JobOperationException(__FILE__, __LINE__, "WMPLogger::logUserTags",
					WMS_OPERATION_NOT_ALLOWED, error_message("edg_wll_LogUserTag"));
			}
 		}
	}
}

// Error Message Parsing
const char* 
WMPLogger::error_message(const char* api) {
	char* error_message =(char*) malloc (1024);
	char *msg;
	char *dsc;
	edg_wll_Error(ctx, &msg, &dsc); // <<-- CHECK IT
	sprintf(error_message, "%s %s %s%s%s%s%s", api,
		getenv(GLITE_WMS_LOG_DESTINATION), "\n", msg, " (", dsc, ")");
	return error_message;
}

//} // wmproxy
//} // wms
//} // glite

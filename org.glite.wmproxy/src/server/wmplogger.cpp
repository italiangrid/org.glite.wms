#include "wmplogger.h"

#include "exception_codes.h"
#include "wmpexceptions.h"

#include "glite/wmsui/partitioner/Partitioner.h" // REMOVE UI DEPENDENCY

// Logging Class Includes:
#include "glite/lb/producer.h"
//#include "glite/lb/consumer.h"

#define GLITE_WMS_LOG_DESTINATION "GLITE_WMS_LOG_DESTINATION"

using namespace std ;
using namespace glite::wmsutils::exception ; //Exception
using namespace glite::wmsutils::jobid ; //JobId
using namespace glite::wms::jdl ; // DagAd

namespace jobid = glite::wmsutils::jobid;

using namespace wmproxyname;

//namespace glite {
//namespace wms {
//namespace wmproxyname {

pthread_mutex_t  WMPLogger::dgtransfer_mutex  = PTHREAD_MUTEX_INITIALIZER;

WMPLogger::WMPLogger()
{
	id = NULL;
	if ((edg_wll_InitContext(&ctx)) || (edg_wll_SetParam(ctx, EDG_WLL_PARAM_SOURCE, EDG_WLL_SOURCE_USER_INTERFACE))) {
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
WMPLogger::registerJob(JobAd *jad)
{
	char str_addr[1024];
	sprintf (str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_JOB_SIMPLE,
			jad->toSubmissionString().c_str(), str_addr, 0, NULL, NULL)) {
		throw JobOperationException(__FILE__, __LINE__, "WMPLogger::registerJob(JobAd* jad)",
			WMS_OPERATION_NOT_ALLOWED, error_message("edg_wll_RegisterJobSync"));
	}
}

// Registers a partitionable job
ExpDagAd * // void ??
WMPLogger::registerJob(JobAd *jad, int res_number)
{
	char str_addr[1024];
	sprintf (str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	string jdl = jad->toSubmissionString();
	edg_wlc_JobId *subjobs = NULL;
	//cout << "Logging:: Registering partitioning job" << endl ;
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_PARTITIONED,
			jad->toSubmissionString().c_str(), str_addr, res_number, NULL, &subjobs)) {
		//throw JobOperationException(__FILE__, __LINE__,
		//		"WMPLogger::registerJob(JobAd* jad, int res_number)",
		//		WMS_OPERATION_NOT_ALLOWED ,  error_message("edg_wll_RegisterJobSync"));
	}

	// Partitioning job implementation
	vector<string> jobids;
	for (int i = 0; i < res_number; i++) {
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	jad->check();
	glite::wmsui::partitioner::Partitioner part(jad->ad(), jobids);
	ExpDagAd *dagad = new ExpDagAd(part.createDag());
	// cout << "Logging:: created DAG:" << dagad->toString(ExpDagAd::MULTI_LINES) << endl << "Logging::Registering partitioning SUB job" << endl ;
	registerSubJobs(dagad, subjobs);
	return dagad;
};

void
WMPLogger::registerSubJobs(glite::wms::jdl::ExpDagAd *ad, edg_wlc_JobId *subjobs)
{
	char str_nsAddr[1024];
	sprintf(str_nsAddr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	vector<string> jdls = ad->getSubmissionStrings();
	// array of Jdls
	char **jdls_char, **zero_char;
	vector<string>::iterator iter;
	//cout << "Logging::Creating N Sub Jobs of size: " << jdls.size()  << endl ;
	jdls_char = (char**) malloc(sizeof(char*) * (jdls.size() + 1));
	zero_char = jdls_char;
	jdls_char[jdls.size()] = NULL;
	int i = 0 ;
	for (iter = jdls.begin(); iter!=jdls.end(); iter++, i++) {
		*zero_char = (char*) malloc(iter->size() + 1);
		sprintf(*zero_char, "%s",  iter->c_str());
		//cout << "Logging::Created Sub: " <<  *zero_char << endl ;
		zero_char++;
	}
	//cout << "\n\nLogging::Performing edg_wll_RegisterSubjobs (Dag father is) : "<< id->toString() << endl ;
	if (edg_wll_RegisterSubjobs(ctx, id->getId(), jdls_char, str_nsAddr, subjobs)) {
		//throw JobOperationException( __FILE__ , __LINE__ ,  "Logging::registerJob" ,
		//WMS_JOBOP_ALLOWED , error_message( "edg_wll_RegisterSubjobs") ) ;
	}

	// Release Memory  REMOVE!!
	for ( unsigned int i = 0 ; i < jdls.size() ; i++ )  {
		std::free(jdls_char[i]);
	}
    	std::free(jdls_char);
    	//delete []jdls_char;

	//cout << "edg_wll_RegisterSubjobs Done" << endl ;
};

void
WMPLogger::registerDag(glite::wms::jdl::ExpDagAd *ad)
{
	// array of subjob ID's
	edg_wlc_JobId *subjobs = NULL;

	// Writing the ns address
	char str_addr[1024];
	sprintf (str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);

	// Register the job
	if (edg_wll_RegisterJobSync(ctx, id->getId(), EDG_WLL_REGJOB_DAG,
		ad->toString(ExpDagAd::NO_NODES).c_str(), str_addr, ad->size(),
		NULL, &subjobs)) {
			//throw JobOperationException( __FILE__ , __LINE__ ,
			//"Logging::registerJob" , WMS_JOBOP_ALLOWED ,
			//error_message("edg_wll_RegisterJobSync") ) ;
	}
	registerSubJobs(ad, subjobs);
	//cout << "Sub Jobs Registered. now unparsing JobIds..." << endl ;
	vector<string> jobids;
	for (unsigned int i = 0; i < ad->size(); i++ ) {
		//cout << "Appending jobid..." <<  edg_wlc_JobIdUnparse( subjobs[i]) << endl ;
		jobids.push_back(string(edg_wlc_JobIdUnparse(subjobs[i])));
	}
	// Insert the sub-jobids into the DagAd
	ad->setJobIds(jobids);

	// cout << "Done: " <<  endl ; //  ad->toString( ExpDagAd::MULTI_LINES  ) ;
}

void
WMPLogger::transfer(txType tx, const std::string &jdl, const char *error)
{
	char str_addr[1024];
	sprintf(str_addr, "%s%s%d", nsHost.c_str(), ":", nsPort);
	switch (tx) {
		case START:
			if (edg_wll_LogTransferSTART( ctx, EDG_WLL_SOURCE_NETWORK_SERVER , nsHost.c_str(), str_addr , jdl.c_str(), error, "" )) {
				//throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::transfer" , WMS_JOBOP_ALLOWED , error_message( "edg_wll_LogTransferSTART") )
			}
		break;
		case OK:
			if (edg_wll_LogTransferOK(ctx, EDG_WLL_SOURCE_NETWORK_SERVER, nsHost.c_str(), str_addr, jdl.c_str(), error, "" )) {
				cout << error_message( "edg_wll_LogTransferOK") << endl ;
			}
		break;
		case FAIL:
			if (edg_wll_LogTransferFAIL(ctx, EDG_WLL_SOURCE_NETWORK_SERVER, nsHost.c_str(), str_addr, jdl.c_str(), error, "" )) {
				cout << error_message( "edg_wll_LogTransferFAIL") << endl ;
			}
		edg_wll_LogAbort(ctx, error);
		break;
	}
}

void
WMPLogger::logUserTags(std::vector<std::pair<std::string, classad::ExprTree*> > userTags) {
	// cout << "logUserTags iterate over # of jobs: " << userTags.size() << endl ;
	for (unsigned int i = 0; i < userTags.size(); i++) {
		if (  userTags[i].second->GetKind () != classad::ExprTree::CLASSAD_NODE )
			//throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::logUserTags" , WMS_JOBOP_ALLOWED , "Wrong UserTags value for " + userTags[i].first ) ;
		glite::wmsutils::jobid::JobId subJobid(  userTags[i].first   ) ;
		/*cout << "logUserTags log for:: " << subJobid.toString() << endl ;
		edg_wll_SetLoggingJob(  ctx , subJobid.getId() ,  NULL, EDG_WLL_SEQ_NORMAL) ;
		logUserTags (  (classad::ClassAd*)(userTags[i].second)  ) ;*/
	}
	edg_wll_SetLoggingJob(ctx, id->getId(), NULL, EDG_WLL_SEQ_NORMAL);
}

void WMPLogger::logUserTags( classad::ClassAd* userTags ) {
	vector<pair<string, classad::ExprTree *> > vect ;
	classad::Value val ;
	string attrValue ;
	userTags->GetComponents( vect ) ;
	// cout << "Logging::log user tag called... (how many tags TB logged?)   "<<vect.size()   << endl ;
	for (unsigned int i = 0 ; i< vect.size() ; i++){
		if ( !userTags->EvaluateExpr( vect[i].second, val ) )
			// unable to parse the attribute
			//throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::logUserTags" , WMS_JOBOP_ALLOWED , "Unable to Parse Expression" ) ;
		// cout << "Logging::Performing log user tag for      " <<vect[i].first << endl ;
 		if  ( val.IsStringValue (attrValue)  )
			if (      edg_wll_LogUserTag( ctx,  (vect[i].first).c_str() ,  attrValue.c_str()  )     ) {
				//throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::logUserTags" ,
				//						WMS_JOBOP_ALLOWED , error_message( "edg_wll_LogUserTag") ) ;
				}
		// cout << "Logging::logged user tag:      " <<vect[i].first << "      =     " <<attrValue  << endl ;
	}
}

/************************
*   Error Message Parsing
************************/
const char* WMPLogger::error_message ( const char* api  ){
	char* error_message =(char*) malloc ( 1024 ) ;
	char *msg, *dsc ;
	edg_wll_Error(  ctx , &msg , &dsc ) ;
	sprintf ( error_message , "%s %s %s%s%s%s%s", api,
	getenv ( GLITE_WMS_LOG_DESTINATION) , "\n" , msg , " (" , dsc , ")" )  ;
	return error_message ;
} ;

//} // wmproxy
//} // wms
//} // glite

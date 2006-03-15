#include "glite/wmsui/api/Request.h"
#include "glite/wmsui/api/Logging.h"
#include "glite/wmsui/api/JobExceptions.h"
/************************  Network Server ******************/
#include "NSClient.h"
/************************ Request Ad *******************/
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/jdl_attributes.h"
/************************Logging and Bookkeeping  *******************/
#include "glite/lb/producer.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
/********************** VARIOUS *************************************/
#include "glite/wmsutils/jobid/manipulation.h"  // to_filename method

using namespace std ;
using namespace glite::wmsutils::exception ; //Exception
using namespace glite::wmsutils::jobid ; //JobId
using namespace glite::jdl ; // DagAd
using namespace glite::wmsutils::classads;

namespace glite {
namespace wmsui {
namespace api {

/***************************************** GENERAL USED METHODS ************************************/
void checkNs( const string& ns , string& nsHost , int& nsPort ){
	string METHOD = "checkNS( const string& nsAddress , string& nsHost , int& nsPort )" ;
	unsigned int port = ns.find (":") ;
	if ( port > ns.size() - 2 )
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Unable to parse NS address: " +ns ) ;
	nsHost =ns.substr(0, port) ;
	sscanf (ns.substr ( port+1 ).c_str() , "%d" , &nsPort );
}

/**********************************************************  DAG  **************************************************/
/*********************************
*     Constructors/Destructors
**********************************/
Request::Request(){
	loggerLevel = 0;
	type = EWU_TYPE_NONE ;
};
Request::Request(const JobId& id){
	loggerLevel = 0;
	setDagId(id);
};
Request::Request(const  ExpDagAd& ad){
	loggerLevel = 0;
	setDagAd(ad);
};

Request::Request(const  JobAd& ad){
	loggerLevel = 0;
	setJobAd(ad);
};
Request::~Request() throw() {
	// if (jad) delete jad ;
	// if (jid) delete jid ;
	// if (dag) delete dag ;
};

void Request::setDagAd(const ExpDagAd& ad){
	dag = new ExpDagAd ( ad ) ;
	type = EWU_TYPE_DAG_AD ;
};
void Request::setDagId(const JobId& id){
	jid = new JobId ( id );
	type = EWU_TYPE_ID ;
};
void Request::setJobAd(const JobAd& ad){
	jad = new JobAd ( ad );
	type = EWU_TYPE_JOB_AD ;
};

/****************************************************
*     NETWORK SERVER Submission Methods:
****************************************************/


void Request::getOutput(const std::string& dir_path){
	using namespace glite::lb ;
	GLITE_STACK_TRY("Request::getOutput(const std::string& nsHost , int nsPort , const std::string& lbHost , int lbPort)");
	if (     (  type!= EWU_TYPE_ID)  && (type!= EWU_TYPE_SUBMITTED  )   )
		//  Get Output is not allowed
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "OutpuFiles Retrieval not allowed" ) ;
	JobStatus stat = getStatus();
	switch ( stat.status  ){
		case JobStatus::DONE:
			if (stat.getValInt ( JobStatus::DONE_CODE) == 0)
				break;
		case JobStatus::CLEARED:
			throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Output files already successfully retrieved") ;
				break;
		default:
			// Operation not allowed
			throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Output not allowed: check the status ("+stat.name() +")" ) ;
	}
	// Nsclient instance:
	string nsHost ;
	int nsPort ;
	checkNs ( stat.getValString(JobStatus::NETWORK_SERVER ) ,  nsHost , nsPort );
	nsClient =   new glite::wms::manager::ns::client::NSClient   ( nsHost, nsPort , (glite::wms::common::logger::level_t) loggerLevel)    ;
	getOutput (dir_path ,  nsClient-> getSandboxRootPath() , stat) ;
	delete nsClient ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}
/**Private method, perform the job output recoursively (if needed) */
void Request::getOutput(const std::string& dir_path , const std::string& nsRootPath  , const  glite::lb::JobStatus& status ){
	using namespace glite::lb ;
	GLITE_STACK_TRY("Request::getOutput(const std::string& dir_path , const std::string& nsRootPath  , const JobStatus& status ) ");
	vector <JobStatus> states = status.getValJobStatusList( JobStatus::CHILDREN_STATES ) ;
	if ( states.size()>0 ){
		/** DAG: Iterate the get output over all the sons and
		finally purge the Dag files*/
		for ( vector<JobStatus>::iterator it = states.begin() ; it!=states.end() ; it++ )
			getOutput( dir_path + "/" + string ( getlogin()) + "_" +  status.getValJobId( JobStatus::JOB_ID ).getUnique(), nsRootPath , *it ) ;
		if ( nsClient-> jobPurge(  status.getValJobId( JobStatus::JOB_ID ).toString() ))
			cerr << "\nWarning: Unable to purge the output files for the job:\n" +status.getValJobId( JobStatus::JOB_ID ).toString() ;
	}
	else{
		/** JOB:: Create the destination directory and
		Retrieve the output files */
		bool outSuccess = true ;
		string jobid_replaced = to_filename(  status.getValJobId( JobStatus::JOB_ID )   ) ;
		// Create the source path in the remote machine :
		// Nsclient instance:
		string nsHost ;
		int nsPort ;
		checkNs ( status.getValString(JobStatus::NETWORK_SERVER ) ,  nsHost , nsPort );
		string source_path =  "gsiftp://" + nsHost  ; //+ nsRootPath + "/" + jobid_replaced  + "/output/";
		string dest_path = dir_path + "/" + status.getValJobId( JobStatus::JOB_ID ).getUnique();
		string commandSource  = "globus-url-copy " +source_path ;
		string command ;
		vector<string> outputList ;
		nsClient->getOutputFilesList ( status.getValJobId( JobStatus::JOB_ID ).toString() , outputList ) ;
		if ( outputList.size() ==0  ) throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,"No OutputSandbox file(s) returned from: " + nsHost) ;
		string outputErr ;
		//  create directory
		if (mkdir( dest_path.c_str(), 0777) == -1)
			throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,"Unable To create the directory: " + dest_path ) ;
		for  (  vector<string>::iterator it  =  outputList.begin() ; it !=outputList.end() ; it++ ) {
			command = commandSource + *it + " file:"+ dest_path +"/" +it->substr( it->find_last_of( "/") , it->length()  ) ;
			// cout << "This is the launching command..." << command << endl ;
			if (  system ( command.c_str() ) ){
				outputErr+=" " +*it ;
				outSuccess = false ;
			}
		}
		if (!outSuccess)  throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,"Unable to retrieve all output file(s):"+outputErr ) ;
		if ( nsClient-> jobPurge(  status.getValJobId( JobStatus::JOB_ID ).toString() ))
			cerr << "\nWarning: Unable to purge the output files for the job:\n" +status.getValJobId( JobStatus::JOB_ID ).toString() ;
	}
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}


JobId Request::submit(const std::string& nsHost , int nsPort , const std::string& lbHost , int lbPort , const std::string& ceid){
	GLITE_STACK_TRY("Request::submit(const std::string& nsHost , int nsPort , const std::string& lbHost , int lbPort)");
	if (     (  type!= EWU_TYPE_DAG_AD  )&& (  type!= EWU_TYPE_JOB_AD  )   )
		//  Submission is not allowed
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Submission not allowed" ) ;
	// Nsclient instance:
	nsClient =   new glite::wms::manager::ns::client::NSClient   ( nsHost, nsPort , (glite::wms::common::logger::level_t) loggerLevel)    ;
	// Multi attriubte check:
	vector<string> multi ;
	nsClient->getMultiattributeList(multi) ;
	if   (  type== EWU_TYPE_DAG_AD  ){
		// Multi attriubte check TBD
	}else{   /*   type== JOB_AD   */
		jad->checkMultiAttribute( multi ) ;
		if (ceid!= "")  jad->setAttribute( JDL::SUBMIT_TO , ceid);
	}
	// Create unique IDentification
	jid = new JobId() ;
	if (lbPort == 0 ) jid->setJobId(lbHost );
	else  jid->setJobId(lbHost , lbPort);

	// checking certificate
	userCred.checkProxy( ) ;

	log.init ( nsHost , nsPort ,  jid ) ;
	// Perform the Logging RegisterJobSync
	regist();
	// PerForm The Submission
	submit ();
	delete nsClient ;
	type = EWU_TYPE_SUBMITTED ;
	return JobId ( *jid) ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/*** Private method, permofm the submission **/
void Request::submit ( ){
	string jdl ;
	if   (  type== EWU_TYPE_DAG_AD  ){
		// It's a DAG
		log.transfer (Logging::START, dag->toString () ) ;
		dag->setAttribute (   ExpDagAd::SEQUENCE_CODE ,   log.getSequence()  );
		log.logUserTags (  dag->getSubAttributes ( JDL::USERTAGS  )  );
		jdl =  dag->toString () ;
		// cout << "Request::submit>NSClient:: submit_dag: "<< endl ;
	}else{
		// it's a Normal Job
		log.transfer ( Logging::START, jad->toSubmissionString () ) ;
		jad->setAttribute (  JDL::LB_SEQUENCE_CODE, log.getSequence()  );
		if (!jad->hasAttribute(JDL::VIRTUAL_ORGANISATION)){
			jad->setAttribute(JDL::VIRTUAL_ORGANISATION,userCred.getDefaultVoName());
		}
		// UserTags implementation:
		jdl = jad->toSubmissionString () ;
		if (  jad->hasAttribute  (   JDL::USERTAGS  )   ){
			log.logUserTags(  ( classad::ClassAd*)  jad->delAttribute (  JDL::USERTAGS  )   ) ;
		}
		// cout << "Request::submit>NSClient:: submit job" << endl ;
	}
	try{
		if   (  type== EWU_TYPE_DAG_AD  ) 
			nsClient->dagSubmit ( jdl ) ;
		else{
			nsClient->jobSubmit ( jdl ) ;
		}
		// cout << "Request::submit>NSClient:: log Transfer OK" << endl ;
		log.transfer (Logging::OK , jdl) ;
	} catch (Exception &exc){
		// cout << exc.printStackTrace() ;
		log.transfer (Logging::FAIL , jdl , exc.what() ) ;
		throw exc;
	} catch (exception &exc){
		log.transfer (Logging::FAIL , jdl , exc.what() ) ;
		throw exc;
	}
} ;


/***************************************************************************************
* JOB specifics Methods ( not allowed for DAG instancies )
	listMatchingCE
	getState
	attach
***************************************************************************************/
std::vector<std::string> Request::listMatchingCE(const std::string& nsHost , int nsPort ){
	GLITE_STACK_TRY("Request::listMatchingCE(const string& host , int port)") ;
	if (   type!= EWU_TYPE_JOB_AD  )
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Matching CE not allowed" ) ;
	vector<string> resources;
	// Nsclient instance:
	nsClient = new glite::wms::manager::ns::client::NSClient(nsHost,nsPort,(glite::wms::common::logger::level_t)loggerLevel);
	if (!jad->hasAttribute(JDL::VIRTUAL_ORGANISATION)){
		jad->setAttribute (JDL::VIRTUAL_ORGANISATION,userCred.getDefaultVoName()) ;
	}
	nsClient->listJobMatch ( jad->toSubmissionString() , resources ) ;
	delete nsClient ;
	return resources ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/***************************************************************************************
* LOGGING and BOOKKEEPING METHODS:
* getStatus
* getLogInfo
* regist (private method)
***************************************************************************************/
glite::lb::JobStatus Request::getStatus(bool ad)  {
	GLITE_STACK_TRY("Request::getStatus(bool ad)")  ;
	using namespace glite::lb ;
	switch (type){
		case EWU_TYPE_NONE:
		case EWU_TYPE_DAG_AD:
		case EWU_TYPE_JOB_AD:
			throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB information retrieval not allowed" ) ;
		default:
			break;
		}
	// if (! jCollect) //TBD
	userCred.checkProxy() ;
	glite::lb::Job    lbJob (*jid)   ;
	glite::lb::JobStatus  status ;
	if (ad) status =  lbJob.status( glite::lb::Job::STAT_CLASSADS );
	else status = lbJob.status( 0 );
	return    status ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

std::vector <glite::lb::Event> Request::getLogInfo() {
	GLITE_STACK_TRY("Request::getLogInfo()")   ;
	using namespace glite::lb ;
	switch (type){
		case EWU_TYPE_NONE:
		case EWU_TYPE_DAG_AD:
		case EWU_TYPE_JOB_AD:
			throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB information retrieval not allowed" ) ;
		default:
			break;
		}
	userCred.checkProxy() ;
	glite::lb::Job    lbJob (*jid)   ;
	return lbJob.log();
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};



void Request::regist(){
	if   (  type== EWU_TYPE_DAG_AD  ){
		dag->setAttribute (   ExpDagAd::EDG_JOBID , jid->toString()   );
		// cout << "Request::submit>Register Dag" << endl ;
		log.registerDag(  dag    ) ;
	}else{   /*   type== EWU_TYPE_JOB_AD   */
		jad->setAttribute ( JDL::JOBID , jid->toString()   );
		if (  jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_PARTITIONABLE )  ){
			// Partitioning job registration:
			int res_number = 0 ; // TBD JobSteps Check
			vector<string> resources ;
			nsClient->listJobMatch ( jad->toSubmissionString() , resources ) ;
			res_number = resources.size() ;
			if (  jad->hasAttribute (  JDL::PREJOB ) ) res_number++ ;
			if (  jad->hasAttribute (  JDL::POSTJOB ) ) res_number++ ;
			dag =  log.registerJob (jad , res_number) ;
			// Release un-needed memory
			delete jad ;
			type = EWU_TYPE_DAG_AD ;
			// throw JobOperationException     ( __FILE__ , __LINE__ ,"Dag regist" , WMS_JOBOP_ALLOWED , "Submission not performed...it is a partitioning" ) ;
		}else
			// Normal job registering:
			log.registerJob (jad) ;
	}
} ;

} // api
} // wmsui
} // glite

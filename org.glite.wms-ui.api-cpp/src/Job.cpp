/* ********************************************************************
* File name: Job.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2004 by DATAMAT
*********************************************************************/
/* Open SSL include files */
#include <globus_common.h>

// #include "glite/wmsutils/tls/ssl_helpers/ssl_pthreads.h"
// #include "glite/wmsutils/tls/ssl_helpers/ssl_inits.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"  // to_filename method

/**  JDL  */
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/jdl_attributes.h"  // Constant attribute values
#include "glite/wms/jdl/JDLAttributes.h" // JobAd attribute names

/**  CHKPT */
#include "glite/wms/checkpointing/jobstate.h"//Checkpointing

/**  NS */
#include "NSClient.h"// NetworkServer

/*  Logging and Bookkeeping */
#include "glite/lb/JobStatus.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/lb/producer.h"  // some methods are needed

// Dgas
#include "glite/dgas/hlr-clients/job_auth/jobAuthClient.h"

/* C++ Userinterface Api Implementation */
#include "glite/wmsui/api/Job.h"
#include "glite/wmsui/api/JobExceptions.h"
#include "glite/wmsui/api/Listener.h"

namespace glite {
namespace wmsui {
namespace api {

using namespace std ;
using namespace glite::wmsutils::exception ; //Exception
using namespace glite::wmsutils::jobid ; //JobId
using namespace glite::wms::jdl ; // JobAd
using namespace glite::wms::manager::ns::client ; //NSClient
using namespace glite::wms::checkpointing ;
using namespace glite::lb ;
using namespace glite::wms::common::utilities;

pthread_mutex_t  Job::dgtransfer_mutex  = PTHREAD_MUTEX_INITIALIZER;
// Compare two edg_wll_Event using the timestamp member
int cmp_by_timestamp(const void* first, const void* second) {
  timeval f = ((edg_wll_Event *) first)->any.timestamp;
  timeval s = ((edg_wll_Event *) second)->any.timestamp;
  if (( f.tv_sec > s.tv_sec ) ||
      (( f.tv_sec == s.tv_sec ) && ( f.tv_usec > s.tv_usec )))
    return 1;
  if (( f.tv_sec < s.tv_sec ) ||
      (( f.tv_sec == s.tv_sec ) && ( f.tv_usec < s.tv_usec )))
    return -1;
  return 0;
}
Job::Job() {
         jid = NULL ;
         jad = NULL ;
         jobType = JOB_NONE ;
         cred_path = "" ;
         jCollect = false ;
};
Job::Job(const JobId& id){
     GLITE_STACK_TRY("Job::Job(const JobId& id)");
     if (!  ((JobId) id).isSet() )
         throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Empty JobId instance" ) ;
     else {
         this->jid = new   JobId (id) ;
         this->jad = new   JobAd();
         jobType = JOB_ID;
         cred_path = "" ;
         jCollect = false ;
         }
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
Job::Job(const JobAd& ad){
      GLITE_STACK_TRY("Job::Job(const JobAd& ad)") ;
      if (!  ((JobAd) ad).isSet() )
          throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Empty JobAd instance" ) ;
      else{
         this->jad =  new JobAd (ad) ;
         this->jad->check();
         this->jid = new JobId();
         jobType = JOB_AD ;
         cred_path = "" ;
         jCollect = false ;
      }
      GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
//Job Constructor Copy
Job::Job(const Job& job) {
   GLITE_STACK_TRY("Job::Job(const Job& )") ;
   jid = NULL ;
   jad = NULL ;
   jobType = job.jobType ;
   jCollect = job.jCollect ;
   cred_path = job.cred_path ;
   if (job.jid !=NULL){
       jid = new  JobId (*(job.jid)) ;
   }
   if (job.jad !=NULL){
       jad = new  JobAd (*(job.jad)) ;
   }
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

void Job::operator=(const Job& job) {
    GLITE_STACK_TRY("Job::operator=(const Job& job)") ;
    if (jid) delete jid  ;
    if (jad) delete jad ;
    jid = NULL ;
    jad = NULL ;
    jobType = job.jobType ;
    jCollect = job.jCollect ;
    cred_path = job.cred_path ;
    if (job.jid !=NULL)
       jid   = new  JobId (*(job.jid)) ;
    if (job.jad !=NULL)
       jad  = new  JobAd (*(job.jad)) ;
    GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

Job::~Job() {
 GLITE_STACK_TRY("Job::~Job");
   if (jid) delete jid ;
   if (jad) delete jad ;
 GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};


/*******
*  initialise ()
* STatic MEthod
* Prepare SSL initialisation
********/
void Job::initialise( ){
  string METHOD = "Job::initialise( )" ;
  /*
  edg_wlc_SSLInitialization();
#ifndef WITHOUT_THREAD
#else
  if (edg_wlc_SSLLockingInit() != 0)
      throw  ThreadException ( __FILE__ , __LINE__ ,METHOD , THREAD_SSL,  0 )  ;
#endif
  if (globus_module_activate(GLOBUS_COMMON_MODULE) != GLOBUS_SUCCESS)
      throw  ThreadException ( __FILE__ , __LINE__ ,METHOD , THREAD_SSL,  0 )  ;
  */
}

JobId* Job::getJobId()  {
   GLITE_STACK_TRY("Job::getJobId()") ;
   if ((this->jid)  == NULL)
       throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Empty JobId instance" ) ;
   else {
       return this->jid ;
       }
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

JobAd* Job::getJobAd() {
   GLITE_STACK_TRY("Job::getJobAd()") ;
   if (this->jad == NULL)
       throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Empty JobAd instance" ) ;
   return this->jad;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};


/*******************************
*    SET METHODS
setLoggerLevel
setCredPath
unsetCredPath
setJobAd
obId

*******************************/
void Job::setLoggerLevel ( int level ){  loggerLevel = level ; } ;
void Job::setCredPath(const string cp) {
   cred_path=cp ;
   UserCredential uc ;
   uc.checkProxy(cp);
}
void Job::unsetCredPath() {
   cred_path="" ;
   UserCredential uc ;
   uc.checkProxy();
}
void Job::setJobAd(const JobAd& ad)  {
   GLITE_STACK_TRY("Job::setJobAd(const JobAd& ad)") ;
   if  (!jid->isSet()){
      if (jad) delete jad ;
      this->jad =  new JobAd (ad) ;
      this->jad->check();
      jobType = JOB_AD ;
   } else
     throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED, "JobId instance already set" ) ;
 GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}
void Job::setJobId(const JobId& id)  {
   GLITE_STACK_TRY("Job::setJobId(const JobId& id)") ;
     //Usare punt temporanei (controllare che JobId.id has been set!)
   if  (jad  == NULL){
      if (jid) delete jid;
      this->jid = new  JobId(id) ;
      this->jad = new JobAd();
      jobType = JOB_ID ;
      }
   else
     throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED, "JobAd instance already set" ) ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}


void Job::retrieveJobAd() {
   GLITE_STACK_TRY("Job::retrieveJobAd()")
   //Check if the JobId has been already set
   if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
              // Operation not allowed
              throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "retrieveJobAd not allowed" ) ;
   //Retrieve the Status information from the LB
   JobStatus status = getStatus();
   jad->fromString( status.getValString (JobStatus::JDL  ) );
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/* ********************************************************
*  CHECKPOINTABLE METHODS:
*  getState
**********************************************************/
JobState Job::getState( unsigned int step )  {
  GLITE_STACK_TRY("Job::getState(unsigned int step)")  ;
  if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
        // JobId not set yet
        throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "getState not allowed" ) ;
  // Check if the jobType is of chkpt type
  if   (   jobType != JOB_SUBMITTED ){
	JobStatus stat = getStatus( );
	// Initialising internal JobAd:
	jad->fromString (stat.getValString(JobStatus::JDL ) );
	string ns = stat.getValString(JobStatus::NETWORK_SERVER ) ;
	unsigned int port = ns.find (":") ;
	nsHost =ns.substr(0, port) ;
	sscanf (ns.substr ( port+1 ).c_str() , "%d" , &nsPort );
	lbInit ( nsHost) ;
  }
  if (  !jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_CHECKPOINTABLE )  )
     throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Checkpointable retrieval not allowed: not a chkpt job" ) ;

  int                 i, cnt, error;
  edg_wll_Event       *events = NULL;
  edg_wll_QueryRec    jc[2];
  edg_wll_QueryRec    ec[2];
  memset(jc,0,sizeof jc);
  memset(ec,0,sizeof ec);
  // job condition: JOBID = jobid
  jc[0].attr = EDG_WLL_QUERY_ATTR_JOBID;
  jc[0].op = EDG_WLL_QUERY_OP_EQUAL;
  jc[0].value.j = jid->getId() ;
  // event condition: Event type = CHKPT
  ec[0].attr = EDG_WLL_QUERY_ATTR_EVENT_TYPE;
  ec[0].op = EDG_WLL_QUERY_OP_EQUAL;
  ec[0].value.i = EDG_WLL_EVENT_CHKPT;
  // cout << "Job::getState > Performing QUERY" << endl << flush ;
  error = edg_wll_QueryEvents( ctx, jc, ec, &events );
  if ( error == ENOENT )  // no events found
   // return "";
   throw JobOperationException( __FILE__, __LINE__, METHOD ,WMS_JOBOP_ALLOWED, "No events found: ENOENT");
  if ( error )  // query failed
    throw JobOperationException( __FILE__, __LINE__, METHOD,WMS_JOBOP_ALLOWED ,"Query failed");
  for ( cnt=0; events[cnt].type; cnt++ ); // counts the number of events
  if ( !cnt ) // no events found
    throw JobOperationException( __FILE__, __LINE__, METHOD , WMS_JOBOP_ALLOWED, "Empty Events vector returned");
  // cout << "Job::getState > Sorting Events" << endl << flush ;
  // sort the events vector using the timestamp
  qsort(events, cnt, sizeof(edg_wll_Event), &cmp_by_timestamp);
  // the last state in the array is the most recent
  // cout << "Job::getState > Retrieving state and free events...   " << endl << flush ;
  if ((int)step>= cnt)
    throw JobOperationException( __FILE__, __LINE__, METHOD , WMS_JOBOP_ALLOWED, "Number of step bigger then chkpt logged events");
  string state( events[cnt-1-step].chkpt.classad );
  for ( i = 0; i < cnt; i++)
    edg_wll_FreeEvent( events+i );
  free(events);
  // return state;
  // string statestr =    "events[step].getValString( Event::CLASSAD )" ;
  return JobState (  state  ) ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}

/* ********************************************************
*  INTERACTIVE JOB METHODS:
*   attach
**********************************************************/
int Job::attach ( Listener* ls , int port) {
  GLITE_STACK_TRY("Job::attach(int port)")  ;
  //  body TBD:
  if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
        // JobId not set yet
        throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Attach not allowed" ) ;
  // Check if the status of the Job allows to cancel it
  if   (   jobType != JOB_SUBMITTED ){
      JobStatus stat = getStatus( );
      switch (   stat.status  ){
         case JobStatus::SUBMITTED:
         case JobStatus::WAITING:
         case JobStatus::READY:
         case JobStatus::SCHEDULED:
         case JobStatus::RUNNING:
                  break;
         case JobStatus::DONE:
	 	//If the job is DONE, then attachment is allowed only if  DONE_CODE = Failed (1)
		if (stat.getValInt ( JobStatus::DONE_CODE) ==1)
			break ;
         default:  // Any other value: CLEARED ,ABORTED ,CANCELLED, PURGED
              // Operation not allowed
              throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Attachement not allowed: check the status" ) ;
      }
      // Initialising internal JobAd:
      jad->fromString (stat.getValString(JobStatus::JDL ) );
  }
  if (  !jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  )
     throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Attachment not allowed: not an interactive job" ) ;
  sh.set(  *jid , ls) ;
  sh.console(  port );
  // cout << "Check wheter the nsHost has been initialised" << endl << flush ;
  lbInit( nsHost) ;
  if   (   jobType != JOB_SUBMITTED){
          if ( edg_wll_SetLoggingJob(ctx , jid->getId() , NULL , EDG_WLL_SEQ_DUPLICATE) )
               throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB  edg_wll_SetLoggingJob  failed" ) ;
  }
  // cout << "\n                    LB-DBG: "<< "edg_wll_LogListener" << flush ;
  if (edg_wll_LogListener(  ctx , "InteractiveListener", sh.getHost().c_str(), (uint16_t) sh.getPort()) )
          throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB edg_wll_LogListener  failed" ) ;
  sh.start ( ) ;
  return 0 ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}

/* ********************************************************
*  LOGGING AND BOOKKEEPING METHODS:
getStatus
getLogInfo
**********************************************************/
//STATUS Method
JobStatus Job::getStatus(bool ad)  {
   GLITE_STACK_TRY("Job::getStatus(bool ad)")  ;
   // if (jCollect) cout << "\n  Job::getStatus   " << endl << flush ;
   if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
        // Operation not allowed
        throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "getStatus not allowed" ) ;
   if (! jCollect)
       //The job does not belong to a collection : credential check needed
       userCred.checkProxy(cred_path) ;
   glite::lb::Job    lbJob (*jid)   ;
   // if (jCollect) cout << "Job::getStatus   lbJob.status for: " << jid->toString() << endl << flush;
   glite::lb::JobStatus  status ;
   if (ad)
      status =  lbJob.status( glite::lb::Job::STAT_CLASSADS );
   else
      status = lbJob.status( 0 );
   return    status ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

vector<Event> Job::getLogInfo() {
   GLITE_STACK_TRY("Job::getLogInfo()")   ;
     if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
        // Operation not allowed
        throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "getLogInfo not allowed" ) ;
     userCred.checkProxy(cred_path) ;
     // cout << "\n                    LB-DBG: "<< "lbJob (*jid)  " << flush;
     glite::lb::Job    lbJob (*jid)   ;
     // cout << "\n                    LB-DBG: "<< "event= lbJob.log();" << flush;
     vector <Event> event= lbJob.log();
     return event ;
    GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/* ********************************************************
*  NETWORK SERVER METHODS:
submit ( const string& host, int port,  const string& lbHost , int lbPort , JobState *state , Listener *listener , const string& ce_id)
:submit ( const string& host, int port , const string& lbHost , int lbPort , const std::string& ce_id) 
listMatchingCE
cancel


**********************************************************/

void Job::submit ( const string& host, int port,  const string& lbHost , int lbPort , JobState *state , Listener *listener , const string& ce_id)  {
     GLITE_STACK_TRY("Job::submit(const string& , const string& )")
     if   (   jobType != JOB_AD  )
            //  Submission is not allowed
            throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Submission not allowed" ) ;
     //Add ce_id (if present) to JDL
     if (ce_id!= "")
            jad->setAttribute( JDL::SUBMIT_TO , ce_id);
     nsHost = host ;
     nsPort = port ;
     this->lbHost  = lbHost ;
     this->lbPort = lbPort ;
     nsInit( nsHost , nsPort ); //initialise NS
     //Create the jobId and add the dg_jobId field into the JobAd
     if (lbPort == 0 )  jid->setJobId(lbHost );  else     jid->setJobId(lbHost , lbPort);
     jad->setAttribute( (string) JDL::JOBID , jid->toString());
     /** Interactive Job managing:*/
     if (listener != NULL){
	if (  jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  )
		jad->addAttribute( JDL::JOBTYPE ,  (string) JDL_JOBTYPE_INTERACTIVE );
	sh.set(*jid , listener );
     }
     /** Chkpt Job managing:*/
     if (state != NULL){
	if (!   jad->hasAttribute (JDL::JOBTYPE ,  JDL_JOBTYPE_CHECKPOINTABLE )   )
		jad->addAttribute( JDL::JOBTYPE ,  (string) JDL_JOBTYPE_CHECKPOINTABLE );
	state->setId(  jid->toString() );
	// Check the state:
	int error_check = state->checkState() ;
	switch (error_check){
	   case 0:
		//Ok, no errors found
		break;
	   default:
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , error_check  , "JobState instance error" ) ;
		break;
	}
	// add the JobState field to the ClassAd
	Ad ad ( state->toString()  );  //TBD convert State in Ad?
	jad->setAttribute( JDL_CHKPT_JOBSTATE , &ad ) ;
     }
     // Log the event into the LB server:
     nsSubmit ( lbHost );   //Performs all submit common operations
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}


void Job::submit ( const string& host, int port , const string& lbHost , int lbPort , const std::string& ce_id)  {
  GLITE_STACK_TRY("Job::submit(const string& , const string& )")
	if   (   jobType != JOB_AD  )
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , EPERM , "Submission not allowed" ) ;
	if ( ce_id != "" )
		jad->setAttribute ( JDL::SUBMIT_TO , ce_id) ;
	// if ( jCollect) cout << "Job::submit - nsInit" << endl << flush;
	nsInit( host , port ); //initialise NS
	this->lbHost  = lbHost ;
	this->lbPort = lbPort ;
	//Create the jobId and add the dg_jobId field into the JobAd
	// if ( jCollect) cout << "Job::submit - jobad setting" << endl << flush;
	if ( lbPort ==0)   jid->setJobId( lbHost); else jid->setJobId( lbHost , lbPort);
	jad->setAttribute( (string) JDL::JOBID , jid->toString());
	// Interactive Jobs manipulation:
	if (  jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  )  sh.set(*jid );
       // Checkpointable job manipulation
	if (  jad->hasAttribute( JDL::JOBTYPE , JDL_JOBTYPE_CHECKPOINTABLE)  ){
		// State has to be a classAd with the following structure:
		// JobState = [ StateId = "<jobid>" ; JobSteps = <steps>;   CurrentStep = 1 ;  UserData = [] ]
		Ad ad ;
		ad.setAttribute ( JDL_CHKPT_JOBSTATE ,  jid->toString()  );
		ad.setAttribute ( JDL_CHKPT_DATA , "[]" ) ;
		if (jad->hasAttribute ( JDL_CHKPT_STEPS))
			ad.setAttribute ( JDL_CHKPT_CURRENTSTEP , jad->toString(JDL_CHKPT_CURRENTSTEP) );
		else
			ad.setAttribute ( JDL_CHKPT_CURRENTSTEP , 1);
		jad->setAttribute ( JDL_CHKPT_JOBSTATE , &ad ); // TBD tested
	}
	nsSubmit ( lbHost );   //Performs all submit common operations
GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

vector<pair < string, double> > Job::listMatchingCE(const string& host , int port  ) {
	GLITE_STACK_TRY("Job::listMatchingCE(const string& host , int port )")
	vector<pair <string , double> >resources;
	if   (   jobType == JOB_NONE )
		//  Liust Match is not allowed
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "List Match not allowed" ) ;
	nsInit( host , port );
	nsList(resources);
	return resources;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

ResultCode Job::cancel() {
  GLITE_STACK_TRY("Job::cancel()")
  if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
         // Operation not allowed
         throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Cancel not allowed" ) ;
  // Check if the status of the Job allows to cancel it
  JobStatus stat = getStatus( );
  switch (   stat.status  ){
         case JobStatus::SUBMITTED:
         case JobStatus::WAITING:
         case JobStatus::READY:
         case JobStatus::SCHEDULED:
         case JobStatus::RUNNING:
                  break;
         case JobStatus::DONE:
	 	//If the job is DONE, then cancellation is allowed only if  DONE_CODE = Failed (1)
		if (stat.getValInt ( JobStatus::DONE_CODE) ==1)
			break ;
         default:  // Any other value: CLEARED ,ABORTED ,CANCELLED, PURGED
              // Operation not allowed
              throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Cancel not allowed: check the status" ) ;
  }               
  if (  stat.getValBool (JobStatus::CANCELLING )  )
     throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Cancel has been already requested" ) ;
     // return ACCEPTED ;

  if (jobType != JOB_SUBMITTED){
	  // Retrieve NS
	  string ns = stat.getValString(JobStatus::NETWORK_SERVER ) ;
	  unsigned int port = ns.find (":") ;
	  nsHost =ns.substr(0, port) ;
	  sscanf (ns.substr ( port+1 ).c_str() , "%d" , &nsPort );
          // Init the NS
  }
  nsInit (nsHost , nsPort) ;
  nsClient-> jobCancel (  list<string> (1   ,   jid->toString ( ) )   ) ;
  delete nsClient ;
  return ACCEPTED ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}
//GET OUTPUT
void Job::getOutput(const string& dir_path) {
   GLITE_STACK_TRY("Job::getOutput(const string& dir_path)")
   if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
      // Operation not allowed
      throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Output not allowed" ) ;
   // Check if the status of the Job allows to retrieve output files
   JobStatus stat = getStatus();
   switch ( stat.status  ){
         case JobStatus::DONE:
		if (stat.getValInt ( JobStatus::DONE_CODE) == 0)
			break;
         default:
              // Operation not allowed
              throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Output not allowed: check the status ("+stat.name() +")" ) ;
   }
   if (jobType != JOB_SUBMITTED){
	// Retrieve NS
	string ns = stat.getValString(JobStatus::NETWORK_SERVER ) ;
	unsigned int port = ns.find (":") ;
	nsHost =ns.substr(0, port) ;
	sscanf (ns.substr ( port+1 ).c_str() , "%d" , &nsPort );
	// Retrieve JobAd
	jad->fromString (stat.getValString(JobStatus::JDL ) );
	if (!jad-> hasAttribute (JDL::OUTPUTSB)){
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "The Job has no output files to be retrieved" ) ;
	}
	// Change the type of the job
	jobType =  JOB_SUBMITTED;
   }
   nsInit( nsHost , nsPort ); // initialise NS
   nsOutput( dir_path);
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
* Static Method: submit a simple JDL
*******************************************************************/
glite::wmsutils::jobid::JobId* Job::submit(
	const std::string& nsHost , int nsPort ,
	const std::string& lbHost , int lbPort,
	const std::string& executable ,
	const std::string& stdOutput ,
	const std::string& stdErr,
	const std::string& outputDir,
	const std::string& ce_id ,
	int timeout ,
	int time_interval){
	GLITE_STACK_TRY("static JobId* Job::submit " ) ;
	JobAd ad ;

	ad.setAttribute ( JDL::EXECUTABLE , executable ) ;
	ad.setAttribute ( JDL::STDOUTPUT , stdOutput ) ;
	ad.setAttribute ( JDL::STDERROR ,  stdErr ) ;
	ad.addAttribute ( JDL::OUTPUTSB ,  stdErr ) ;
	ad.addAttribute ( JDL::OUTPUTSB ,  stdOutput ) ;
	
	
	// MANDATORY attribute Check:
	ad.setAttributeExpr( JDL::RANK , JDL_RANK_DEFAULT ) ;
	ad.setAttributeExpr( JDL::REQUIREMENTS , JDL_REQ_DEFAULT  ) ;
	UserCredential uc ;
	uc.checkProxy() ;
	try {
		 ad.setAttribute ( JDL::VIRTUAL_ORGANISATION , uc.getDefaultVoName ()   ) ;
	}catch (  Exception exc   ){
		 ad.setAttribute ( JDL::VIRTUAL_ORGANISATION, string ("vo_template")   ) ;
	}
	Job  job (ad ) ;
	job.submit ( nsHost , nsPort , lbHost , lbPort, ce_id) ;
	sleep (time_interval) ;
	int cycles = 0 ;
	while (    cycles < timeout    ){
		JobStatus stat = job.getStatus();
		if(     ( stat.status == JobStatus::DONE ) &&   (  stat.getValInt ( JobStatus::DONE_CODE) == 0  )   )
			break ;
		else {
			cycles ++ ;
			// cout << "Job: " <<  *(job.jid)  << "\nNot yet ready for output files retrieval, please wait..." << endl ;
			sleep (time_interval) ;
		}
	}
	if ( cycles == timeout )
		throw JobOperationException ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "Timeout exceeded for output retrieval" ) ;
	// Retrieve the output and exit
	job.getOutput( outputDir) ;
	return job.jid ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}




/******************************************************************
* Protected Methods Network Server APIs:
nsInit
nsList
nsSubmit
nsOutput

*******************************************************************/
void Job::nsInit(const string& host ,int  port){
	GLITE_STACK_TRY("Job::nsInit(const string& nsAddress,int nsPort)")
	if (! jCollect)  userCred.checkProxy(cred_path) ;
	nsHost = host ;
	nsPort = port ;
	nsClient = new NSClient   (nsHost, nsPort , (glite::wms::common::logger::level_t)loggerLevel)    ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
void Job::nsList( vector<pair <string, double> > &resources ) {
	GLITE_STACK_TRY("Job::nsList(vector<string> *resources )")
	vector<string> multi ;
	nsClient->getMultiattributeList(multi) ;
	if (!jad->hasAttribute ( JDL::VIRTUAL_ORGANISATION)  )
		jad->setAttribute (JDL::VIRTUAL_ORGANISATION ,  userCred.getDefaultVoName() ) ;
	jad->checkMultiAttribute( multi) ;
	string jdl = jad->toSubmissionString();
	nsClient->listJobMatch ( jdl , resources ) ;
	delete nsClient ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
// nsSubmit   NS API calling:
void Job::nsSubmit(const string&  lb_addr ) {
	GLITE_STACK_TRY("Job::nsSubmit()")
	// NS multi attribute check
	try{
		vector<string> multi ;
		pthread_mutex_lock( & dgtransfer_mutex);    //LOCK RESOURCES    -->
		nsClient->getMultiattributeList(multi) ;
		pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <--
		jad->checkMultiAttribute( multi) ;
	}catch (  Exception &exc  ) {
		pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <-- In case of multi attribute failed
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , exc.what()  ) ;
	}
	// OutputData changes:
	if (jad->hasAttribute ( JDL::OUTPUTDATA) )
		jad->addAttribute (JDL::OUTPUTSB , JDL::DSUPLOAD + "_" + jid->getUnique()+".out"  ) ;
	if (!jad->hasAttribute ( JDL::VIRTUAL_ORGANISATION)  )
		jad->setAttribute (JDL::VIRTUAL_ORGANISATION ,  userCred.getDefaultVoName() ) ;

     //Set The Jdl string
     string jdl = jad->toSubmissionString() ;
     // LB PROCEDURES:
     // Initialise the context:
     lbInit (nsHost);
     // Log the submission to the LB server:
     char str_addr [1024];
     sprintf (str_addr , "%s%s%d" , nsHost.c_str(),":",nsPort );
     // cout << "Registering job..." <<  jid->toString()  << endl ;
     if (edg_wll_RegisterJobSync ( ctx , jid->getId() ,   EDG_WLL_JOB_SIMPLE ,  jdl.c_str(),  str_addr, 0 ,   NULL, NULL) ){
           char error_message [1024];
           char *msg, *dsc ;
           edg_wll_Error(  ctx , &msg , &dsc ) ;
           sprintf ( error_message , "%s%s%s%s%s%s%s","Unable to perform  edg_wll_RegisterJobSync  at: ",
           getenv ( GLITE_WMS_LOG_DESTINATION) , "\n" , msg , " (" , dsc , " )" )  ;
           throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,  error_message) ;
     }
     // cout << "Job properly Registered.." <<  jid->toString() << endl ;
     // CheckPointable Job:
     if (  jad->hasAttribute( JDL::JOBTYPE , JDL_JOBTYPE_CHECKPOINTABLE)  ){
        string current_step ;
        if (  ! jad->hasAttribute( JDL::CHKPT_CURRENTSTEP ) )
             current_step = "First checkpoint state without JobSteps" ;
        else
             current_step = "1" ;
        if (edg_wll_LogEventSync( ctx, EDG_WLL_EVENT_CHKPT ,EDG_WLL_FORMAT_CHKPT , current_step.c_str() , jad->getAd(JDL_CHKPT_JOBSTATE).toString().c_str() ) )
              throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB  edg_wll_LogEventSync failed" ) ;
        jad->delAttribute(JDL_CHKPT_JOBSTATE );
     }
     // Interactive Job:
     if (  jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  ){
        int port = 0 ;
        if( jad->hasAttribute (JDL::SHPORT))
           port = jad->getInt ( JDL::SHPORT ) ;
        sh.console(port);
	if (edg_wll_LogListener(  ctx , "InteractiveListener", sh.getHost().c_str(), (uint16_t) sh.getPort()) )
	   throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB edg_wll_LogListener  failed" ) ;
	char  attribute [1024];
	// Insert the Interactive Environment variables required for the server listener:
	sprintf (attribute , "%s%s%s" , JDL_INTERACTIVE_STDIN , "=" , sh.getPipeIn().c_str() ) ;
	jad->addAttribute (  JDL::ENVIRONMENT , string(attribute) );
	sprintf (attribute , "%s%s%s" , JDL_INTERACTIVE_STDOUT , "=" , sh.getPipeOut().c_str() ) ;
	jad->addAttribute (  JDL::ENVIRONMENT , string(attribute) );
	sprintf (attribute , "%s%s%s" , JDL_INTERACTIVE_STDERR , "=" , sh.getPipeErr().c_str() ) ;
	jad->addAttribute (  JDL::ENVIRONMENT , string(attribute) );
	sprintf (attribute , "%s%s%d" ,JDL_INTERACTIVE_SHADOWPORT , "=" , sh.getPort() ) ;
	jad->addAttribute (  JDL::ENVIRONMENT , string(attribute) );
	sprintf (attribute , "%s%s%s" , JDL_INTERACTIVE_SHADOWHOST , "=" , sh.getHost().c_str() ) ;
	jad->addAttribute (  JDL::ENVIRONMENT , string(attribute) );
	}
	// Interactive Job:
	if (  jad->hasAttribute(JDL::USERTAGS )  ){
		classad::ClassAd* userTags = (classad::ClassAd*)jad->delAttribute( JDL::USERTAGS ) ;
		vector< pair< string, classad::ExprTree *> > vect ;
		classad::Value val ;
		string attrValue ;
		userTags->GetComponents( vect ) ;
		// cout << "Logging::log user tag called... (how many tags TB logged?)   "<<vect.size()   << endl ;
		for (unsigned int i = 0 ; i< vect.size() ; i++){
			if ( !userTags->EvaluateExpr( vect[i].second, val ) )
				// unable to parse the attribute
				throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::logUserTags" , WMS_JOBOP_ALLOWED , "Unable to Parse Expression" ) ;
			// cout << "Logging::Performing log user tag for      " <<vect[i].first << endl ;
			if  ( val.IsStringValue (attrValue)  )
				if (      edg_wll_LogUserTag( ctx,  (vect[i].first).c_str() ,  attrValue.c_str()  )     )
					throw JobOperationException     ( __FILE__ , __LINE__ ,  "Logging::logUserTags",WMS_JOBOP_ALLOWED , "edg_wll_LogUserTag failed") ;
			// cout << "Logging::logged user tag:      " <<vect[i].first << "      =     " <<attrValue  << endl ;
		}
	}


// pthread_mutex_lock( & dgtransfer_mutex);    //LOCK RESOURCES    -->
     sprintf (str_addr , "%d" , nsPort );
     if (edg_wll_LogTransferSTART( ctx,
		EDG_WLL_SOURCE_NETWORK_SERVER ,
		nsHost.c_str(),
		str_addr ,
		jdl.c_str(), "", "" )) cerr << "Warning: edg_wll_LogTransferSTART failed" << endl ;
// pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <--
	jad->setAttribute (JDL::LB_SEQUENCE_CODE , string ( edg_wll_GetSequenceCode(ctx)  ) );
	jdl = jad->toSubmissionString();
	try{
		pthread_mutex_lock( & dgtransfer_mutex);    //LOCK RESOURCES    -->
		// cout << "Job::nsSubmit> proper submission" << endl ;
		nsClient->jobSubmit( jdl   );
		// cout << "Job::nsSubmit> proper submission END" << endl ;
		pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <--
	} catch (Exception &exc){
		pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <--
		//Call the dgLog Transfer method  for failure (No Thread Safe Method)
		// cerr << "\n                    LB-DBG: "<< "edg_wll_LogTransferFAIL & Log Abort!!!\n Exception caught: "<< exc.what() << endl ;
		if (edg_wll_LogTransferFAIL(ctx, EDG_WLL_SOURCE_NETWORK_SERVER , nsHost.c_str(), str_addr , jdl.c_str(), exc.what(),""))
			cerr << "Warning: edg_wll_LogTransferFAIL  Failed"<< endl ;
		edg_wll_LogAbort (ctx , exc.what() );
		throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , exc.what()  ) ;
	}
	this->jobType = JOB_SUBMITTED ;
	//Call the dgLog Transfer method (No Thread Safe Method)
	// pthread_mutex_lock( & dgtransfer_mutex);    //LOCK RESOURCES    -->
	if (edg_wll_LogTransferOK(ctx, EDG_WLL_SOURCE_NETWORK_SERVER , nsHost.c_str(),  str_addr , jdl.c_str(), "",""))
		cerr << " Warning edg_wll_LogTransferOK Failed" << endl ;
// pthread_mutex_unlock( & dgtransfer_mutex);  //UNLOCK RESOURCES   <--
	if ( jad->hasAttribute(JDL::HLR_LOCATION) ){
		//DGAS Job Authorisation manipulation2
		jobAuth_data jobAuth ;
		jobAuth.dgJobId = jid->toString() ;
		string ptr;
		string hlr = jad->getString ( JDL::HLR_LOCATION ) ;
		try{
			if (dgas_jobAuth_client ( hlr, jobAuth , &ptr )) cerr << "dgas_jobAuth_client failed" << endl ;
		}catch (exception &exc){ cerr << "dgas_jobAuth_client failed:" << exc.what() << endl;}
	}
	edg_wll_FreeContext(ctx) ;
	delete nsClient ;
	// If everithing run well, launch the listener implementation:
	if (  jad->hasAttribute(JDL::JOBTYPE , JDL_JOBTYPE_INTERACTIVE )  )  sh.start();
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
// get_output_sandbox   NS API:
void Job::nsOutput( const string& finalDir ){
	GLITE_STACK_TRY("Job::nsOutput( const string& jobId , const string& dir )")
	bool outSuccess = true ;
	string source_path =  "gsiftp://" + nsHost ;
	string dest_path = " file:" + finalDir ;
	string commandSource  = "globus-url-copy " + source_path ;
	string command ;
	vector<string> outputList ;
	nsClient->getOutputFilesList (jid->toString() , outputList ) ;
	if ( outputList.size() ==0  ) throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,"No OutputSandbox file(s) returned from: " + nsHost) ;
	string outputErr ;
	for  (  vector<string>::iterator it  =  outputList.begin() ; it !=outputList.end() ; it++ ) {
		if (  it->find_last_of( "/") > it->length() ){
			//Unable to find Separator
			outputErr += " " + *it ;
			outSuccess = false ;
		}else{
			command = commandSource + *it + dest_path +"/" + it->substr( it->find_last_of( "/")  ) ;
			// cout << "Job::nsOutput >>> GLOBUS URL COPY COMMAND: " << command << endl ;
			if (  system ( command.c_str() ) ){
				outputErr += " " + *it ;
				outSuccess = false ;
			}
		}
	}
	if (!outSuccess)  throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED ,"Unable to retrieve all output file(s):"+outputErr ) ;
	if ( nsClient-> jobPurge( jid->toString() ))  cerr  << "Warning:  jobPurge failed" << endl ;
	delete nsClient ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
* Protected Methods Logging & Bookkeeping APIs:
*******************************************************************/
void Job::lbInit(  const string& nsHost  ) {
     string METHOD = "lbInit(const string& nsHost)"  ;
     if(
        (edg_wll_InitContext( &ctx )) ||
        (edg_wll_SetParam( ctx, EDG_WLL_PARAM_SOURCE, EDG_WLL_SOURCE_USER_INTERFACE ) )
     ) throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB initialisation failed" ) ;
     if   ( ! getenv ( GLITE_WMS_LOG_DESTINATION) )
          if (edg_wll_SetParamString( ctx, EDG_WLL_PARAM_DESTINATION, nsHost.c_str() ) )
               throw JobOperationException     ( __FILE__ , __LINE__ ,METHOD , WMS_JOBOP_ALLOWED , "LB initialisation failed (set destination)" ) ;
}

} // api
} // wmsui
} // glite

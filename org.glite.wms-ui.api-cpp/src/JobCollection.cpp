/***************************************************************************
*  filename  : JobCollection.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
**************************************************************************/
#include "glite/wms/jdl/JobAd.h"
#include "glite/wmsui/api/JobCollection.h"
#include "glite/wmsui/api/JobExceptions.h"
#include "glite/wmsui/api/CredentialException.h"
#include "glite/lb/JobStatus.h"
#include "glite/lb/producer.h"
// requestad
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wmsutils/jobid/JobId.h"
// NetworkServer
#include "NSClient.h"

using namespace glite::wmsustils::exception ;
using namespace glite::wmsutils::jobid ;
using namespace glite::wms::lb ;
using namespace std ;

USERINTERFACE_NAMESPACE_BEGIN //Defining UserInterFace NameSpace

/******************************************************************
 methods :  ResultStruct Methods:
*******************************************************************/
void resultStruct::set (  ResultCode res  )   {    result = res;  };
/******************************************************************
 methods: Constructors, Copy-Constructor and Operations
*******************************************************************/
//Constructor
JobCollection::JobCollection() {
  // SSL thread Safe
  GLITE_STACK_TRY("JobCollection::JobCollection()") ;
  Job::initialise() ;
  maxThreadNumber = 1 ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
//   Constructor from a vector
JobCollection::JobCollection(const vector<Job>& jobs) {
  GLITE_STACK_TRY("JobCollection::JobCollection(const vector<Job>& jobs)") ;
  vector <Job>::const_iterator it ;
  for (it = jobs.begin() ; it != jobs.end() ; it++)  insert(*it)  ;
  Job::initialise() ;
  maxThreadNumber = 1 ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
// Constructor from a Job (AD type) copied n times into the collection
JobCollection::JobCollection(const Job& job , unsigned int n) {
 GLITE_STACK_TRY("JobCollection::JobCollection(const Job& job , unsigned int n) ") ;
 if   (   ((Job) job).getType() == JOB_AD) {
     for (unsigned int i = 0 ;  i <n ; i++){
              jobs.push_back( job );
              (jobs[i]).setCollect();
     }
 } else
       throw   JobCollectNoJobException ( __FILE__ , __LINE__ , METHOD ,WL_DUPLICATE_JOB );   //TBD other code
  Job::initialise() ;       
  maxThreadNumber = 1 ;
 GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
 method :  insert
*******************************************************************/
void JobCollection::insert(const Job& job){
	GLITE_STACK_TRY("JobCollection::insert(const Job& job)") ;
	cout << "JobCollection::insert> checking jobid" << endl ;
	switch ( job.jobType){
		case JOB_ID:{
			//The JobId has been already set: chek if a previous identical jobId has been inserted (throw)
			string id  = job.jid->toString()   ;
			vector<Job>::iterator   jobIt;
			for (jobIt = jobs.begin() ;  jobIt!=jobs.end() ; ++jobIt )
				if    (   jobIt->jid->toString()   == id   )
					throw   JobCollectNoJobException ( __FILE__ , __LINE__ , METHOD ,WL_DUPLICATE_JOB , id);
			break;
		}
		case JOB_AD:
			break;
		default:
			throw   JobCollectNoJobException ( __FILE__ , __LINE__ , METHOD ,WL_DUPLICATE_JOB );    //TBD other code
	}
/* OLD CHECK
    if  (job.jobType == JOB_NONE) {      //The Job Is empty, cannot insert
                throw   JobCollectNoJobException ( __FILE__ , __LINE__ , METHOD ,WL_DUPLICATE_JOB );    //TBD other code
    }  else if (       job.jid->isSet()  ) {
         //The JobId has been already set: chek if a previous identical jobId has been inserted (throw)
         string id  = job.jid->toString()   ;
         vector<Job>::iterator   jobIt;
         for (jobIt = jobs.begin() ;  jobIt!=jobs.end() ; ++jobIt ){
             if    (   jobIt->jid->toString()   == id   )
                   throw   JobCollectNoJobException ( __FILE__ , __LINE__ , METHOD ,WL_DUPLICATE_JOB , id);
         }
    }
*/
	//The JobId has not been set yet or no duplicate jobIdfound: job can be added to the vector
	jobs.push_back( job );
	(jobs[size()-1]).setCollect();
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
 method :  remove
*******************************************************************/
void JobCollection::remove (const Job& job){
     GLITE_STACK_TRY("JobCollection::remove (const Job& job)") ;
     vector<Job>::iterator   jobIt;
     bool cleared = false ;
     string jobString;
     try{
        string jobId  =  job.jid->toString() ;
        jobString=jobId;
         for (jobIt = jobs.begin() ; jobIt!=jobs.end() ; ++jobIt ){
           try {
             if (  jobIt->jid->toString() == jobId   ){
                   this->jobs.erase(jobIt);
                   cleared = true ;
                   break;
                 }
           } catch  (glite::wmsustils::exception::Exception &exc){
                 //The job is not of Id type do nothing
           }
         }
     }  catch  (glite::wmsustils::exception::Exception &exc){
/* TBD
          //No JobId Found: looking for JobAd
          try{
             string jobAd =( ( (Job)job).getJobAd() )->toString();
             jobString=jobAd;
             for (jobIt = jobs.begin() ; jobIt!=jobs.end() ; ++jobIt ) {
                 try {
                   if (( jobIt->getJobAd() )->toString() == jobAd   ){
                      this->jobs.erase(jobIt);
                      cleared = true ;
                      break;
                   }
                 } catch  (glite::wmsustils::exception::Exception &exc){
                   //The job is not of Ad type   do nothing
                 }
             }
          }catch  (glite::wmsustils::exception::Exception &exc)  {
          //No Jobad found ---> ERROR
            throw JobCollectNoJobException ( __FILE__ , __LINE__ ,METHOD ,WL_NOSUCHJOB ,jobString);
          }
*/    //TBD
     }
     if (! cleared)    {
         throw  JobCollectNoJobException ( __FILE__ , __LINE__ ,METHOD ,WL_NOSUCHJOB ,jobString);
         }
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
 method :   set logger level
*******************************************************************/
void JobCollection::setLoggerLevel ( int level ){
for ( unsigned int i = 0 ; i < jobs.size() ; i++ )
     jobs[i].setLoggerLevel (level ) ;
} ;

/******************************************************************
 method :   set/unset CredPath
*******************************************************************/
void JobCollection::setCredPath(const std::string cp) {  cred_path=cp ;  userCred.checkProxy(cp) ; }
void JobCollection::unsetCredPath() {  cred_path="" ;   userCred.checkProxy() ; }

/******************************************************************
 methods :        getStatus
*******************************************************************/
CollectionResult  JobCollection::getStatus(   ){
     GLITE_STACK_TRY("JobCollection::getStatus(vector <pair< Job , JobStatus > >)") ;
     operation = STATUS ;//returning variable:
     paramStruct       ps   ;
     return launch (ps) ;
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods :  submit
*******************************************************************/
//TBD lb--> lbVector
CollectionResult JobCollection::submit(const string& ns_host , int ns_port , vector < pair < string , int > > lbAddrs , const string& ce_id ){
  GLITE_STACK_TRY("JobCollection::submit(const string& ns_host, int ns_port , const string& lbHost , int lbPort , const string& ce_id )") ;
  operation = SUBMIT ;
  // Contact the NS address:
  glite::wms::manager::ns::client::NSClient *nsClient ;
  nsClient = new  glite::wms::manager::ns::client::NSClient   ( ns_host , ns_port ) ;
  lbAddresses = lbAddrs ;
  time_t   zeroTime = time(NULL);
  lbPosition =   (rand() + (int) zeroTime ) % lbAddrs.size() ;
  paramStruct       ps   ;
  ps.parA.assign(ns_host) ;
  ps.parB.assign (ce_id)  ;
  ps.parX = ns_port  ;
  delete nsClient ;
  return launch (ps) ;
  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods :       cancel
*******************************************************************/
CollectionResult  JobCollection::cancel( ){
   GLITE_STACK_TRY("JobCollection::cancel( const string &email)") ;
   operation = CANCEL ;
   paramStruct       ps   ;
   return launch (ps) ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods :    getOutput
*******************************************************************/
CollectionResult  JobCollection::getOutput(const string& dir_path){
 GLITE_STACK_TRY("JobCollection::getOutput(const string& dir_path)") ;
   operation = OUTPUT ;
   paramStruct       ps   ;
   ps.parA.assign( dir_path + "/" + getlogin() + "_" ) ;
   return launch (ps) ;
 GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
 methods :  ExecuteThread
*******************************************************************/
pthread_t JobCollection::ExecuteThread(void* (*fn)(void*), void *arg) {
    GLITE_STACK_TRY("JobCollection::ExecuteThread(void* (*fn)(void*), void *arg)");
   // All functions return 0 on success  and  a  non-zero  error code  on  error.
   pthread_t tid ;
   pthread_attr_t pthread_attr;
   //INIT
   int jobNumber =(    (struct paramStruct* )arg    )->jobNumber ;
   int errorCode = pthread_attr_init(&pthread_attr) ;
   if (errorCode !=0)
         throw  ThreadException ( __FILE__ , __LINE__ ,METHOD , THREAD_INIT,  jobNumber )  ;
   //DETACH
   errorCode = pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE) ;
   if (errorCode != 0 )
         throw   ThreadException ( __FILE__ , __LINE__ ,METHOD , THREAD_DETACH  , jobNumber ) ;
   //CREATE
   errorCode =  pthread_create(&tid, &pthread_attr   , fn, arg) ;
   if (errorCode!= 0  )
        throw   ThreadException ( __FILE__ , __LINE__ ,METHOD ,  THREAD_CREATE, jobNumber) ;
   return tid ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods : /Retrieve Paramethers
*******************************************************************/
resultStruct   JobCollection::retrieve ( pthread_t tid ,  int  jobNumber){
     GLITE_STACK_TRY("JobCollection::retrieve ( pthread_t tid , int  jobNumber)") ;
     resultStruct *res = NULL ;
     int joint =  pthread_join( tid ,  (void**) &res) ;
     if ( !  joint   )  {
          resultStruct result = *res ;
          delete res ;
          return result ;
     }else  if (joint ==ESRCH) {
          return resultStruct (JOIN_FAILED , "Unable To Join the thread");
     }else
          throw  ThreadException ( __FILE__ , __LINE__ ,METHOD , THREAD_INIT, jobNumber )  ;
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 * methods :  launch
 * Depending on the required operation, perform collection
 * This Method is the core of the job  collection
*******************************************************************/
CollectionResult JobCollection::launch( const paramStruct  &params ){
     GLITE_STACK_TRY("JobCollection::launch( const paramStruct &params )") ;
     cout << "JobCollection::launch" << endl ;
     if (jobs.size()==0)
       throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WL_JOBOP_ALLOWED ,"The collection is empty" ) ;
     //Check If the Credential are OK
     string subj , issuer;
     int cred_type, strength, time_left;
     userCred.getInfo ( subj,  issuer,  cred_type,  strength,  time_left, cred_path);
     if (time_left  <  PROXY_TIME_LIMIT)
                  throw  ProxyException ( __FILE__ , __LINE__ , METHOD );
     time_t   zeroTime = time(NULL);

     int maxTh = (getMaxThreadNumber() > size()) ? ( size() ) : (getMaxThreadNumber()) ;  // The largest number between MAX_number of threads and collection size
     int numJob= 0 ; // count the jobs in the collection
     int numTh = 0 ; //Count the thread
     // returning variable:
     vector<pair<string , resultStruct>  > jobResults (   size()  );
     // CollectionResult jobResults (   size()  );
#ifndef WITHOUT_THREAD
     //These variables are used only with Threads
     int freeTh = maxTh ; //pointing to the first free thread
     pthread_t             tid          [maxTh] ;  //vector of thread identificators
     paramStruct       ps            [maxTh] ;  //vector of structures
#endif
     vector<Job>::iterator   jobIt;
     for (jobIt = jobs.begin() ; jobIt!=jobs.end() ; ++jobIt ){
	jobIt->setLoggerLevel ( loggerLevel ) ;
	// Prepare the job for the required operation:
	switch (operation) {
		case SUBMIT:
			// jobIt->subject = issuer ;
			jobIt->lbHost =  lbAddresses[ numTh % lbAddresses.size()].first ;
			jobIt->lbPort =  lbAddresses[ numTh % lbAddresses.size()].second ;
			//multi attrib?? TBD
			break;
		default: /* No ERROR, continue*/ break ;
	} //end switch operation
#ifdef WITHOUT_THREAD
	/*  Non-Threaded Collection operation. Wait the end of one job till process to the following */
	resultStruct*  res ;
	switch (operation) {
		case SUBMIT:  res = ( resultStruct* )  (    submitTo      ((void*) &params  )     ); break;
		case STATUS:  res = ( resultStruct* )  (    statusTo       ((void*) &params  )     ); break;
		case OUTPUT :  res = ( resultStruct* ) (    getOutputTo ((void*) &params  )     ); break;
		case CANCEL:  res = ( resultStruct* ) (    cancelTo     ((void*) &params  )     ); break;
		default: /* TBD Some error occurred */ break ;
	}
	string id = jobs[numJob].jid->isSet()? jobs[numJob].jid->toString() : "job not submitted" ;
	jobResults[numJob] = pair<string , resultStruct>  (  id    , res  ) ; //TBD res has been created
	delete res ;
	numTh++ ;
#else
               /*
	       * Threaded collection operation: Launch several parallel job processes
	       */
               // Fill in the parameter info:
	       // could make a switch where assign only the requested parameter(s) TBD check
	       // for each command which parameter(s) are initilalised
               ps[numTh%maxTh].parA.assign(params.parA) ;
               ps[numTh%maxTh].parB.assign (params.parB)  ;
               ps[numTh%maxTh].parX =   params.parX;
               ps[numTh%maxTh].job = jobIt ;
               ps[numTh%maxTh].jobNumber =  numJob ;
	       // EXECUTE Thread:
               switch (operation) {
               case SUBMIT:
	          lbPosition = ( lbPosition + 1 ) %lbAddresses.size();
	          ps[numTh%maxTh].parC =  lbAddresses[lbPosition%lbAddresses.size()].first   ;
	          ps[numTh%maxTh].parY =   lbAddresses[lbPosition%lbAddresses.size()].second   ;
	          tid[numTh%maxTh] = ExecuteThread ( submitTo ,     (void*) &ps[numTh%maxTh]);
	          break;
               case OUTPUT:   tid[numTh%maxTh] = ExecuteThread ( getOutputTo , (void*) &ps[numTh%maxTh]); break;
               case STATUS:   tid[numTh%maxTh] = ExecuteThread ( statusTo ,       (void*) &ps[numTh%maxTh]); break;
               case CANCEL:  tid[numTh%maxTh] = ExecuteThread ( cancelTo ,     (void*) &ps[numTh%maxTh]); break;
               default:
                   break;
               }  // end switch operation
               numTh++ ;
               //CHECK if max Thread has been reached:
               if (   numTh%maxTh ==  freeTh%maxTh ){
                  //Check if proxy is valid
                  if (    difftime (time(NULL) , zeroTime) < time_left    )  {
                         time_left = userCred.checkProxy() ;
                         zeroTime = time(NULL);
                  }
                  // WAITING  for a Thread to finish   and retrieve result
                  resultStruct   retrievedResult =  retrieve(    tid [  freeTh%maxTh  ] ,  ps[  freeTh%maxTh  ].jobNumber ) ;
                  string id = jobs[   ps[  freeTh%maxTh  ].jobNumber    ].jid->isSet()?  jobs[   ps[  freeTh%maxTh  ].jobNumber    ].jid->toString()  : "job not submitted" ;
                  jobResults[ ps[  freeTh%maxTh  ].jobNumber    ]      =  pair<string , resultStruct> (  id   ,  retrievedResult  ) ;
                  freeTh++ ;
               }
#endif
            //increase the associated job indentificator number
            numJob++ ;
     }  //End jobs cycle
     //If some Thread is still running then wait for them:
#ifndef WITHOUT_THREAD
     while (freeTh%maxTh != numTh%maxTh) {
                  // cout << "Retrieving info from thread..." << endl << flush ;
                  resultStruct   retrievedResult =  retrieve(    tid [  freeTh%maxTh  ] ,  ps[  freeTh%maxTh  ].jobNumber ) ;
                  string id = jobs[   ps[  freeTh%maxTh  ].jobNumber    ].jid->isSet()?   jobs[   ps[  freeTh%maxTh  ].jobNumber    ].jid->toString()  : "job not submitted" ;
                  jobResults[ ps[  freeTh%maxTh  ].jobNumber    ]      =  pair<string , resultStruct> (  id   ,      retrievedResult   ) ;
                  freeTh++ ;
     }
#endif
  CollectionResult cr ;
  cr.result = jobResults ;
  return cr ;
  // return CollectionResult (jobResults) ;

  GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods :  submit     STATIC Thread Method
*******************************************************************/
void* JobCollection::submitTo  (void* paramStruct) {
	GLITE_STACK_TRY("JobCollection::submitTo  (void* paramStruct)") ;
	//Retrieve Parameters from paramStruct
	const string ns = (   (struct paramStruct* )paramStruct)-> parA ;
	const string ce = (   (struct paramStruct* )paramStruct)-> parB ;
	const string lb = (   (struct paramStruct* )paramStruct)-> parC ;
	int nsPort = (   (struct paramStruct* )paramStruct)-> parX ;
	int lbPort = (   (struct paramStruct* )paramStruct)-> parY ;
	//Call the submit ns api
	try{
		(   (struct paramStruct* )paramStruct )->job->submit(ns, nsPort, lb , lbPort, ce );
		return  (void*)    new resultStruct (SUCCESS) ;
	} catch ( exception &exc ){
		return  (void*)    new resultStruct ( SUBMISSION_FAILURE , exc.what() ) ;
	}
	GLITE_STACK_CATCH() ;  //remove last SET_NAME (exited with throw)
};


/******************************************************************
 methods :  cancel     STATIC Thread Method
*******************************************************************/
void* JobCollection::cancelTo (void* ps) {
	GLITE_STACK_TRY("JobCollection::cancelTo (void* ps)") ;
	try{
		ResultCode code = (   (struct paramStruct* )ps )->job->cancel(  );
		return  (void*)    new resultStruct (   code   ) ;
	} catch (exception &exc){
		return  (void*)    new resultStruct ( CANCEL_FAILURE, exc.what() ) ;
	}
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

//Function cancel  (SYNC)
/******************************************************************
 methods :  getStatus     STATIC Thread Method
*******************************************************************/
void* JobCollection::statusTo (void* paramStruct) {
	GLITE_STACK_TRY("JobCollection::statusTo (void* paramStruct)") ;
	try{
		glite::wms::lb::JobStatus *status  =   new   glite::wms::lb::JobStatus(  ) ;
		*status = ( (struct paramStruct* )paramStruct )->job->getStatus()   ;
	return  (void*)    new resultStruct ( SUCCESS , *status  )  ;
	} catch  (exception &exc){
		return  (void*)    new resultStruct ( STATUS_FAILURE, exc.what()  )  ;
	}
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 methods :   getOutput    STATIC Thread Method
*******************************************************************/
void* JobCollection::getOutputTo (void* paramStruct) {
	GLITE_STACK_TRY("JobCollection::getOutputTo (void* paramStruct)");
	string dir_path = (   (struct paramStruct* )paramStruct)-> parA ;
	try{
		if (mkdir( dir_path.c_str(), 0777) == -1)
			throw JobOperationException  ( __FILE__ , __LINE__ ,METHOD , WL_JOBOP_ALLOWED ,"Unable To create the directory: " + dir_path ) ;
		(   (struct paramStruct* )paramStruct )->job->getOutput(dir_path);
		return  (void*)    new resultStruct ( SUCCESS )  ;
	} catch  ( exception &exc){
		// If possible remove path
		rmdir ( dir_path.c_str() ) ;
		return  (void*)    new resultStruct (GETOUTPUT_FAILURE, exc.what() ) ;
	}
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

USERINTERFACE_NAMESPACE_END } //Closing  UserInterFace NameSpace

/* ********************************************************************
* File name: UserJobs.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
*********************************************************************/
#include "edg/workload/userinterface/client/UserJobs.h"
#include "edg/workload/userinterface/client/CredentialException.h"
#include "edg/workload/userinterface/client/JobExceptions.h"
// #include  "edg/workload/userinterface/client/NotificationAd.h" Deprecated class
// Common
#include "edg/workload/common/jobid/JobId.h"
//LB includes:
#include "edg/workload/logging/client/ServerConnection.h"


USERINTERFACE_NAMESPACE_BEGIN //Defining UserInterFace NameSpace
using namespace std ;
using namespace glite::wmsustils::exception ;
using namespace glite::wmsutils::jobid ;
using namespace glite::wms::lb ;

/******************************************************************
 method :  Constructors /Destructors
*******************************************************************/
UserJobs::UserJobs () { } ;
UserJobs::~UserJobs () {} ;
UserJobs::UserJobs (const string cred) {
 EDG_STACK_TRY("UserJobs (const string cred_path)");
 cred_path= cred ;
 EDG_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 };
/******************************************************************
 method :  getJobs
*******************************************************************/
 void UserJobs::getJobs ( const string &lbHost , int lbPort ,  vector<JobId>  &jobs){
   EDG_STACK_TRY("getJobs   (string lb_address ,  vector <JobId> &jobs");
     uc.checkProxy(cred_path) ;
     ServerConnection server ;
     server.setQueryServer(lbHost , lbPort);
     server.userJobs(jobs) ;
   EDG_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 }
/******************************************************************
 method :  getStatus
*******************************************************************/
void UserJobs::getStatus   (const string& lbHost , int lbPort,  vector <JobStatus> &jobsStates) {
EDG_STACK_TRY("getStatus   (string lb_address ,  vector <JobStatus> &jobsStatus");
   uc.checkProxy(cred_path) ;
   ServerConnection server ;
   server.setQueryServer(lbHost , lbPort);
   server.userJobStates(jobsStates) ;
EDG_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
USERINTERFACE_NAMESPACE_END } //Closing  UserInterFace NameSpace

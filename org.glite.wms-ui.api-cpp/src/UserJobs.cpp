/* ********************************************************************
* File name: UserJobs.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
*********************************************************************/
#include "glite/wmsui/api/UserJobs.h"
#include "CredentialException.h"
#include "glite/wmsui/api/JobExceptions.h"
// #include  "glite/wmsui/api/NotificationAd.h" Deprecated class
// Common
#include "glite/wmsutils/jobid/JobId.h"
//LB includes:
#include "glite/lb/ServerConnection.h"

namespace glite {
namespace wmsui {
namespace api {

using namespace std ;
using namespace glite::wmsutils::exception ;
using namespace glite::wmsutils::jobid ;
using namespace glite::lb ;

/******************************************************************
 method :  Constructors /Destructors
*******************************************************************/
UserJobs::UserJobs () { } ;
UserJobs::~UserJobs () {} ;
UserJobs::UserJobs (const string cred) {
 GLITE_STACK_TRY("UserJobs (const string cred_path)");
 cred_path= cred ;
 GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 };
/******************************************************************
 method :  getJobs
*******************************************************************/
 void UserJobs::getJobs ( const string &lbHost , int lbPort ,  vector<JobId>  &jobs){
   GLITE_STACK_TRY("getJobs   (string lb_address ,  vector <JobId> &jobs");
     uc.checkProxy(cred_path) ;
     ServerConnection server ;
     server.setQueryServer(lbHost , lbPort);
     server.userJobs(jobs) ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 }
/******************************************************************
 method :  getStatus
*******************************************************************/
void UserJobs::getStatus   (const string& lbHost , int lbPort,  vector <JobStatus> &jobsStates) {
GLITE_STACK_TRY("getStatus   (string lb_address ,  vector <JobStatus> &jobsStatus");
   uc.checkProxy(cred_path) ;
   ServerConnection server ;
   server.setQueryServer(lbHost , lbPort);
   server.userJobStates(jobsStates) ;
GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

} // api
} // wmsui
} // glite

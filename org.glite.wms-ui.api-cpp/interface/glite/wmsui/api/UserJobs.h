#ifndef GLITE_WMS_UI_CLIENT_USERJOBS_H
#define GLITE_WMS_UI_CLIENT_USERJOBS_H
/*
 * UserJobs.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */

#include  "Job.h"
#include <list>

// ClassAd Definition
namespace classad {
class ClassAd ; 
}

// JobId Definition
namespace glite { 

namespace wmsutils { 
namespace jobid { 
class JobId ; 
} 
} 

namespace wmsui {
namespace api {

#define UJ_CANCEL_ERR           -1
/**
 * Allow controlling all the jobs owned by the user
 * The UserJobs class provides methods that allow controlling all the user's jobs during its lifetime.
 * such as getting their status, or cancelling them

 *
 * @brief  Allow controlling all the jobs owned by the user
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class UserJobs{
   public:

/**@name Constructors/Destructor */
//@{
      /** Instantiates an  UserJobs object using default user credential*/
      UserJobs () ;
      /**Destructor*/
      ~UserJobs ();
      /** Instantiates an  UserJobs object using non-default credential
      *@param cred_path The proxy certificate file where to retrive credential from*/
      UserJobs (const std::string cred_path) ;
//@}

/**@name Action Methods */
	//@{

	/** Retreive the jobs owned by the user in a specific LB
	* @param lbHost the LB server host name
	* @param lbPort the LB server port value
	* @param  jobs the vector of JobId which will be filled with all the user jobs
	*/
	void getJobs ( const std::string& lbHost, int lbPort,  std::vector<glite::wmsutils::jobid::JobId> &jobs ) ;

	/** Retrieve the status of all the user's jobs
	* @param lbHost the LB server host name
	* @param lbPort the LB server port value
	* @param jobsStatus the vector to be filled of all the jobs status information*/
	void getStatus   (const std::string& lbHost , int lbPort ,  std::vector <glite::lb::JobStatus> &jobsStatus) ;
//@}

   private:
      UserCredential uc ;
      std::string cred_path ;

};

} // api
} // wmsui
} // glite

#endif

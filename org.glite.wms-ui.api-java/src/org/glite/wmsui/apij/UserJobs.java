/*
* UserJobs.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
import java.util.* ;
import java.io.File ;
import  org.glite.jdl.Ad ;
import org.globus.gsi.GlobusCredentialException;
import java.lang.Math ; // to perform the power
/**
 * Allow controlling all the jobs owned by the user
 * The UserJobs class provides methods that allow controlling all the user's jobs during its lifetime.
 * such as getting their status, logging information, or cancelling them
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class UserJobs{
	/** Instantiates an  UserJobs object using default user credential
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate
	*/
	public UserJobs () throws  GlobusCredentialException , java.io.FileNotFoundException{
		userCred= new UserCredential ( );
	}
	/** Instantiates an  UserJobs object using specified user credential
	* @param cp The full path of the proxy certificate file to be set
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate
	*/
	public UserJobs ( File cp  ) throws  GlobusCredentialException , java.io.FileNotFoundException{
		userCred= new UserCredential ( cp   );
	}
	/**
	* Set a different  Proxy certificate from the default one and check it
	* @param cp  The full path of the proxy certificate file to be set
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate */
	public void setCredPath( File cp)  throws  GlobusCredentialException {
		userCred.setProxy(cp);
	}
	/**Set the Proxy certificate as default
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate */
	public void unsetCredPath() throws GlobusCredentialException, java.io.FileNotFoundException {
		userCred.unsetProxy();
	}
	/** Retreive the jobs owned by the user in a specific LB
	* @param lbAddress  the full Logging and Bookkeeping address
	* @return A vector containing all the jobs owned by the user for the specified LB address
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate
	*/
	public Vector getJobs (  Url lbAddress )  throws  GlobusCredentialException, java.io.FileNotFoundException {
		Api api = new Api() ;  api.initialise() ;
		userCred.checkProxy() ;
		Vector vect = api.lbGetUserJobs ( lbAddress     ) ;
		return getJobs ( lbAddress , null , null ,  new Ad() , new boolean[JobStatus.MAX_STATUS] , new boolean[JobStatus.MAX_STATUS]  , true ) ;
	}
	/** Retreive a vector of JobStatus that match the query for the specified LB
	* @param lbAddress  the full Logging and Bookkeeping address
	* @param query filter specifying all the requirements of the search
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate */
	public Vector getJobs ( Url lbAddress , Query query)  throws  GlobusCredentialException, java.io.FileNotFoundException {
		// return getJobs ( lbAddress , query.getFrom() , query.getTo () , query.getUserTags()  , query.getIncludes () , query.getExcludes() , query.getOwned() );
		Api api = new Api() ;  api.initialise() ;
		userCred.checkProxy() ;
		long fromInt , toInt;
		fromInt = (query.getFrom()==null) ? 0: query.getFrom().getTimeInMillis()/1000 ;
		toInt    =  (query.getTo()   ==null ) ? 0: query.getTo().getTimeInMillis()/1000 ;
		// Check includes excludes values
		int resultInEx = 0 ;
		for (int i = 0 ; i< JobStatus.MAX_STATUS ;i++){
			if (query.getInclude(i) ) {
				resultInEx += Math.pow( 2 , i ) ;
			}
		}
		if (resultInEx==0){
			for (int i = 0 ; i< JobStatus.MAX_STATUS ;i++){
				if (query.getExclude(i) ) {
					resultInEx += Math.pow( 2 , i ) ;
				}
				// remember: Negative resultInEx means reverse (UNEQUAL) query
			}
			resultInEx *=(-1) ;
		}
		Vector vect = api.lbGetJobs (    lbAddress , fromInt , toInt , query.getUserTags() , resultInEx, query.getOwned()?userCred.getIssuer( ):"" ) ;
		return vect ;
	}
	/** Retreive a vector of JobStatus that match the query for the specified LB
	*@deprecated use instead getJobs ( Url lbAddress , Query query)
	public Vector getJobs ( Url lbAddress , Calendar from , Calendar to , Ad userTags,boolean owned )  throws  GlobusCredentialException, java.io.FileNotFoundException {
		boolean [] includes = new boolean[JobStatus.MAX_STATUS];
		boolean [] excludes = new boolean[JobStatus.MAX_STATUS];
		return getJobs ( lbAddress , from , to , userTags, includes, excludes, owned ) ;
	}
	*/
	/** Retreive a vector of JobStatus that match the query for the specified LB
	* @param lbAddress  the full Logging and Bookkeeping address
	* @param from filter selecting only jobs submitted after the specified time
	* @param to  filter selecting only jobs submitted before the specified time
	* @param userTags Ad instance specifying a list of <userTag name>=<userTag value>
	* @param includes filter selecting only jobs with specified states (cannot be specified with excludes parameter) is a boolean array of JobStatus.MAX_STATUS dimension
	* @param excludes filter excluding only jobs with specified states  (cannot be specified with includes parameter) is a boolean array of JobStatus.MAX_STATUS dimension
	* @param owned filter selecting only jobs owned by user
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate 
	*@deprecated use instead getJobs ( Url lbAddress , Query query)  */
	public Vector getJobs ( Url lbAddress , Calendar from , Calendar to , Ad userTags, boolean [] includes, boolean [] excludes, boolean owned )  throws  GlobusCredentialException, java.io.FileNotFoundException {
		Api api = new Api() ;  api.initialise() ;
		userCred.checkProxy() ;
		long fromInt , toInt;
		fromInt = (from==null) ? 0: from.getTimeInMillis()/1000 ;
		toInt    = (  to==null ) ? 0: to.getTimeInMillis()/1000 ;
		// Check includes excludes values
		int resultInEx = 0 ;
		for (int i = 0 ; i< JobStatus.MAX_STATUS ;i++){
			if (includes[i] ) {
				resultInEx += Math.pow( 2 , i ) ;
			}
		}
		if (resultInEx==0){
			for (int i = 0 ; i< JobStatus.MAX_STATUS ;i++){
				if (excludes[i] ) {
					resultInEx += Math.pow( 2 , i ) ;
				}
				// remember: Negative resultInEx means reverse (UNEQUAL) query
			}
			resultInEx *=(-1) ;
		}
		Vector vect = api.lbGetJobs (    lbAddress , fromInt , toInt , userTags , resultInEx, owned?userCred.getIssuer( ):"" ) ;
		return vect ;
	}
	/** Retrieve the status of all the user's jobs
	* @param  lbAddress  the full Logging and Bookkeeping address
	* @return  the vector to be filled of all the jobs status information
	* @throws FileNotFoundException Unable to find certificates files
	* @throws GlobusCredentialException - Unable to get the specified proxy certificate
	*/
	public Vector getStates   ( Url lbAddress ) throws GlobusCredentialException, java.io.FileNotFoundException {
		Api api = new Api() ;  api.initialise() ;
		userCred.checkProxy() ;
		Vector vect = api.lb_get_user_jobs_status ( lbAddress) ;
		return vect ;
	}
	private UserCredential userCred ;
};

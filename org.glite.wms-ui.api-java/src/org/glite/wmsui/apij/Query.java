/*
* Query.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij ;
import java.util.* ;
import  org.glite.wms.jdlj.Ad ;
import javax.naming.directory.InvalidAttributeValueException ;
/**
 * LB QeuryServer Class Wrapped.
 * <p>
 * This class is used to specify a filter while performing LB server jobs information
 * It allows the user to specify the
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
 */
public class Query {
	/**  Default Constructor*/
	public Query () {
		owned = false ;
		userTags = new Ad () ;
		includes = new boolean[JobStatus.MAX_STATUS] ;
		excludes = new boolean[JobStatus.MAX_STATUS] ;
		from = null ;
		to = null ;
	}
	/** Make a deep copy of the current query */
	public Query copy () {
		Query q = new Query() ;
		q.owned = owned ;
		q.userTags = userTags ;
		q.from= from ;
		q.to = to ;
		q.includes = new boolean[JobStatus.MAX_STATUS] ;
		q.excludes = new boolean[JobStatus.MAX_STATUS] ;
		System.arraycopy(includes , 0 , q.includes , 0 ,  JobStatus.MAX_STATUS) ;
		System.arraycopy(excludes , 0 , q.excludes , 0 ,  JobStatus.MAX_STATUS) ;
		return q ;
	}
	/** Clean all the filter in the current Query */
	public void clean () {
		owned = false;
		userTags = new Ad();
		from = null;
		to = null ;
		includes = new boolean[JobStatus.MAX_STATUS] ;
		excludes = new boolean[JobStatus.MAX_STATUS] ;
	};
	/** Retrieve current query string representation
	* @return query string representation */
	public String  toString() {
		String result = new String () ;
		result += "Only owned jobs : " + owned ;
		if (from != null) result += "\njobs submitted since: " + from.getTime() ;
		if (to != null) result += "\njobs submitted untill: " + to.getTime()  ;
		if (userTags.size() >0 ) result += "\nOnly Jobs with usertags: " + userTags.toString (true , true ) ;
		for (int i = 0 ; i < JobStatus.MAX_STATUS ; i++ ){
			if ( includes[i] ) result += "\n- Retrieving  all '" + JobStatus.code[i] +"' jobs" ;
			if ( excludes[i] ) result += "\n- Excluding all '" + JobStatus.code[i] +"' jobs" ;
		}
		return result ;
	}
	/***********
	* Set Methods:
	************/
	/**
	* get all jobs submitted after the specified time
	* @param time Calendar representing the specified time
	*/
	public void setTimeFrom( Calendar time) { from = time ; }
	/**
	* get all jobs submitted before the specified time
	* @param time Calendar representing the specified time
	*/
	public void setTimeTo( Calendar time) { to = time ;}
	/**
	* get all jobs submitted after the specified time
	* @param time number of seconds since epoch
	*/
	public void setTimeFrom( int time ) {
		from = new GregorianCalendar () ;
		from.setTime( new Date (time) ) ;
	}
	/**
	* get all jobs submitted before the specified time
	* @param time number of seconds since epoch
	*/
	public void setTimeTo( int time) {
		to = new GregorianCalendar () ;
		to.setTime( new Date ( time  )) ;
	}
	/**
	* Set all userTags couples in the query. Only jobs containing such tags will be retrieved
	* If the attribute already exists is automatically replaced
	* @param tags a valid ad containing all usertags information
	*/
	public void setUserTags( Ad tags ){
		userTags= (Ad)(tags.clone());
	}
	/**
	* get all jobs submitted with the specified user tag
	* If the attribute already exists is automatically replaced
	* @param name the name of the user tag
	* @param value the value of the user tag
	* @throws InvalidAttributeValueException when the couple name-value has classad syntax error
	*/
	public void setUserTag( String name , String value) throws InvalidAttributeValueException{
		try{
			if (userTags.hasAttribute ( name ) ) userTags.delAttribute (name) ;
		}catch  ( NoSuchFieldException exc ) {
			throw new InvalidAttributeValueException ("Fatal Error: NoSuchFieldException thrown" );
		}
		userTags.setAttribute ( name , value) ;
	}
	/**
	* Retrieve only current certificate owned jobs
	*/
	public void setOwned () {  this.owned = true ;  }
	/***********
	* Status filter Methods :
	************/
	/**
	* Including only the jobs in the specified status.
	*  This method could be called  repetively in order to include different states
	* @param status_code must be a valid JobStatus code
	* @see JobStatus#UNDEF
	* @see JobStatus#SUBMITTED
	*@see JobStatus#WAITING
	*@see JobStatus#READY
	*@see JobStatus#SCHEDULED
	*@see JobStatus#RUNNING
	*@see JobStatus#DONE
	*@see JobStatus#CLEARED
	*@see JobStatus#ABORTED
	*@see JobStatus#CANCELLED
	*@see JobStatus#UNKNOWN
	*@see JobStatus#PURGED
	*/
	public void setInclude ( int status_code ) {
		if(    ( status_code < JobStatus.UNDEF ) || (status_code >= JobStatus.MAX_STATUS)   )
			throw new ArrayIndexOutOfBoundsException  ("status code :" + status_code +" is out of range" ) ;
		includes [ status_code ] = true ;
	};
	/**
	* Exclude all the jobs in the specified status.
	* @param status_code must be a valid JobStatus code
	* @see JobStatus#UNDEF
	* @see JobStatus#SUBMITTED
	*@see JobStatus#WAITING
	*@see JobStatus#READY
	*@see JobStatus#SCHEDULED
	*@see JobStatus#RUNNING
	*@see JobStatus#DONE
	*@see JobStatus#CLEARED
	*@see JobStatus#ABORTED
	*@see JobStatus#CANCELLED
	*@see JobStatus#UNKNOWN
	*@see JobStatus#PURGED  */
	public void setExclude ( int status_code ){
		if(    ( status_code < JobStatus.UNDEF ) || (status_code >= JobStatus.MAX_STATUS)   )
			throw new ArrayIndexOutOfBoundsException  ("status code :" + status_code +" is out of range" ) ;
		excludes [ status_code ] = true ;
	} ;
	/***********
	* Get Methods:
	************/
	/**
	* Retrieve the set from filter
	* @return a Calendar representing the time when to look for the jobs from (Null if not set) */
	public Calendar getFrom( ) {return from;}
	/**
	* Retrieve the set to filter
	* @return a Calendar representing the time when to look for the jobs to (Null if not set) */
	public Calendar getTo( ) { return to ;}
	/**
	* retrieve UserTag value
	* @param name the name of the user tag looked for
	* @return the value for the specified userTag (Null if not set)
	* @throws NoSuchFieldException if the specified attribute is not set */
	public String getUserTag( String name )  throws NoSuchFieldException {
		return  userTags.getStringValue ( name).get(0).toString() ;
	}
	/**
	* Retrieve all the query user tags
	*@return an Ad class containing all the usertags names and values (empty Ad if no user tag set yet) */
	public Ad getUserTags () { return userTags ; }
	/**
	* retrieve the owned flag
	*@return true if flag has been set, false otherwise  */
	public boolean getOwned () { return owned; }
	/**
	* Retrieve the value of the restriction for the specified status
	* @return true if the parameter has been set, false otherwise
	* @param status_code must be a valid JobStatus code
	* @see JobStatus#UNDEF
	* @see JobStatus#SUBMITTED
	*@see JobStatus#WAITING
	*@see JobStatus#READY
	*@see JobStatus#SCHEDULED
	*@see JobStatus#RUNNING
	*@see JobStatus#DONE
	*@see JobStatus#CLEARED
	*@see JobStatus#ABORTED
	*@see JobStatus#CANCELLED
	*@see JobStatus#UNKNOWN
	*@see JobStatus#PURGED 
	*/
	public boolean getInclude ( int status_code ) {
		if(    ( status_code < JobStatus.UNDEF ) || (status_code >= JobStatus.MAX_STATUS)   )
			throw new ArrayIndexOutOfBoundsException  ("status code :" + status_code +" is out of range" ) ;
		return includes[status_code];
	};
	/**
	* Retrieve the value of the restriction for the specified status
	* @param status_code must be a valid JobStatus code
	* @return true if the parameter has been set, false otherwise
	* @see JobStatus#UNDEF
	* @see JobStatus#SUBMITTED
	*@see JobStatus#WAITING
	*@see JobStatus#READY
	*@see JobStatus#SCHEDULED
	*@see JobStatus#RUNNING
	*@see JobStatus#DONE
	*@see JobStatus#CLEARED
	*@see JobStatus#ABORTED
	*@see JobStatus#CANCELLED
	*@see JobStatus#UNKNOWN
	*@see JobStatus#PURGED  
	*/
	public boolean getExclude ( int status_code ){
		if(    ( status_code < JobStatus.UNDEF ) || (status_code >= JobStatus.MAX_STATUS)   )
			throw new ArrayIndexOutOfBoundsException  ("status code :" + status_code +" is out of possible range" ) ;
		return excludes[status_code];
	}
	/**
	* Retrieve the filter on the included states
	* @return an array of booleans, each value represents whether the
	*/
	boolean [] getIncludes () { return includes ;  }
	/**
	* Retrieve the filter on the excluded states
	*/
	boolean [] getExcludes () { return excludes ;  }
	/***********
	* Private memebers:
	************/
	private boolean owned ;
	private Ad userTags ;
	private Calendar from , to ;
	private boolean [] includes ;
	private boolean [] excludes ;
}

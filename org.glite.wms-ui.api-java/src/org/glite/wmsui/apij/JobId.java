/*
* JobId.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij ;
/**
 * Managing Identification, checking, retreiving info from a job
 * File name: JobId.h
 * The JobId class provides a representation of the Datagrid job identifier
 * (dg_jobId) and the methods for manipulating it.
 * We remind that the format of the dg_jobId is as follows:
 * <LB address>/<unique string>
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it> */
import java.lang.* ;
public class JobId {
	/** Instantiates an  empty  JobId object */
	public JobId(){   jobId =null ;     } ;
	/**Intantiates a JobId object providing full information: LB server address and unique string 
	* @param lbAddress  the Logging and Bookkeeping server  address
	* @param unique a Unique identifier */
	public JobId(Url lbAddress, String unique ) { setJobId (lbAddress , unique ) ; }
	/**
	* Instantiates a JobId object from the passed dg_jobId in String format.
	* @param  jid  a String representig a classAd expression
	* @throws  IllegalArgumentException - When the String is passed in a wrong format
	* @throws NumberFormatException Unable to find the integer LB server port number inside the jobid */
	public JobId(String jid ) throws IllegalArgumentException, NumberFormatException  { fromString( jid )   ; }
	/**
	* Check if the job identifiers are equals
	* @param jid the JobId instance to check for equality
	* @return true if the job identifiers are the same, false otherwise*/
	public boolean equals (JobId jid){
		if (  isSet() ) return    jobId.equals (jid.jobId)   ;
		return false ;
	}
	/** Unsets the JobId instance. Clear all it's memebers */
	public void   clear() { jobId = null ;  }
	/**
	* Check whether the jobId has been already created (true) or not (false)
	*@return  true (jobId created) or false (jobId not yet created)  */
	public boolean isSet() { return (  jobId!=null  ) ; }
	/**
	* Converts the jobId into a String
	*@return a string representing the job identifiers (if set), null otherwise*/
	public String toString (){
		if (   isSet()   ) return jobId ;
		else return null ;
	}
	/**
	* Set the JobId instance from the String given as input.
	* @param jid the job identifier string representation
	* @throws  IllegalArgumentException Wrong jobId format
	* @throws NumberFormatException Unable to find the integer LB server port number inside the jobid
	*/
	public void fromString (String jid) throws  IllegalArgumentException,NumberFormatException {
		int prot = jid.indexOf    ("://")   ;
		int endLb=0 ; //This is the last char of LB address
		if (prot>0) endLb = jid.indexOf("/" , prot + 3) ;
		else endLb = jid.indexOf("/") ;
		if ( jid.indexOf(" ") != -1 )
			throw new IllegalArgumentException (jid+ ": wrong JobId format, blanks not allowed" ) ;
		if (endLb < 0) throw new IllegalArgumentException( jid + ": wrong JobId format, unable to find separator char '/' " ) ;
		setJobId(   new Url (jid.substring(0, endLb) )  /* LB */ ,    jid.substring(endLb+1) /* UNIQUE */ ) ;
	};
	/**
	* Set the JobId instance according to the LB server and the unique String passed as input parameters.
	* @param lbServer  the Logging and Bookkeeping server  address
	* @param   uniqueString a Unique identification */
	private synchronized void setJobId(Url lbServer , String uniqueString ) {
		//Update the members:
		lbAddress = lbServer   ;
		unique =  uniqueString ;
		jobId = lbAddress.getAddress() + "/" + unique  ;
		if ( lbAddress.getProtocol() == null )  jobId = LB_DEFAULT_PROTOCOL + jobId ;
	}
	/**
	* Set the JobId instance from the dg_jobId in String format given as input.
	* @param lbServer  An Url instance representing the LB address
	* @throws RunTimeException unable to create a JobId */
	public synchronized void  setJobId (Url lbServer) throws RuntimeException{
		String md5UniqueString = Api.generateId ( lbServer.getHost() , lbServer.getPort() ) ;
		setJobId (  lbServer , md5UniqueString );
	}
	/** Retrieve the address related to the JobId's LB server
	* @return the Url corresponding to the current jobid
	* @throws  NoSuchFieldException   If the jobId has not been initialised yet */
	public Url getServer( )throws  NoSuchFieldException{
		if (   isSet()   ) return lbAddress  ;
		else throw new NoSuchFieldException("JobId not set yet, unable to get LB address")  ;
	}
	/** @return the unique String into its String format
	* @throws  chFieldException  If the jobId has not been initialised yet */
	public String getUnique()throws  NoSuchFieldException {
		if (   isSet()   ) return unique ;
		else throw new NoSuchFieldException("JobId not set yet, unable to get unique string")  ;
	}
	// Private/Package Memebrs :
	String        jobId , unique;
	Url        lbAddress ;
	private static final String LB_DEFAULT_PROTOCOL = "https://" ;
};

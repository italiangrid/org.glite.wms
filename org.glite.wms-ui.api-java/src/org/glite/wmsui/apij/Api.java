/*
* Api.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;

import java.util.* ;
import  org.glite.wms.jdlj.Ad ;
import java.lang.RuntimeException ;
/**
 * The Api class provides static native implementation of common C/C++ operation not generally allowed by the JVM
 * It also internally implements the methods used RB-LB c++ classes
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class Api {
	/** Default constructor */
	public Api(){    /*initialise () ;*/  }

/***********************************************************************************
*                PUBLIC NATIVE METHODS:
************************************************************************************/
	/** Static native System Info Method: retrieve the current process pid */
	static public native int getPid() ;
	/** Static native System Method: Execute the command specified and returns its exit code
	@param command the executable command to be launched as a standard C process
	@return the integer value of the exit code returned by the launched process*/
	static public native int shadow(String command) ;
	/** Static native System Info Method: retrieve the environment specified variable
	*@param key the name of the variable to be retrieved
	*@return the integer corresponding to the id of the executed process*/
	static public native String getEnv (String key) ;
	/** Static native System Info Method: retrieve the user id number
	*@return the User identification number on the current system*/
	static public native int getUid () ;
	/** Static native System Info Method:  Set the specified Environment variable
	*@param key the name of the environment variable to set
	*@param value the value of the environment variable to be set */
	static public native void setEnv (String key , String value) ;
	/** Static native System Info Method:  Unset the specified Environment variable
	*@param key the name of the environment variable to unset*/
	static public native void unsetEnv (String key);
	// LB query
	Vector lbGetJobs (Url lbAddress ,  long from , long to , Ad userTags, int inex , String issuer) {
		result = new Vector() ;
		operation= USERJOBS ;
		lb_jobs ( lbAddress.getHost(), lbAddress.getPort() , (int)from , (int)to , (userTags==null)||(userTags.size()==0)?"":userTags.toString() , inex ,issuer) ;
		return (Vector) result ;
	}
	// LB User jobs value:
	Vector lbGetUserJobs( Url lbAddress ) throws RuntimeException {
		result = new Vector() ;
		operation= USERJOBS ;
		lb_user_jobs( lbAddress.getHost(), lbAddress.getPort() );
		return (Vector) result ;
	}
	/*    LB  Notification  wait for the first JobId that enter  one of  the specified states  */
	JobStatus lbNotification ( String jobids_states , int timeout ) {
		status = new JobStatus(  ) ;
		operation= STATUS ;
		lb_notify (jobids_states  , timeout ) ;
		return status ;
	}
	// RETRIEVE Jobs States... (TBCancelled)
	Vector lb_get_user_jobs_status (Url lbAddress) throws  RuntimeException {
		result =  new Vector() ;
		operation = USERJOBS ;
		lb_user_status( lbAddress.getHost(), lbAddress.getPort() );
		return (Vector) result ;
	}
	// LB Event value:
	Vector lb_get_log_info(String jobId) throws RuntimeException{
		result = new Vector() ;
		operation= LOGGING ;
		lb_log(jobId) ;
		return (Vector) result ;
	}
	// LB status Value:
	JobStatus lb_get_status(String jobId) throws RuntimeException{
		result = new Vector() ;
		operation = STATUS ;
		lb_status(jobId);
		return JobStatus.createStatus ( (Vector)result  , 0  )  ;
	};
	// NS multi attribute
	Vector ns_multi_attribute () throws RuntimeException{
		result = new Vector () ;
		operation = MULTIATTRS ;
		ns_multi();
		return (Vector)result ;
	} ;
	// NS getOutput files
	Vector ns_output_files (String jobid) throws RuntimeException{
		result = new Vector () ;
		operation = OUTPUTFILES ;
		ns_output( jobid );
		return (Vector)result ;
	} ;
	// RB  List Match Value:
	HashMap ns_list_match (  String jdl ) throws RuntimeException{
		result = new HashMap() ;
		operation = LISTMATCH ;
		ns_match ( jdl );
		return (HashMap)result ;
	}
	private void appendString(  int attrName , String attrValue) {
		if (attrValue !=null) switch ( operation ){
			case   STATUS:
				if (attrName== JobStatus.STATUS){
					status = new JobStatus(  ) ;
					((Vector)result).add( status ) ;
				}
				status.put ( attrName    ,    new String (attrValue)   ) ;
				break;
			case   LOGGING:
				if (attrName==Event.EVENT ){
					event = new Event () ;
					((Vector)result).add( event ) ;
				}
				event.put ( attrName    ,    new String (attrValue)   ) ;
				break;
			case USERSTATUS:
				if (attrName==JobStatus.STATUS){
					status = new JobStatus() ;
					((Vector)result).add( status ) ;
				}
				status.put ( attrName    ,    new String (attrValue)   ) ;
				break ;
			case USERJOBS:
				try{ ((Vector)result).add(new JobId(attrValue)) ;
				}catch (Exception exc) { 
				   /** DO NOTHING*/ 
                                }
				break ;
			case   LISTMATCH:
                                int index = attrValue.lastIndexOf("=");
                                if (index != -1) {
                                  ( (HashMap) result).put(attrValue.substring(0, index).trim(),
                                                          new Double(attrValue.substring(index + 1).
                                                                      trim()));
                                }
				break;
			case MULTIATTRS:
			case OUTPUTFILES:
				((Vector)result).add(new String (attrValue)) ;
				break;
			default: //nothing more so far
				break;
		} // end switch Operation
	};

	private void appendInt (  int attrName , int attrValue) {
		switch ( operation ){
		case   STATUS:
			status.put ( attrName   ,    new Integer(attrValue)   ) ;
			break;
		case LOGGING:
			event.put ( attrName    ,    new Integer( attrValue)   ) ;
			break;
		case USERSTATUS:
			status.put ( attrName    ,    new Integer( attrValue)   ) ;
			break ;
		default: // It is an intialisation
			switch (attrName){
			case 0:
				nsContext = attrValue ;
				break;
			case 1:
				lbContext = attrValue ;
				break;
			case 2:
				dagContext = attrValue ;
				break;
			default:
				throw new RuntimeException ("Unexpected initializing Context returned by JNI getCtx method: " + attrValue ) ;
			}
			break;
		}
	};
	/**
	@param value
	* true (LB)
	* false (NS)
	* @return the index of the required context context */
	private int getCtx ( int value){ ;
		switch (value){
		case 0:
			return nsContext ;
		case 1:
			return lbContext ;
		case 2:
			return dagContext ;
		default:
			throw new RuntimeException ("Unexpected code returned by JNI getCtx method: " + value ) ;
		}
	};

/***********************************************************************************
*                private/package NATIVE METHODS:
***********************************************************************************/
	native static void initialise () throws RuntimeException;
	native static String generateId( String lbHost , int lbPort ) throws RuntimeException;
	/**  native C++ NS Methods:   */
	synchronized native void ns_init (String nsAddress , int nsPort , int nsLoggerLevel)  throws RuntimeException;
	native void ns_submit (String jdl , String nsHost , int nsPort)  throws RuntimeException;
	native int ns_cancel ( String jobId )  throws RuntimeException;
	native String ns_get_root(String jobId)  throws RuntimeException;
	native void ns_purge(String jobId)  throws RuntimeException;
	native void ns_dgas (String jobid , String hlr ) throws RuntimeException;
	private native void ns_multi()  throws RuntimeException;
	private native void ns_output( String jobid  )  throws RuntimeException;
	private native void ns_match (String jdl )  throws RuntimeException;
	native void ns_free () ;

	/** native C LB Methods:  */
	native void lb_init ( String lb ) ;
	native void lb_register( String jobId , String jdl , String ns ) ;
	native void lb_logSync ( String state ) ;
	native void lb_log_output( String jobId );
	native void lb_log_start (String jdl , String Host , int nsPort);
	native void lb_free () ;
	native void lb_user_jobs ( String host , int port ) ;
	native void lb_jobs ( String host , int port  , int from , int to , String utags , int inex ,String issuer) ;
	native void lb_notify( String jobids_states , int timeout );
	native void lb_user_status (String host , int port ) ;
	native String lb_getSequence();
	native void lb_status(String jobId);
	private native void lb_log (String jobId);
	// Log Listener -> set the grid-console-shadow where to listen to
	native void lb_log_listener( String jobid , String host , int port  ) ;
	// Log quuery -> perform a  query over the database in order to retrieve State info
	native String lb_log_query( String jobid  , int step ) ;
	native void lb_log_tag ( String name , String value) ;
	/*************************************************
	*      Dag-Ad wrapper methods
	**************************************************/
	native void dagFromFile( String file );
	native String dagToString ( int level ) ;
	native void dag_logUserTags (String jobid ) ;
	native void dagSetAttribute ( int attr_name , String  attrValue ) ;
	native void registerDag ( String jobid , String nsAddress ) ;
	native void registerPart( String jobid , String original, String submission , String nsAddress , int res_number ) ;
	native void logDefaultValues ( boolean set);
	/***@deprecated */
	private native void dag_getSubmissionStrings () ;
	/** Members*/
	Object result ;
	Event event ;
	JobStatus status ;
	private int operation = 0 ;
	private static final int LOGGING = 1 ;
	private static final int STATUS = 2 ;
	private static final int LISTMATCH = 3 ;
	private static final int MULTIATTRS = 4 ;
	private static final int USERJOBS = 5 ;
	private static final int USERSTATUS = 6 ;
	private static final int OUTPUTFILES = 7 ;
	private int lbContext    ;
	private int nsContext   ;
	private int dagContext   ;
	/** Load native C++ Library */
	static  { System.loadLibrary("glite_wmsui_native"); }
};

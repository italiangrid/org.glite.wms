/*
* Result.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij ;
import java.util.* ;
/**
* The Result Class is used to store information of a required operation
* Depending on the operation type, the user will be able to check for possible error 
* or in case of success retrieve back the Job required information
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it> */
public class Result {
	Result ( String id , Object obj , int op , int i)   {
		jobId = id ;
		operation = op ;
		code = i ;
		result = obj ;
	}
        /** Convert the Result into a string*/
        public String toString() {
	   return toString ( 0 ) ;
	}
	/** Convert the Result into a string
	* if verbosity is 0 then basic info are returned.
	* if verbosity is 1 then medium info are returned.
	* if verbosity is 2 then all info are returned.
	@param verbosity depending on the Operation required, the verbosity prints additional information.
	@return the string representation of the current Result
	*/
	public String toString(int verbosity) {
		String toPrint = "" ;
		switch (operation){
		case SUBMIT:
			toPrint +="Submit" ;
			break;
			case STATUS:
			toPrint +="Status" ;
			break;
			case CANCEL:
			toPrint +="Cancel" ;
			break;
			case OUTPUT:
			toPrint +="Get-Output" ;
			break;
			case LISTMATCH:
			toPrint +="List Match" ;
			break;
			case LOGINFO:
			toPrint +="Logging Info" ;
			break;
			default:
			toPrint +="Unknown" ;
			break;
		}
		toPrint += " operation result: " ;
		// adding code info
		if (code<10)
			toPrint += "Success/Accepted" ;
		else if (code < 20)
			toPrint += "Failure" ;
		else if (code<30)
		toPrint += "Forbidden" ;
		else
		toPrint += "Generic error" ;
		// adding id info
		toPrint +=" " ;
		if ( jobId.equals("") )
		toPrint += "(jobid not created)" ;
		else
		toPrint += "(jobid = "+jobId +")" ;
		toPrint +="\n   " ;
		// adding operation info
		// adding optional result Info
		if (code<10)switch (operation){
			case SUBMIT:
			case CANCEL:
				break ;
			case LISTMATCH:
				HashMap res = (HashMap)result ;
				toPrint +="Matched resources:  " ;
				Iterator keys = res.keySet().iterator() ;
				Iterator entries = res.entrySet().iterator() ;
				while( keys.hasNext() ) {
					Object key = keys.next() ;
					toPrint+= "\n- " +  key + "     rank = " + res.get(key) ;
				}
				break;
			case OUTPUT:
				toPrint += "Output directory: " + (String) result ;
				break;
			case STATUS:
				JobStatus st = (JobStatus) result ;
				toPrint +="Status Info:\n";
				toPrint += ((JobStatus)result).toString( verbosity );
				break;
			case LOGINFO:
				Vector vect = (Vector) result ;
				toPrint +="-- Events Logged-- \n" ;
				for (int i = 0 ; i< vect.size() ; i++) toPrint +=((Event) vect.get(i)).toString(  verbosity ) + "\n   ---\n";
			default:
				break;
		}// end if (code<10), switch operation
		else{ //Exception caught
			if (result!=null) toPrint += "Error Reason: "+ ((Exception)result).getMessage() ;
			else toPrint += "Error Reason not available (null error message)" ;
		}
		return toPrint +"\n" ;
	}

	/** Retutn the code representig the required operation*/
	public int getCode() { return code ; }
	/** The Object returned from the operation depending on the operation itself
	* The value of the Object is:
	* <p>
	* SUBMIT        = null
	* <p>
	* CANCEL        = null
	* <p>
	* OUTPUT        = null
	* <p>
	* STATUS        = the JobStatus of the Job
	* <p>
	* LOGINFO       = the Vector of the job's Events
	* <p>
	* LISTMATCH  = an HashMap listing the Computing Elements*/
	public Object getResult() { return result ; }
	/** Retrun the Id representing the Job (empty string if the job hasn't been submitted)*/
	public String getId() { return jobId ; }
	/*
	* offset values to obtain failure starting from operation
	* es : SUBMIT_FAILURE = SUBMIT + FAILURE
	* es:   CANCEL_FORBIDDEN = CANCEL + FORBIDDEN*/
	static final int FAILURE   = 10   ;
	static final int FORBIDDEN   = 20   ;
	/** Submission Result value*/
	public static final int  SUBMIT         = 0 ;
	/** JobStatus info Result value*/
	public static final int  STATUS         = 1  ;
	/** Job Cancellation  Result value*/
	public static final int  CANCEL        = 3  ;
	/** Job Output files retrieval  Result value*/
	public static final int  OUTPUT         = 4  ;
	/** Events info  Result value*/
	public static final int  LOGINFO       = 5  ;
	/** Matching CE info Result value*/
	public static final int  LISTMATCH    = 6  ;

	/**  The requested operation has been completed successfully  */
	public static final int SUCCESS   =          0     ;
	/** The requested operation has been accepted                       */
	public static final int ACCEPTED   =         1   ;

	/**  API failed   = general RB Exc remapping                           */
	public static final int SUBMIT_FAILURE   = 10   ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int STATUS_FAILURE   =    11    ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int CANCEL_FAILURE   =   13    ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int GETOUTPUT_FAILURE   =   14   ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int LOGINFO_FAILURE = 15 ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int LISTMATCH_FAILURE   =   16   ;
	/**  API failed   = general RB Exc remapping                           */
	public static final int GENERIC_FAILURE   =   18    ;
	/**Cancel Method Result                                                       */
	public static final int CONDOR_FAILURE   =   19   ;

	/** When trying to submit a non-ad job  */
	public static final int SUBMIT_FORBIDDEN   =  20    ;
	/**When trying to retrieve status from a not submitted job          */
	public static final int STATUS_FORBIDDEN   =  21   ;
	/**When trying to cancel a not submitted job                           */
	public static final int CANCEL_FORBIDDEN   =  23    ;
	/**When trying to retrieve output from a not submitted job         */
	public static final int GETOUTPUT_FORBIDDEN   = 24    ;
	/**   When trying to retrieve logging info from a not submitted job                      */
	public static final int LOGINFO_FORBIDDEN = 25 ;
	/**   When trying to retrieve ListMatch from a non-ad job           */
	public static final int LISTMATCH_FORBIDDEN   =   26   ;
	/*************************************
	*   General Errors
	**************************************/
	/**JobNotDoneException                                                      */
	public static final int OUTPUT_NOT_READY   =    101   ;
	/**SandboxIOException                                                        */
	public static final int FILE_TRANSFER_ERROR   =   102    ;
	/**JobNotFoundException                                                     */
	public static final int JOB_NOT_FOUND   =   103    ;
	/**Cancel Method Result                                                       */
	public static final int MARKED_FOR_REMOVAL   =   104    ;
	/** Globus jobManager falied                                                */
	public static final int GLOBUS_JOBMANAGER_FAILURE   = 105    ;
	/** The Job Has been already completed                                */
	public static final int JOB_ALREADY_DONE   =  106  ;
	/** The Job Has aborted                                                      */
	public static final int JOB_ABORTED   =   107  ;
	/** The Job Has been removed                                              */
	public static final int JOB_CANCELLING   =  108  ;
	/** The User is not the owner of the job                                   */
	public static final int JOB_NOT_OWNER   =   109 ;
	/** submit skipped because the job has been already submitted */
	public static final int SUBMIT_SKIP   =  110    ;
	/** submit skipped because the job has been already submitted */
	public static final int OUTPUT_UNCOMPLETED   =  111    ;
	/** Time out Exception found*/
	public static final int TIMEOUT_REACHED   =  111    ;
	/**
	* Private members
	*/
	private int code ;
	private Object result = null;
	private String jobId = null ;
	private int operation ;
	private String       message ;



};

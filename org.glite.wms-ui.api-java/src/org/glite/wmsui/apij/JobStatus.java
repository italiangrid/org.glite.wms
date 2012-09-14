/*
* JobStatus.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
import java.util.* ;
import  org.glite.jdl.Ad ;
/**
 * LB JobStatus Class Wrap.
 * <p>This class represents the status of a job, retrieved from the LB server.
 * During its lifecycle a job can likely change the status, and almost all of its attributes.
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class JobStatus extends InfoLB{
	/**
	* Current status name.
	* @see #code()
	* @return the current status name string representation */
	public String name ()  { return   getValString(STATUS) ; }
	/** Default Constructor */
	public JobStatus ( ){
		super();
		indent = 0 ;
	} ;
	/**
	*Current status code as an int, see possible values:
	* @see #UNDEF
	* @see #SUBMITTED
	*@see #WAITING
	*@see #READY
	*@see #SCHEDULED
	*@see #RUNNING
	*@see #DONE
	*@see #CLEARED
	*@see #ABORTED
	*@see #CANCELLED
	*@see #UNKNOWN
	*@see #PURGED
	*@see #name
	*@return the current status int representation  */
	public int code (){ return   getValInt(STATUS_CODE) ; }

	/** String representation of all the status attributes
	@return all the attributes values represented as a String*/
	public String toString (){  return toString (DEFAULT_LOG_LEVEL) ;  }

	/** Retrieve all the sub jobs status information as vector of JobStatus instances
	@return  a Vector of JobStatus for a DAG. empty vector otherwise */
	public Vector getSubJobs ( ) { return subjobs ; };

	/**
	* Convert the JobStatus into a string
	* @return the string representation for the status attributes
	@param level of verbosity, can be 0 (minimal) 1(normal) or 2 (high) */
	public String toString (int level){
		String result  = "" ;
		// Indentation managing:
		String ind = "";
		for (int i= 0 ; i< indent ; i++ ) ind = "       " +ind;

		for (int i = 0 ; i< attNames.length ; i++ ) {
			Object obj =get(i);
			if ( obj!= null ){
				switch (i) {
					case JDL:
					case MATCHED_JDL:
					case USER_TAGS:
						if (level > 1 )
							try{ result += ind + attNames[i]  + " = " + new Ad (obj.toString()).toString(true , true)  + "\n" ; }
							catch (Exception exc) { System.out.println("Caught Exception!!!" + exc + obj);  }
						break;
					case STATUS:
					case REASON:
					case JOB_ID:
						result += ind + attNames[i]  + " = " + obj.toString() + "\n" ;
						break;
					default:
						if (level >0)
							result += ind + attNames[i]  + " = " + obj.toString() + "\n" ;
						break;
					}
			}
		}
		if ( subjobs.size() >0  ){
			result += "\n" + ind + "Nodes Information:\n";
			for ( int i = 0 ; i< subjobs.size() ; i++ )
				result +=  (   (JobStatus)subjobs.get(i)   ).toString(level ) ;
		}

		// Sub Jobs approach:
		return result ;
	}

	/** The String irepresentation of the attribute names */
	static final public String attNames[] = {"Acl", "cancelReason", "cancelling", "ce_node", "children", "children_hist", "children_num", "children_states",
		"condorId", "condor_jdl", "cpuTime", "destination", "done_code", "exit_code", "expectFrom", "expectUpdate", "globusId", "jdl", "jobId",
		"jobtype", "lastUpdateTime", "localId", "location", "matched_jdl", "network_server", "owner", "parent_job", "reason", "resubmitted", "rsl",
		"seed", "stateEnterTime", "stateEnterTimes", "subjob_failed", "user_tags", "Status", "status_code" };

	/** Static value that indicates a Dag*/
	static final public int JOBTYPE_DAG = 1 ;
	/** Static value that indicates a Job*/	
	static final public int JOBTYPE_JOB = 0  ;

	/**********************************************************************************
	* STATUS ATTRIBUTE INTEGER NAMES
	***********************************************************************************/
	/** JobStatus Attribute: Acl */
	static final public int ACL  = 0      ;
	/** JobStatus Attribute: Cancel Reason      */
	static final public int CANCEL_REASON  = 1      ;
	/** JobStatus Attribute: Cancellation request*/
	static final public int CANCELLING          = 2      ;
	/** Worker node where the job is executed */
	static final public int CE_NODE = 3;
	/** list of subjob IDs */
	static final public int CHILDREN =4 ;
	/** summary (histogram) of children job states */
	static final public int CHILDREN_HIST =5;
	/** number of subjobs */
	static final public int CHILDREN_NUM =6 ;
	/** full status information of the children */
	static final public int CHILDREN_STATES =7 ;
	/** Id within Condor-G */
	static final public int CONDOR_ID =8 ;
	/** ClassAd passed to Condor-G for last job execution */
	static final public int CONDOR_JDL = 9 ;
	/** Consumed CPU time */
	static final public int CPU_TIME               = 10     ;
	/** JobStatus Attribute: Ce Id*/
	static final public int DESTINATION         = 11      ;
	/** Return code */
	static final public int  DONE_CODE = 12 ;
	/** Unix exit code */
	static final public int EXIT_CODE = 13;
	/** Sources of the missing information */
	static final public int EXPECT_FROM      = 14      ;
	/** JobStatus Attribute: Logged information not arrived*/
	static final public int EXPECT_UPDATE   = 15      ;
	/** JobStatus Attribute: Globus allocated Id*/
	static final public int GLOBUS_ID            = 16      ;
	/** JobStatus Attribute: Job Description Language*/
	static final public int JDL                        =  17     ;
	/** JobStatus Attribute: JobId*/
	static final public int JOB_ID                    = 18    ;
	/** Type of job */
	static final public int  JOBTYPE= 19;
	/** JobStatus Attribute: Last known event of the job*/
	static final public int LAST_UPDATE_TIME = 20  ;
	/** JobStatus Attribute: Id within LRMS*/
	static final public int LOCAL_ID               = 21    ;
	/** JobStatus Attribute: Location*/
	static final public int LOCATION               = 22   ;
	/** Full job description after matchmaking */
	static final public int  MATCHED_JDL= 23;
	/** Network server handling the job */
	static final public int NETWORK_SERVER  = 24   ;
	/** JobStatus Attribute: Job owner*/
	static final public int OWNER                   = 25   ;
	/** parent job of subjob */
	static final public int  PARENT_JOB= 26;
	/** JobStatus Attribute: Status Reason*/
	static final public int REASON                 = 27    ;
	/** JobStatus Attribute: The job was resubmitted */
	static final public int  RESUBMITTED= 28 ;
	/** JobStatus Attribute: RSL to Globus*/
	static final public int RSL                         = 29   ;
	/** JobStatus Attribute: string used for generation of subjob IDs */
	static final public int  SEED= 30;
	/** JobStatus Attribute: Status enter time*/
	static final public int STATE_ENTER_TIME = 31   ;
	/** JobStatus Attribute: When all previous states were entered */
	static final public int  STATE_ENTER_TIMES= 32;
	/** JobStatus Attribute: Subjob failed (the parent job will fail too) */
	static final public int  SUBJOB_FAILED= 33;
	/** JobStatus Attribute: List of user-specified information tags */
	static final public int  USER_TAGS= 34;
	/** JobStatus Attribute: The Status of the job*/
	static final public int STATUS = 35;
	static final public int STATUS_CODE = 36;




	/**
	String representation of the attribute codes
	* @see #name()
	* @see #code() */
	static final public String code [] ={ "undefined", "Submitted", "Waiting", "Ready", "Scheduled",
		"Running", "Done", "Cleared", "Aborted", "Cancelled", "Unknown","Purged" } ;

	/**********************************************************************************
	* STATUS CODE INTEGER VALUES:
	***********************************************************************************/
	/**Return Status Code: indicates invalid, i.e. uninitialized instance */
	static final public int UNDEF        =     0    ;
	/**Return Status Code: Just submitted by user interface  */
	static final public int SUBMITTED =      1   ;
	/**Return Status Code: Accepted by WMS, waiting for resource allocation */
	static final public int WAITING      =      2   ;
	/**Return Status Code:   Matching resources found  */
	static final public int READY          =     3    ;
	/**Return Status Code: Accepted by LRMS queue  */
	static final public int SCHEDULED =       4  ;
	/**Return Status Code: Executable is running   */
	static final public int RUNNING      =        5 ;;
	/**Return Status Code: Execution finished, output not yet available  */
	static final public int DONE              =    6     ;
	/**Return Status Code:  Output transfered back to user and freed  */
	static final public int CLEARED =       7  ;
	/**Return Status Code:  Aborted by system (at any stage)   */
	static final public int ABORTED =       8  ;
	/**Return Status Code:  Cancelled by user  */
	static final public int CANCELLED =       9  ;
	/**Return Status Code:  Status cannot be determined  */
	static final public int UNKNOWN =      10   ;
	/**Return Status Code:  Status cannot be determined  */
	static final public int PURGED =      11   ;
	/**Return Status Code:  Status cannot be determined  */
	static final public int MAX_STATUS =      12   ;


	/** Static constructor.Used bu Api class */
	static JobStatus createStatus ( Vector vect , int indent  ) {
		if ( vect.size() ==0 )
			if (indent == 0 )
				throw new  java.lang.ArrayIndexOutOfBoundsException(  "Empy Job Status Returned"  );
			else
				return  null ;
		JobStatus status = (JobStatus) vect.remove(0) ;
		status.indent = indent ;
		for (int i = 0 ; i< status.getValInt( CHILDREN_NUM )  ; i++ )
			status.addSubJob ( createStatus ( vect , indent +1 )    );
		return status ;
	}

	/** Add a sub job status information **/
	private void addSubJob ( JobStatus status ) {
		if ( status!= null){
			subjobs.add(status);
		}
	};

	/* Sub Jobs implementation*/
	private Vector subjobs = new Vector() ;
	private int indent ;




} ;

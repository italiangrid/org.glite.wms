/*
* Event.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij ;
import java.util.* ;
import org.glite.jdl.Ad ;

/**
 * LB Event Class Wrapped.
 * <p>
 * During its life-cycle, the job logs several events to the logging server. Each of them is represented
 * by a series of couple attribute name, attribute value. This class is used in order to represent a single event
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class Event extends InfoLB {
	/** Retrieve the EVENT field
	* @return the string representation of the event
	* @see #code
	* @see #EVENT the attribute code to be retrieved  */
	public String name ()  {  return  getValString(EVENT) ; }
	/**
	* Retreive the EVENT_CODE field
	* @return the integer representation of the Event code
	* @see #EVENT_CODE the attribute to be retrieved
	* @see #CODE_UNDEF a possible returned value
	* @see #CODE_TRANSFER a possible returned value
	* @see #CODE_ACCEPTED a possible returned value
	* @see #CODE_REFUSED a possible returned value
	* @see #CODE_ENQUEUED a possible returned value
	* etc...   */
	public int code () { return getValInt ( EVENT_CODE ) ; }
	/**
	Retrieve the string representation of the Event.
	It is identical to toString(0)
	@return The string representation of  the basic event attributes
	@see #toString(int)   */
	public String toString ( ){  return toString ( 0 ) ; }

	/** Return a String representation of the Event
	* @return The string representation of  the event attributes. The number of the events taken into account depends of the value of the level parameter
	* @param level the verbosity level of toString representation as described in InfoLB class
	* @see InfoLB#DEFAULT_LOG_LEVEL basical information retrieved
	* @see InfoLB#NORMAL_LOG_LEVEL normal information retrieved
	* @see InfoLB#HIGH_LOG_LEVEL full-complete information retrieved
	*/
	public String toString( int level ){
		String result = new String() ;
		Object obj ;
		String app ;
		Iterator it = keySet().iterator() ;
		int tag ;
		while( it.hasNext()  ){
		tag = ((Integer)it.next()).intValue() ;
		obj = get(tag) ;
		if ( obj!= null ){
			app = obj.toString() ;
			if (!app.equals ("") ){
			switch (tag){
				case JOBID:
				case EVENT:
				case DESTINATION:
				case RESULT:
				case SOURCE:
				case TIMESTAMP:
					result += attrNames[ tag ]  + "  =  "  + app  + "\n";
					break;
				case JDL:
				case CLASSAD:
				case JOB:
					if (level>1)
					try{   result += attrNames[ tag ] + "  = " + new Ad(app).toString (true , true  ) + "\n" ;     } catch (Exception exc) {
					result += attrNames[ tag ] + "  = " + app  + "\n" ;
					// System.out.println ("\nWarning!!! Unable to parse into a classad: " + app ) ;
					}
					break;
				default:
					if (level >0)  result += attrNames[ tag ] + " = " + app  + "\n" ;
					break;
			} //end switch (tag)
			}
		}
		} // end while(it.hasNext() )
		return result ;
	}
	/** This array contains the possible Event names */
	static final public String code[] = {
		"Undefined",
		"Transfer",
		"Accepted",
		"Refused",
		"EnQueued",
		"DeQueued",
		"HelperCall",
		"HelperReturn",
		"Running",
		"Resubmission",
		"Done",
		"Cancel",
		"Abort",
		"Clear",
		"Purge",
		"Match",
		"Pending",
		"RegJob",
		"Chkpt",
		"Listener",
		"CurDescr",
		"UserTag",
		"Change Acl",
		"Notification"
	};
	/** This array contains the name of all the possible events attributes*/
	static final public String attrNames[] ={
		"classad",
		"descr",
		"dest_host",
		"dest_id",
		"dest_instance",
		"dest_jobid",
		"dest_port",
		"destination",
		"exit_code",
		"from",
		"from_host",
		"from_instance",
		"helper_name",
		"helper_params",
		"host",
		"jdl",
		"job",
		"jobId",
		"jobstat",
		"jobtype",
		"level",
		"local_jobid",
		"name",
		"node",
		"notified",
		"ns",
		"nsubjobs",
		"operation",
		"owner",
		"parent",
		"permission",
		"permission type",
		"priority",
		"queue",
		"reason",
		"result",
		"retval",
		"seed",
		"seqcode",
		"source",
		"src_instance",
		"src_role",
		"status_code",
		"svc_host",
		"svc_name",
		"svc_port",
		"tag",
		"timestamp",
		"user",
		"user Id",
		"User Id Type",
		"value",
		"Event",
		"event_code",
		"arrived"
	} ;
	/*****************************************************************************
	*               EVENT  CODES
	*******************************************************************************/
	/** Possible value returned by code() method. Undefined code. Should never happen*/
	static final public int CODE_UNDEF = 0;
	/** Possible value returned by code() method.  Start, success, or failure of job transfer to another component */
	static final public int CODE_TRANSFER= 1    ;
	/** Possible value returned by code() method.  Accepting job (successful couterpart to Transfer) */
	static final public int CODE_ACCEPTED= 2    ;
	/** Possible value returned by code() method.  Refusing job (unsuccessful couterpart to Transfer) */
	static final public int CODE_REFUSED=   3  ;
	/** Possible value returned by code() method.  The job has been enqueued in an inter-component queue */
	static final public int CODE_ENQUEUED=  4   ;
	/** Possible value returned by code() method.  The job has been dequeued from an inter-component queue */
	static final public int CODE_DEQUEUED=   5  ;
	/** Possible value returned by code() method.  Helper component is called */
	static final public int CODE_HELPERCALL= 6    ;
	/** Possible value returned by code() method.  Helper component is returning the control */
	static final public int CODE_HELPERRETURN= 7    ;
	/** Possible value returned by code() method.  Executable started */
	static final public int CODE_RUNNING=   8  ;
	/** Possible value returned by code() method.  Result of resubmission decision */
	static final public int CODE_RESUBMISSION=   9  ;
	/** Possible value returned by code() method.  Execution terminated (normally or abnormally) */
	static final public int CODE_DONE=    10 ;
	/** Possible value returned by code() method.  Cancel operation has been attempted on the job */
	static final public int CODE_CANCEL=  11   ;
	/** Possible value returned by code() method.  Job aborted by system */
	static final public int CODE_ABORT=   12  ;
	/** Possible value returned by code() method.  Job cleared, output sandbox removed */
	static final public int CODE_CLEAR=   13  ;
	/** Possible value returned by code() method.  Job is purged from bookkepping server */
	static final public int CODE_PURGE=   14  ;
	/** Possible value returned by code() method.  Matching CE found */
	static final public int CODE_MATCH=   15  ;
	/** Possible value returned by code() method.  No match found yet */
	static final public int CODE_PENDING=  16   ;
	/** Possible value returned by code() method.  New job registration */
	static final public int CODE_REGJOB=    17 ;
	/** Possible value returned by code() method.  Application-specific checkpoint record */
	static final public int CODE_CHKPT=    18 ;
	/** Possible value returned by code() method.  Listening network port for interactive control */
	static final public int CODE_LISTENER=  19   ;
	/** Possible value returned by code() method.  current state of job processing (optional event) */
	static final public int CODE_CURDESCR=  20   ;
	/** Possible value returned by code() method.  user tag -- arbitrary name=value pair */
	static final public int CODE_USERTAG=    21 ;
	static final public int CODE_TYPE_MAX = 22 ;
/*****************************************************************************
*               EVENT ATTRIBUTES
*******************************************************************************/
	/** Event attribute -  Chkpt: checkpoint value	 */
	static final public int CLASSAD         = 0         ;
	/** Event attribute -  CurDescr: description of current job transformation (output of helper)	 */
	static final public int DESCR        =     1        ;
	/** Event attribute -  Transfer: destination hostname	 */
	static final public int DEST_HOST        =     2        ;
	/** Event attribute -  Match: Id of the destination CE/queue	 */
	static final public int DEST_ID        =     3        ;
	/** Event attribute -  Transfer: destination instance	 */
	static final public int DEST_INSTANCE        =     4        ;
	/** Event attribute -  Transfer: destination internal jobid	 */
	static final public int DEST_JOBID        =     5        ;
	/*** Notification: destination port */
	static final public int DEST_PORT = 6 ;
	/** Event attribute -  Transfer: destination where the job is being transfered to	 */
	static final public int DESTINATION        =     7        ;
	/** Event attribute -  Done: process exit code	 */
	static final public int EXIT_CODE        =     8        ;
	/** Event attribute
	* <p>
	* Accepted: where was the job received from
	* <p>
	* Refused: where was the job received from */
	static final public int FROM        =     9        ;
	/** Event attribute
	* <p>Accepted: sending component hostname
	* <p> Refused: sending component hostname	 */
	static final public int FROM_HOST        =     10       ;
	/** Event attribute
	*<p> Accepted: sending component instance
	*<p> Refused: sending component instance*/
	static final public int FROM_INSTANCE        =     11        ;
	/** Event attribute
	* <p> HelperCall: name of the called component
	*<p> HelperReturn: name of the called component	 */
	static final public int HELPER_NAME        =     12        ;
	/** Event attribute -  HelperCall: parameters of the call	 */
	static final public int HELPER_PARAMS        =     13        ;
	/** Event attribute -  common: hostname of the machine where the event was generated	 */
	static final public int HOST        =     14        ;
	/** Event attribute -  RegJob: job description	 */
	static final public int JDL        =     15        ;
	/** Event attribute
	* <p>  EnQueued: job description in receiver language
	*<p> Transfer: job description in receiver language	 */
	static final public int JOB        =     16        ;
	/** Event attribute -  common: DataGrid job id of the source job 	 */
	static final public int JOBID        =    17         ;
	/** * Notification: job status */
	static final public int JOBSTAT = 18 ;
	/** Event attribute -  common: DataGrid job id of the source job 	 */
	static final public int JOTYPE        =    19         ;
	/** Event attribute -  common: logging level (system, debug, ...)	 */
	static final public int LEVEL        =     20        ;
	/** Event attribute
	* <p> Accepted: new jobId (Condor, Globus ...) assigned by the receiving component.
	* <p> DeQueued: new jobId assigned by the receiving component. */
	static final public int LOCAL_JOBID        =     21       ;
	/** Event attribute -  UserTag: tag name	 */
	static final public int NAME        =     22        ;
	/** Event attribute -  Running: worker node where the executable is run	 */
	static final public int NODE        =     23       ;
	/**  * Notification: notification id */
	static final public int NOTIFID = 24 ;
	/** Event attribute -  RegJob: NetworkServer handling the job	 */
	static final public int NS        =     25        ;
	/** Event attribute -  RegJob: number of subjobs	 */
	static final public int NSUBJOBS        =     26        ;
	/** ChangeACL: operation requested to perform with ACL (add, remove) */
	static final public int OPERATION        =     27        ;
	/**  Notification: owner */
	static final public int OWNER = 28 ;
	/** Event attribute -  RegJob: jobid of parent job	 */
	static final public int PARENT        =     29        ;
	/** * ChangeACL: ACL permission to change (currently only READ) */
	static final public int PERMISSION =     30      ;
	/**  ChangeACL: type of permission requested ('allow', 'deny') */
	static final public int PERMISSION_TYPE =     31       ;
	/** Event attribute -  common: message priority (yet 0 for asynchronous and 1 for synchronous transfers)	 */
	static final public int PRIORITY        =    32         ;
	/** Event attribute -  DeQueued: queue name	 * EnQueued: destination queue	 */
	static final public int QUEUE        =     33        ;
	/** Event attribute -  Abort: reason of abort
	* <p>Cancel: detailed description
	* <p> Clear: why the job was cleared
	* <p>Done: reason for the change
	* <p>EnQueued: detailed description of transfer, especially reason of failure
	* <p>Pending: why matching CE cannot be found
	* <p>Refused: reason of refusal
	* <p>Resubmission: reason for the decision
	* <p>Transfer: detailed description of transfer, especially reason of failure */
	static final public int REASON        =     34        ;
	/** Event attribute -
	* <p> EnQueued: result of the attempt
	* <p> Resubmission: result code
	* <p> Transfer: result of the attempt	 */
	static final public int RESULT        =    35        ;
	/** Event attribute -  HelperReturn: returned data	 */
	static final public int RETVAL        =     36       ;
	/** Event attribute -  RegJob: seed for subjob id generation	 */
	static final public int SEED        =     37        ;
	/** Event attribute -  common: sequence code assigned to the event	 */
	static final public int SEQCODE        =     38        ;
	/** Event attribute -  common: source (WMS component) which generated this event	 */
	static final public int SOURCE        =     39        ;
	/** Event attribute -  common: instance of WMS component (e.g. service communication endpoint)	 */
	static final public int SRC_INSTANCE        =     40        ;
	/** Event attribute
	* <p> HelperCall: whether the logging component is called or calling one
	* <p>HelperReturn: whether the logging component is called or calling one	 */
	static final public int SRC_ROLE        =     41        ;
	/** Event attribute -  Cancel: classification of the cancel	 * Done: way of termination	 */
	static final public int STATUS_CODE        =     42       ;
	/** Event attribute -  Listener: hostname	 */
	static final public int SVC_HOST        =     43       ;
	/** Event attribute -  Listener: port instance name	 */
	static final public int SVC_NAME        =     44        ;
	/** Event attribute -  Listener: port number	 */
	static final public int SVC_PORT        =     45        ;
	/** Event attribute
	* <p> Chkpt: checkpoint tag
	* <p> Resubmission: value of the attribute on which the decision is based	 */
	static final public int TAG        =     46      ;
	/** Event attribute -  common: timestamp of event generation	 */
	static final public int TIMESTAMP        =     47        ;
	/** Event attribute -  common: identity (cert. subj.) of the generator	 */
	static final public int USER        =     48        ;
	/** ChangeACL: DN or VOMS parameter (in format VO:group) */
	static final public int USER_ID =     49       ;
	/**  ChangeACL: type of information given in user_id (DN or VOMS)*/
	static final public int USER_ID_TYPE =     50        ;
	/** Event attribute -  UserTag: tag value	 */
	static final public int VALUE        =     51        ;
	/* time stamp arrived event */
	static final public int ARRIVED = 54        ;
	// This field cannot be removed/modified without modifying the native implementation
	/** The Event string name attribute */
	static final public int EVENT        =     52        ;
	/** The Event code integer attribute */
	static final public int EVENT_CODE        =     53        ;
};

/*
* Job.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*/
package org.glite.wmsui.apij;
import  org.glite.wms.jdlj.JobAd ;
import  org.glite.wms.jdlj.JobAdException ;
import  org.glite.wms.jdlj.JobState ;
import  org.glite.wms.jdlj.Ad ;
import  org.glite.wms.jdlj.Jdl ;
import org.globus.io.urlcopy.UrlCopy ;
import org.globus.util.* ;   // GlobusUrl class
import org.globus.gsi.GlobusCredentialException;
import condor.classad.* ;
import java.util.* ;
import  java.io.* ;

/**
 * Allow controlling the job and perform several operations.
 * <p>
 * The Job class provides methods that allow controlling the job during its lifetime.
 * The allowed operations are:
 * <ul>
 * <li> Submitting the job to a network server
 * <li> Check NS for possible matching Computer Elements
 * <li> Cancelling the job during its life-cycle
 * <li> Retrieving status information from LB server
 * <li> Retrieving Events logging information from LB server
 * <li> Retrieving the output sandbox files
 * </ul>
 Also some special features are provided:
 * <ul>
 * <li> Attaching an interactive job to a shadow listener
 * <li> Retrieving a submitted State information from a checkpointable Job
 * <li> Submitting a checkpointable Job starting from a specified State
 * </ul>
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>  */
public class Job  {
	/** Instantiates an  empty  Job object */
	public Job()   {
		jobType = JOB_NONE ;
	} ;
	/** Instantiates an  Job object with a JobId
	* @param id  the JobId instance from which the Job has to be created
	* @throws IllegalArgumentException If the JobId is not valid */
	public Job( JobId id) throws IllegalArgumentException  {  setJobId(id) ; }  ;
	/** Instantiates an  Job object with a JobAd
	* @param ad  the JobAd instance from which the Job has to be created
	* @throws IllegalArgumentException If the JobAd is not valid */
	public Job( JobAd ad)throws IllegalArgumentException  { setJobAd(ad) ;  };
	/** Instantiates an  Job object with a JobAd
	* @param dagFile  the dagad file instance from which the Dag has to be created
	* @throws IllegalArgumentException If the DagAd is not valid */
	public Job( String dagFile)throws IllegalArgumentException  {
		setDagAd(dagFile) ;
	};
	/** Create a copy of the Job, including its private JobId and JobAd members information
	@return the copy of the Job instance*/
	public Job copy (){
		Job job ;
		try{
			switch (jobType){
				case JOB_ID:
					job = new Job( jid ) ;
					break;
				case JOB_SUBMITTED:
					job = new Job( jid ) ;
					job.setJobAd(jad) ;
					break;
				case JOB_AD:
					job = new Job( jad ) ;
					break;
				case DAG_AD:
					job = new Job ( dagad) ;
					break;
				default:
					job = new Job() ;
					break;
			} // end Switch
		}catch (Exception exc){
			job = new Job() ;
		}
		job.jCollect = jCollect ;
		return job ;
	}
	/** Check if two Job objects are the same, i.e have the same JobId
	* @param job the Job to check with the current instance
	* @return true if the jobs are equal, false otherwise*/
	public boolean equals (Job job) {
		switch (jobType){
			case JOB_ID:
			case JOB_SUBMITTED:
				if (  jid.equals( job.jid)  )
				return true ;
				break;
			case JOB_AD:
				if (  jad.equals( job.jad)  )
				return true ;
				break;
			case DAG_AD:
				if (dagad.equals( job.dagad) )
				return true ;
				break;
			case JOB_NONE:
			default:
				return false ;
		}
		return false ;
	}
	/**
	* Get the JobId instance
	* @return a pointer to the internal JobId intance
	* @throws NoSuchFieldException  If the JobId instance  is empty */
	public JobId getJobId() throws NoSuchFieldException {
		if ( jid!=null )  if ( jid.isSet() )  return jid ;
		throw new  NoSuchFieldException ("the JobId instance  is empty");
	}  ;
	/**
	* Get the JobAd instance
	* @return a pointer to the internal JobAd intance
	* @throws NoSuchFieldException  If the JobAd instance is empty */
	public JobAd getJobAd() throws  NoSuchFieldException {
		if ( jad!=null )  if (jad.isSet()) return jad ;
		throw new  NoSuchFieldException ("the JobAd instance  is empty");
	}  ;

	/** Set the NS verbosity level
	@param level minimum (no verbosity) level = 0. Maximum (full verbosity) = 6
	*/
	public void setLoggerLevel ( int level ) {  nsLoggerLevel = level ;  } ;
	/** Decide whether to log or not some UI default values such as:
	* UI node name information in UserTags attribute
	@param set if set to true the UI will automatically log its default values, otherwise not
	*/
	public void logDefaultValues ( boolean set){  uiLogDefault  = set ; };

	/**
	* Set a different  Proxy certificate from the default one and check it
	* @param cp The full path of the proxy certificate file to be set
	* @throws  FileNotFoundException Unable to find-load-read the specified file
	* @throws GlobusCredentialException Unable to Load the specified proxy certificate  */
	public void setCredPath( File cp)  throws  GlobusCredentialException, FileNotFoundException {
		if (userCred== null)  userCred = new UserCredential ( cp ) ;
		else   userCred.setProxy(cp);
	}  ;
	/**Set the Proxy certificate as default
	* @throws FileNotFoundException Unable to find-load-read the deafult proxy file
	* @throws GlobusCredentialException Unable to Load the default proxy certificate  */
	public void unsetCredPath() throws GlobusCredentialException  ,  FileNotFoundException {
		if (userCred== null)     userCred = new UserCredential ( ) ;
		else  userCred.unsetProxy();
	}  ;
	/** set  the JobAd instance
	* @param ad  the JobAd Instance to set from
	* @throws IllegalArgumentException   The JobAd instance has been already set  */
	public void setJobAd( JobAd ad)throws  IllegalArgumentException  {
		if (jad !=null){
			if ( jad.isSet()  ) throw new  IllegalArgumentException( "JobAd instance already set" ) ;
		}else {
			jad = (JobAd) (ad.clone() );
			jobType = JOB_AD;
			jCollect = false ;
		}
	}  ;
	/** set  the Dagad instance
	* @param ad  the Dagad Instance to set from
	* @throws IllegalArgumentException   The JobAd instance has been already set  */
	public void setDagAd( String ad)throws  IllegalArgumentException  {
		if (dagad !=null)
			throw new  IllegalArgumentException( "DagAd instance already set" ) ;
		else
			dagad = new String ( ad ) ;
		jobType = DAG_AD;
	}  ;
	/** set  the JobId instance
	* @param id  the JobId Instance to set from
	* @throws  IllegalArgumentException  If the JobId has been alreaty set
	*/
	public void setJobId( JobId id) throws  IllegalArgumentException {
		if (jid !=null){
			if (  jid.isSet()  ) throw new  IllegalArgumentException  ( "JobId instance already set" ) ;
		}else {
			jid = id ;
			jobType = JOB_ID;
			jCollect = false ;
		}
	}  ;
	/** Set the JobAd member attribute of the Job instance to the job description got from the LB
	* @return a pointer to the internal JobAd intance, which has been just loaded from the retrieved LB info
	* @throws JobAdException when the Jdl has errors
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public JobAd retrieveJobAd() throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		//Check if the JobId has been already set
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			// Operation not allowed
			throw new UnsupportedOperationException ("retrieveJobAd not allowed" ) ;
		// Does not Check The status code but load Jdl
		// 		  (lbApi,  nsApi,  Jdl, Finished ,  ignore)
		checkStatus (  false , false,  true,  false , true ) ;
		return jad ;
	};
	/**************************************************************************
	*      JOB      OPERATION: LB
	**************************************************************************/

	/**  The same as getStatus( true)
	* @see #getStatus(boolean)
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity
	*/
	public Result getStatus( ) throws  UnsupportedOperationException ,
		java.io.FileNotFoundException , org.globus.gsi.GlobusCredentialException
	{  return getStatus ( true) ;  }

	/**
	* Retrieve the status of the job
	* @param ad determine whether to download (true) or not (false)  all JobAd information from server
	* @return the status of the job
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity */
	public Result getStatus(  boolean ad   ) throws  UnsupportedOperationException , org.globus.gsi.GlobusCredentialException,
		java.io.FileNotFoundException
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			throw new UnsupportedOperationException ("getStatus: operation not allowed");
		if (api==null)   api = new Api() ;
		if (! jCollect){
			api.initialise() ;
			if (userCred== null) userCred = new UserCredential () ;
			userCred.checkProxy() ;
		}

		return new Result ( jid.toString() , api.lb_get_status(  jid.toString()  ) , Result.STATUS , Result.SUCCESS );
	};
	/**
	* Retrieves the status of the Job once it enters the specified status code (s)
	* @param states an array of JobStatus status codes
	* @param timeout seconds the method has to wait for notification. After that period the methods returns anyway. If 0 specified it hangs untill notify arrives
	* @return a Result containing the JobStatus (or empty when timeout reached)
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity
	* @see Result#TIMEOUT_REACHED
	* */
	public Result notify (  int [] states  , int timeout ) throws  UnsupportedOperationException ,
		java.io.FileNotFoundException , org.globus.gsi.GlobusCredentialException
	{
		if ((jobType == JOB_NONE) || (jobType == JOB_AD))
			throw new UnsupportedOperationException ("notify: operation not allowed");
		// Initialise context and check permission
		if (api == null) api = new Api();
		api.initialise();
		if (userCred == null) userCred = new UserCredential();
		userCred.checkProxy();
		Ad jobStates = new Ad () ;
		try{
			jobStates.setAttribute ("JOBIDS" , jid.toString() );
			for (int i =0 ; i< states.length; i++ ){
				if(   ( states[i]< JobStatus.SUBMITTED) ||  ( states[i] > JobStatus.PURGED)   )
					throw new UnsupportedOperationException ("Unable to notify status code out of limits: "+ states[i]);
					jobStates.addAttribute ("STATES" , states[i] );
			}
		} catch ( javax.naming.directory.InvalidAttributeValueException exc ) {
			throw new UnsupportedOperationException ("notify: Fatal Error"); /* never happens */
		}
		return new Result ( jid.toString() ,  api.lbNotification ( jobStates.toString() , timeout) ,Result.STATUS , Result.SUCCESS ) ;
	}


	/**
	* Retrieves the Job Id of all sub jobs of the job.
	* @return a Vector containing a String representation of all sub jobs JobId.
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity
	* */
	public Vector getSubJobsId() throws  UnsupportedOperationException ,
	org.globus.gsi.GlobusCredentialException, java.io.FileNotFoundException {
		if ((jobType == JOB_NONE) || (jobType == JOB_AD))
			throw new UnsupportedOperationException ("getSubJobsId: operation not allowed");
		if (api == null) api = new Api();
		if (!jCollect) {
			api.initialise();
			if (userCred == null) userCred = new UserCredential();
			userCred.checkProxy();
		}
		Vector returnVector = new Vector();
		Vector subJobStatusVector = api.lb_get_status(jid.toString()).getSubJobs();

		for (int i = 0; i < subJobStatusVector.size(); i++){
			returnVector.add(     (         (JobStatus) subJobStatusVector.get(i)          ).getValString(JobStatus.JOB_ID)          );
		}
		return returnVector;
	}

	/**
	* Retrieve the bookkeeping information of the job ( syncronous version )
	* @return all the events logged during the job life
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity
	* @throws UnsatisfiedLinkError  Unable to fine the native shared library that implements LB-RB functions*/
	public Result getLogInfo() throws  UnsupportedOperationException ,
		java.io.FileNotFoundException , org.globus.gsi.GlobusCredentialException
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			throw new UnsupportedOperationException ("getLogInfo: operation not allowed") ;
		if (api==null)  api = new Api() ;
		if (! jCollect){
			api.initialise() ;
			if (userCred== null) userCred = new UserCredential () ;
			userCred.checkProxy() ;
		}
		return new Result ( jid.toString() ,  api.lb_get_log_info(   jid.toString() ) ,Result.LOGINFO,Result.SUCCESS) ;
	};
	/**************************************************************************
	*      JOB      OPERATION: SPECIAL (attach, getState)
	**************************************************************************/
	/**
	* Attach the Job with the specified implemented listener
	* <p>
	* The port where to listen to is automatically determined: if possible it at appends to the first logged port
	* This method is equal to attach (Listener , 0 )
	* @param ls the implementation of the listener
	* @see #attach(Listener, int)
	*/
	public void attach ( Listener ls ) throws   Exception
	{  attach (ls , 0 ) ; }
	/**
	* Attach the Job with the spècified implemented listener
	* @param ls an implementation of the Listener interface
	* @param port the interactive port where the console listener listens to
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws java.net.UnknownHostException If unable to determine the current system operator
	* @throws NoSuchFieldException If any required attribute is missing from the JDL
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public void attach ( Listener ls , int port) throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException, // Proxy
		java.net.UnknownHostException  // Shadow
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			throw new UnsupportedOperationException ("Attachement: operation not allowed") ;
		// Check The status code, perform lb init and load Jdl
		// 		  (lbApi,  nsApi,  Jdl, Finished ,  ignore)
		checkStatus (  true,  false,  true,  false , false ) ;
		if (  !jad.hasAttribute(Jdl.JOBTYPE,  Jdl.JOBTYPE_INTERACTIVE ) )
			throw new UnsupportedOperationException ("Unable to attach: the job is not interactive" );
		if (port ==0 ){
			// No port specified: try to retrieve the port from Server Logged
			String hostandport = api.lb_log_query (jid.toString() , -1) ;
			int ind = hostandport.indexOf (":") ;
			if (ind !=-1){
				try{
					port  =  java.lang.Integer.parseInt( hostandport.substring( ind +1 )  );
				}catch (java.lang.NumberFormatException exc) { 
				  /** do nothing. Port is zero*/  
				}
			}
		}
		// Start the interactive session
		shadow = new Shadow (  jid , ls) ;
		shadow.console(  port );
		api.lb_log_listener(  jid.toString() , shadow.getHost() , shadow.port ) ;
		shadow.start ( ) ;
	}
	/**
	* Retrieve the specified checkpointable step from the LB server
	* @param step In order to retrieve the last logged checkpointable event, step must be 0. step must be 1 for the last-but-one etc etc
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	* @throws java.text.ParseException Unable to parse the JobState string retrieved from LB
	* @throws JobAdException the Jdl has semantic errors
	*/
	public JobState getState(int step) throws  UnsupportedOperationException ,
		java.io.FileNotFoundException,
		org.globus.gsi.GlobusCredentialException ,
		JobAdException,
		java.text.ParseException
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			// JobId not set yet
			throw new UnsupportedOperationException ("getState not allowed" ) ;
		// Does not Check The status code but perform lb init and load Jdl
		// 		  (lbApi,  nsApi,  Jdl, Finished ,  ignore)
		checkStatus (  true,  false,  true,  false , true ) ;
		if (  !jad.hasAttribute(Jdl.JOBTYPE,  Jdl.JOBTYPE_CHECKPOINTABLE  )  )
			throw new UnsupportedOperationException ("Unable to Retrieve Job State: the job is not checkpointable" );
		return new JobState ( api.lb_log_query ( jid.toString() , step   )   ) ;
	}

	/**************************************************************************
	*      JOB      OPERATION: NS
	**************************************************************************/
	/**
	* Submit the job to the Network Server
	* This method is equal to submit (ns, lb, ceId, null, null)
	* @see #submit(Url , Url , String , Listener, JobState) Perfroms extra Interactive/checkpointable job activities
	* @param  ns  The Network Server  address
	* @param  lb  The Logging and bookkeeping address
	* @param  ceId The Computing Element Identificator where to perform the job
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws InvalidAttributeValueException Unable to set values inside the JobAd instance
	* @throws java.net.UnknownHostException If unable to determine the current system operator
	* @throws JobAdException when the Jdl has semantic errors
	* @throws NoSuchFieldException If any required attribute is missing from the JobAd
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public Result submit( Url ns,  Url lb , String ceId ) throws
		javax.naming.directory.InvalidAttributeValueException, // JobAd addVal , getVal
		java.net.UnknownHostException ,  // Shadow
		JobAdException, // checkAll
		NoSuchFieldException, // each JobAd attribute
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		if (jobType == JOB_AD)
			// Create the first State step, if needed:
			if(      ( jad.hasAttribute (Jdl.JOBTYPE , Jdl.JOBTYPE_CHECKPOINTABLE)    )   &&   ( ! jad.hasAttribute( Jdl.CHKPT_JOBSTATE ) )    ){
				JobState state =  new JobState() ;
				if (jad.hasAttribute (Jdl.CHKPT_STEPS))
					switch (jad.getType(Jdl.CHKPT_STEPS) ){
						case Ad.TYPE_STRING:
							state.setAttribute (Jdl.CHKPT_STEPS , jad.lookup(Jdl.CHKPT_STEPS).toString() ) ;
						default:
							state.setAttribute (Jdl.CHKPT_STEPS , ((Integer) jad.getIntValue( Jdl.CHKPT_STEPS ).get(0)).intValue() ) ;
					}
				if (jad.hasAttribute (Jdl.CHKPT_CURRENTSTEP))
					state.setAttribute (Jdl.CHKPT_CURRENTSTEP ,  jad.lookup(Jdl.CHKPT_CURRENTSTEP).toString() ) ;
				else
					state.setAttribute (Jdl.CHKPT_CURRENTSTEP , "1" ) ;
				state.setAttribute (Jdl.CHKPT_DATA , new Ad () ) ;
				return submit (ns , lb ,  ceId , null, state ) ;
			}
		return submit (ns , lb, ceId , null, null ) ;
	}

	/**
	* Submit the job to theNetworkServer. Optional parameter for interactive and checkpointable jobs
	* @param  ns  The Network Server  address
	* @param  lb  The Logging and bookkeeping address
	* @param  ceId  The Computing Element Identificator where to force the job submission
	* @param ls an instance implementation of Listener interface.When the submission is finished, the Job will launch the Listener.run method
	* @param state A specific JobState step of the job (which must be of checkpointing jobtype). The job will take on the submission starting from the specified step
	* @see Listener#run( Shadow)
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws InvalidAttributeValueException Unable to set values inside the JobAd instance
	* @throws java.net.UnknownHostException If unable to determine the current system operator
	* @throws JobAdException when the Jdl has semantic errors
	* @throws NoSuchFieldException If any required attribute is missing from the JobAd
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public Result submit( Url ns,  Url lb , String ceId , Listener ls , JobState state) throws
		javax.naming.directory.InvalidAttributeValueException, // JobAd addVal , getVal
		java.net.UnknownHostException ,  // Shadow
		JobAdException, // checkAll
		NoSuchFieldException, // each JobAd attribute
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		if   (   (jobType != JOB_AD) &&(jobType != DAG_AD)   )
			throw new UnsupportedOperationException ("submit Operation not allowed" );
		if (     (jobType == JOB_AD)&& (ceId!=null) )
			// Insert the Ce Id into the JobAd
			jad.setAttribute(Jdl.CEID , ceId  );
		nsInit   (ns) ;
		nsAddress = ns ;
		lbAddress = lb ;
		jid = new JobId( ) ;
		jid.setJobId( lbAddress );
		if    (jobType == DAG_AD){
			// DAG
			api.dagFromFile (dagad) ;
			api.dagSetAttribute (  0 , jid.toString() );
			api.logDefaultValues ( uiLogDefault ) ;
		} else{
			// JOB
			jad.setAttribute(Jdl.JOBID , jid.toString() );
			if ( jad.hasAttribute(Jdl.JOBTYPE,  Jdl.JOBTYPE_INTERACTIVE )  ) shadow = new Shadow (  jid , ls) ;
			if (ls !=null){
				// INTERACTIVE
				if (  !jad.hasAttribute(Jdl.JOBTYPE,  Jdl.JOBTYPE_INTERACTIVE ) )
					throw new UnsupportedOperationException ("Interactive console requested for a non interactive job");
				shadow = new Shadow (  jid , ls) ;
			}
			// CHECKPOINTABLE
			if (state !=null){
				if ( !jad.hasAttribute (Jdl.JOBTYPE , Jdl.JOBTYPE_CHECKPOINTABLE) )
					throw new UnsupportedOperationException ("Checkpointable State requested for a non Checkpointable job");
				state.setAttribute (JobState.JOBID , jid.toString() ) ;
				state.check();
				jad.setAttribute( Jdl.CHKPT_JOBSTATE ,  state );
			}
		}
		nsSubmit (  nsAddress ,  lbAddress );
		return new Result( jid.toString () , jid, Result.SUBMIT,  Result.ACCEPTED  ) ;
	}

	/**
	* TO BE DONE  TBD
	* Submits and get the output of a simple Job once it's ready.
	* once the job has been submitted, a cycle of status retrieval is done untill
	* the status code is reached. Then the output files are retrieved and stored in the specified output directory
	* @return  The JobId pointer to the submitted job
	* @param  ns  The Network Server  address
	* @param  lb  The Logging and bookkeeping address
	* @param  executable The value of the Executable JDL attribute
	* @param  stdOutput  The value of the standard Output JDL attribute
	* @param stdErr  The value of the standard Error JDL attribute
	* @param  ce_id The Computing Element Identificator where to perform the jo
	* @param outputDir the directory where to retrieve the output files from the job once it is ready
	* @param timeout lentgh of status cycle retrieval. Each 30 seconds a getStatus is called for timeout times
	* @ throws InvalidAttributeValueException  - unable to create a valid classd
	static Result submit(Url ns , Url lb ,
		String executable ,String stdOutput ,String stdErr,String vo ,
		String outputDir,String ce_id ,
		int timeout ,
		int time_interval) throws javax.naming.directory.InvalidAttributeValueException
		{
			JobAd ad = new JobAd () ;
			ad.setAttribute ( Jdl.EXECUTABLE , executable ) ;
			ad.setAttribute ( Jdl.STDOUTPUT , stdOutput ) ;
			ad.setAttribute ( Jdl.STDERROR ,  stdErr ) ;
			ad.addAttribute ( Jdl.OUTPUTSB ,  stdErr ) ;
			ad.addAttribute ( Jdl.OUTPUTSB ,  stdOutput ) ;
			// MANDATORY attribute Check:
			//ad.setAttributeExpr( Jdl.RANK , JDL_RANK_DEFAULT ) ;
			//ad.setAttributeExpr( Jdl.REQUIREMENTS , JDL_REQ_DEFAULT  ) ;
			return null ;
		}
	*/

	/**Look for matching Computing Element available resources ( syncronous version )
	* @param  nsAddr = The Resource Broker full address given in the form  <RB host>:<RB port>
	* @return all the Computing Elemets that match with the given JobAd
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws JobAdException when the Jdl has semantic errors
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public Result listMatchingCE( Url nsAddr )throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		if   (   jobType == JOB_NONE )   throw new UnsupportedOperationException ("listMatchingCE: operation not allowed" );
		nsInit   (nsAddr) ;
		Vector vect = api.ns_multi_attribute();
		if ( (vect!=null)  && vect.size()>0 ){
			nsMultiAttribute ( Jdl.RANK, vect ) ;
			nsMultiAttribute ( Jdl.REQUIREMENTS , vect  );
		}
		HashMap matched = api.ns_list_match ( jad.toSubmissionString() ) ;
		int res = (matched.size()>0)?  Result.SUCCESS :  Result.LISTMATCH_FAILURE ;
		// api.ns_free();
		if (matched.size()>0) return new Result( "<null>" , matched ,  Result.LISTMATCH , res  )   ;
		else  return new Result( "<null>" , new UnsupportedOperationException ("Unable to find any matching CE available" )  ,  Result.LISTMATCH , res  ) ;
	};


	/**Cancel the job from the resource broker ( syncronous version )
	* @return  The Result of the operation
	* @throws JobAdException when the Jdl has semantic errors
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public Result cancel ( ) throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			// Operation not allowed
			throw new UnsupportedOperationException ("cancel: operation not allowed" );
		// Check The status code and perform ns init
		// 		  (lbApi,  nsApi,  Jdl, Finished ,  ignore)
		checkStatus (  false , true , false,  false , false ) ;
		api.ns_cancel (  jid.toString() ) ;
		return new Result( jid.toString () , null , Result.CANCEL,  Result.ACCEPTED  )   ;
	};

	/**Retrieve output files of a submitted job ( syncronous version )
	* @param dirPath  the path where to retrieve the OutputSandbox files
	* @return  The Result of the operation
	* @throws JobAdException when the Jdl has semantic errors
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException unable to find any certificate file
	* @throws org.globus.gsi.GlobusCredentialException proxy certificate information failed
	*/
	public Result getOutput( String dirPath)  throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		if   (   jobType == JOB_NONE ||  jobType == JOB_AD  )
			// Operation not allowed
			throw new UnsupportedOperationException ("get_Output: operation not allowed" );
		// Check The status code, perform ns init and load Jdl
		// 						(lbApi,  nsApi,  Jdl, Finished ,  ignore)
		JobStatus status = checkStatus (  false , true , true,  true , false ) ;

		// Check Directory
		File path = new File (dirPath);
		if ( !path.isDirectory() )
			throw new UnsupportedOperationException ("The path " +dirPath + " does not exists or it is not a directory" );
/*
		// Check for OutputSandbox Files: the attribute is mandatory only if jid is a JOB (not mandatory for a DAG)
		if(    ( !jad.hasAttribute (Jdl.OUTPUTSB)   ) && ( status.getValInt ( JobStatus.JOBTYPE ) == JobStatus.JOBTYPE_JOB  )    )
			return new Result ( jid.toString () ,
				new JobException ( "The OutputSandbox attribute has not been specified in the Job JDL. Unable to retrieve the output files." ) ,
				Result.OUTPUT,
				Result.GETOUTPUT_FAILURE  ) ;
		return nsOutput ( jid.toString()  ,  dirPath);
*/
		String parent = status.getValString (  JobStatus.PARENT_JOB  ) ;
		nsInit (  new Url  (  status.getValString (JobStatus.NETWORK_SERVER )  )   ) ;
		return getOutput (  path   , status ,   (parent==null )  ) ;

	}

	private Result getOutput( File path  , JobStatus status , boolean purge)  throws
		JobAdException, // checkAll
		UnsupportedOperationException, // Not allowed
		FileNotFoundException, // Proxy
		org.globus.gsi.GlobusCredentialException // Proxy
	{
		 String jobid = status.getValString( JobStatus.JOB_ID) ;
		if   ( status.code() !=JobStatus.DONE )
			throw new UnsupportedOperationException ("Job Status != Done. Unable to retrieve OutputFiles from " + jobid );
		if ( status.getValInt ( JobStatus.DONE_CODE) !=0 )
			return new Result (  jid.toString() ,
						new JobException ("Done code !=0 for " + jobid) ,
						Result.OUTPUT, Result.GETOUTPUT_FAILURE ) ;
		//NO problems so Far: Create Directory:
		if ( path.isDirectory() ) {
		   /** OK: Path is a directory and already Exists */
                }
		// The path exists but it is not a directory
		else if ( path.exists() ) {
			//  Path already BUT NOT DIR 
			throw new UnsupportedOperationException ("The path " +path + " is already existing but it is not a directory" );
		}
		// The path does not exist. Try to create the directory
		else{
			//  Path Creation.....  
			if (!path.mkdir() ) throw new UnsupportedOperationException ("Unable to create the path: " + path );
		}
		// Check For Dags children iteration:
		if  (  status.getValInt ( JobStatus.JOBTYPE )==  JobStatus.JOBTYPE_DAG ){
			Vector subJobs =status.getSubJobs() ;
			JobStatus sonStatus = null ;
			JobId sonId = null ;
			for (int i = 0 ; i< subJobs.size()  ; i++){
				sonStatus = (JobStatus)(subJobs.get(i) )  ;
				sonId = new JobId (  sonStatus.getValString (JobStatus.JOB_ID )   );
				//  terating over son 
				getOutput (    new File (  path.getAbsolutePath() + "/" + System.getProperty( "user.name" ) + "_"  + sonId.unique) ,  sonStatus  , purge   ) ;
			}
		}
		// Actually Retrieve the output files
		Result result = nsOutput ( jobid  ,  path.getAbsolutePath() ) ;
		// If some error occurred do not purge
		if ( result.getCode()>=10 ) return result;
		// If purge needed do it:
		if ( purge ){
			api.ns_purge (  jobid.toString()  );
		}
		if(    ( status.getValInt ( JobStatus.CHILDREN_NUM) >0 )  && ( result.getCode() <10 )  )
			// It is a Dag and successfully retrieved
			result = new Result ( jobid.toString () , path.getAbsolutePath() , Result.OUTPUT,  Result.SUCCESS  ) ;
		if ( path.list().length==0 )
			try{
				path.delete() ;
			}  catch (SecurityException  exc ) {
				/* Unable to remove Directory, nothing to do*/
				System.out.println("Job::getOutput Warning, unable to remove directory : " + path ) ;
			}
		return result ;
	}




	/****************************************************************
	*    PRIVATE METHODS (LB&NS api native calls)
	****************************************************************/
	private void lbInit   ( ) throws java.io.FileNotFoundException,  org.globus.gsi.GlobusCredentialException{
		if (api == null)   api = new Api() ;
		if (! jCollect) {
			//The job does not belong to a collection : credential check needed
			if (userCred== null)     userCred = new UserCredential () ;    userCred.checkProxy() ;
			api.initialise () ;
		}
	}
	/** Perform RB initalisation*/
	private void nsInit   ( Url nsAddress) throws java.io.FileNotFoundException,  org.globus.gsi.GlobusCredentialException {
		if (api == null)   api = new Api() ;
		if (jCollect){
			if (  jobType == JOB_AD)
				// The Job Init has been already called by JobCollection, no more needed
				api.ns_init( nsAddress.getHost() , nsAddress.getPort() , nsLoggerLevel ) ;
		}else{
			// DOES NOT belong to a Collection initial checks needed
			if (userCred== null)   userCred = new UserCredential () ;   userCred.checkProxy() ;
			nsAddress.checkNS() ;
			api.initialise () ;
			api.ns_init( nsAddress.getHost() , nsAddress.getPort() , nsLoggerLevel ) ;
		}
	};
	/** Cancel the Job from RB*/
	private int  nsCancel( String jobId  ) {
		int result  = api.ns_cancel (jobId ) ;
		// Delete the Ns pointer
		// api.ns_free();
		return result ;
	}   ;
	/** Submit the job*/
	private void nsSubmit (  Url  nsAddr ,  Url lbAddr )
		throws
		javax.naming.directory.InvalidAttributeValueException, // JobAd addVal , getVal
		java.net.UnknownHostException ,  // Shadow
		org.glite.wms.jdlj.JobAdException, // checkAll
		NoSuchFieldException, // each JobAd attribute
		GlobusCredentialException,
		FileNotFoundException // Credential problems
	{
		String jdl = null ;
		if    (jobType == JOB_AD){
			//Retrieve Multi Attribute list:
			jad.checkAll() ;
			Vector vect = api.ns_multi_attribute();
			if ( (vect!=null)  && vect.size()>0 ){
				nsMultiAttribute ( Jdl.RANK, vect ) ;
				nsMultiAttribute ( Jdl.REQUIREMENTS , vect  ) ;
			}
			if (jad.hasAttribute (Jdl.OUTPUTDATA) )
				jad.addAttribute (Jdl.OUTPUTSB , Jdl.DSUPLOAD + "_" + jid.unique+".out" ) ;
			/*  JobType management */
			if (jad.hasAttribute (Jdl.JOBTYPE) ){
				if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_INTERACTIVE ) ) {
					// Shadow Port attribute management:
					if (   jad.hasAttribute( Jdl.SHPORT  )  )
						shadow.console( jad.getInt(Jdl.SHPORT)  );
					else
						shadow.console(   );
					jad.addAttribute (  Jdl.ENVIRONMENT , Jdl.INTERACTIVE_STDIN    +"="  + shadow.getPipeIn() );
					jad.addAttribute (  Jdl.ENVIRONMENT , Jdl.INTERACTIVE_STDOUT +"="  + shadow.getPipeOut() );
					jad.addAttribute (  Jdl.ENVIRONMENT , Jdl.INTERACTIVE_SHADOWHOST +"="  + shadow.getHost() );
					jad.addAttribute (  Jdl.ENVIRONMENT , Jdl.INTERACTIVE_SHADOWPORT +"="  + String.valueOf( shadow.getPort() ) );
				}
				if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_CHECKPOINTABLE ) ) {
					// TBD retrieve JobState
				}
			}
			// USER TAGS implementation
			if (   jad.hasAttribute ( Jdl.USER_TAGS)  ){
				Ad uTags = jad.getAd (Jdl.USER_TAGS)  ;
				Iterator  tagsIterator = uTags.attributes() ;
				String tagName  = null ;
				while ( tagsIterator.hasNext()  ){
					tagName = (String)          (tagsIterator.next());
					api.lb_log_tag( tagName , (String)(    uTags.getStringValue(tagName).get(0)   )     ) ;
				}
			}
			jdl = jad.toSubmissionString() ;
		} else {
			jdl =api.dagToString( 2) ;  // No nodes are shown (for registering)
		}
		// initialise the LB context
		api.lb_init(  nsAddr.getHost() ) ;
		api.initialise() ;
		if    (jobType == JOB_AD){
			// PERFORM REGISTRATION:
			if (jad.hasAttribute (Jdl.JOBTYPE) ) if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_PARTITIONABLE ) ) {
				// PERFORM A LIST MATCH
				int res_number = ((Vector)listMatchingCE(nsAddr).getResult()).size() ;
				// Pre-post check (if pre-post job present res_number incremented)
				if (  jad.hasAttribute ( Jdl.PRE_JOB ) ) res_number++ ;
				if (  jad.hasAttribute ( Jdl.POST_JOB ) ) res_number++ ;
				// Register jobs (perform a registration for res_number jobs and create jobids)
				// and Switch to DAG
				api.registerPart( jid.toString() , jad.toString(), jad.toSubmissionString(), nsAddr.getAddress(), res_number );
				jobType = DAG_AD ;
				jdl = api.dagToString ( 1  ) ;  // Submission String
				api.dag_logUserTags ( jid.toString() ) ;
			}  else  api.lb_register(jid.toString() , jdl , nsAddr.getAddress() );  // ANY JOB (but partitionable)
			//					SPECIAL features:
			if (jad.hasAttribute (Jdl.JOBTYPE) ){
				//	1	Interactive:
				if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_INTERACTIVE ) ) {
					api.lb_log_listener( jid.toString() , shadow.getHost() , shadow.getPort()  ) ;
				}
				//	2	CheckPointable:
				if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_CHECKPOINTABLE ) ) {
					api.lb_logSync(jad.lookup(Jdl.CHKPT_JOBSTATE ).toString());
					//TBD try to send the flat JDL avoiding multiple check
					jad.delAttribute(Jdl.CHKPT_JOBSTATE);
				}
			}  //end has attribute JOBTYPE
		}else{
				api.registerDag ( jid.toString() , nsAddr.getAddress() );
				jdl = api.dagToString ( 1  ) ;  // Submission String
				api.dag_logUserTags ( jid.toString() ) ;
		}
		api.lb_log_start ( jdl , nsAddr.getHost() , nsAddr.getPort());
		// LB  Sequence Code
		if    (jobType == JOB_AD) {
			/*   JOB  */
			// Submission performs log transfer ok as well
			jad.setAttribute(  Jdl.LB_SEQUENCE_CODE ,api.lb_getSequence());
			jdl = jad.toSubmissionString() ; //TBD try to send the flat JDL avoiding multiple check
			api.ns_submit (jdl , nsAddr.getHost() , nsAddr.getPort() );
		}else {
			/*   DAG */
			// Submission performs log transfer ok as well
			api.dagSetAttribute ( 3 ,   api.lb_getSequence()  ) ;
			jdl = api.dagToString ( 1  ) ;  // Submission String
			api.ns_submit (jdl , nsAddr.getHost() , -nsAddr.getPort() );
		}
		// Interactive: if no error has been found, launch the interactive listener:
		if    (jobType == JOB_AD){
			// Dgas Authorisation check
			if ( jad.hasAttribute(Jdl.HLR_LOCATION) )  api.ns_dgas(  jid.toString()  , jad.getString ( Jdl.HLR_LOCATION )  );
			// Interactive jobs: lunche the listener
			if (jad.hasAttribute (Jdl.JOBTYPE) )   if (  jad.getStringValue(Jdl.JOBTYPE).contains(  Jdl.JOBTYPE_INTERACTIVE ) ) shadow.start();
		}
		jobType = JOB_SUBMITTED ;
	}

	/** Retrieve job output files*/
	private Result nsOutput (  String jobId ,  String dir ){
		String source_path =  "gsiftp://" + nsAddress.getHost() +  "//"   ;
		String dest_path = "file:///" + dir ;
		Vector outputList = api.ns_output_files ( jobId.toString()) ;
		if ( outputList.size() ==0 )
			return new Result(
				jobId.toString () ,
				"No file of the Job OutputSandbox is available on the RB node. Unable to retrieve the output files." ,
				Result.OUTPUT,
				Result.SUCCESS   ) ;
		String outputErr = new String() ;
		UrlCopy uCopy = new UrlCopy();
		File outputFile  = null ;
		for  (  int i = 0 ; i< outputList.size() ; i++) {
			try {
				outputFile = new File ( (String) outputList.get(i) );
				uCopy.setSourceUrl (     new GlobusURL (  source_path + "/" +  outputFile.getPath()   )  ) ;
				uCopy.setDestinationUrl (new GlobusURL (  dest_path    + "/" + outputFile.getName()  )  ) ;
				uCopy.copy() ;
			}catch ( org.globus.io.urlcopy.UrlCopyException exc ) {
			 	outputErr += "\n" + (String) outputList.get(i)   ;
			}catch (java.net.MalformedURLException exc ) {
			 	outputErr += "\n" + (String) outputList.get(i)   ;
			}
		}
		int CODE ;
		Object obj = null ;
		if (outputErr.length() ==0){ //NO errors found,
			CODE =Result.SUCCESS ;
			obj = dir ;
		}else {
			CODE = Result.GETOUTPUT_FAILURE ;
			outputErr = "UrlCopy Failed for: " + outputErr ;
			obj = new  JobException ( outputErr ) ;
		}
		return new Result( jobId.toString () , obj , Result.OUTPUT,  CODE  ) ;
	}  ;

	/** Retrieve Multi attribute list and check if Member-IsMember syntax is ok */
	private void nsMultiAttribute (  String attrName, Vector multi) throws  JobAdException {
		String attrValue =  jad.lookup(  attrName  ).toString()  ;
		if (attrValue== null) return;
		Vector values  = new Vector ();
		Vector lists = new Vector() ;
		int len = attrValue.length() ;
		//find Keyword memebr:
		int position = attrValue.indexOf ("Member") ;
		int openPar, comma1, comma2 , closedPar ;
		int i = 0 ;
		// Parsing the string in order to find member function usage
		while (position!=-1 ){
			// position found
			openPar   = attrValue.indexOf ("("  , position) +1;
			while (   attrValue.charAt(openPar) == ' '  )  openPar++;
			comma1     = attrValue.indexOf (","  , position) - 1 ;
			comma2 =comma1+2 ;
			while (   attrValue.charAt(comma1) == ' '  )  comma1-- ;
			while (   attrValue.charAt(comma2) == ' '  )   comma2++ ;
			closedPar = attrValue.indexOf (")"  , position) -1 ;
			while (   attrValue.charAt(closedPar) == ' '  )  closedPar--;
			//check Consistency:
			if (  openPar !=-1 && closedPar < len  && comma1 !=-1 && openPar<comma1 && comma2 < closedPar ){
				lists.add(         attrValue.substring (comma2 , closedPar  +1).trim()   );
				values.add(   attrValue.substring (          openPar , comma1  +1 ).trim()   );
			}
			position = attrValue.indexOf ("Member", closedPar ) ;
			i++ ;
		} //end while position...
		// iterating over the multi attributes to check whether are wrongly used
		for  (  int j = 0 ; j <multi.size(); j++  ) {
			String it = (String)multi.get( j )  ;
			if (  attrValue.indexOf ("other." +  it ) !=-1  ){
				//The Attribute cannot be present in the values
				if  ( values.contains( "other." + it) ){
					throw new JobAdException (attrName +": wrong usage of Member/InMember function for multi attribute "
						+it + ".\nSyntax is: Member/IsMember(List,Value)" );
				}
				//The Attribute must be present in the lists
				if  (! lists.contains( "other." + it) ){
					throw new JobAdException (attrName +": wrong usage of Member/InMember function for multi attribute "
						+it + ".\nSyntax is: Member/IsMember(List,Value)" );
				}
			}
		}
	}





	/** Check the status of the job and perform if necessary some operation
	lbInit
	nsInit
	loadJdl
	jobFinished: should be true for getOutput option and false for attach and cancel
	ignore: this parameter ignore the exit status exit code
	*/
	private JobStatus checkStatus (  boolean lbApi,  boolean nsApi,  boolean loadJdl,  boolean jobFinished , boolean ignore)
		throws org.globus.gsi.GlobusCredentialException, JobAdException, java.io.FileNotFoundException
	{
		Api.initialise() ;
		Result result = getStatus(   loadJdl  ) ;
		if ( result.getCode() != Result.SUCCESS )
			throw new UnsupportedOperationException ("Unable to retrieve the status of the Job" );
		JobStatus status =  (JobStatus)result.getResult( ) ;
		boolean thrown = false ;
		if (  !ignore ) switch ( status.code()  ){
			case JobStatus.SUBMITTED:
			case JobStatus.WAITING:
			case JobStatus.READY:
			case JobStatus.SCHEDULED:
			case JobStatus.RUNNING:
				// cancel & attach OK
				if ( jobFinished  )   thrown = true ;
				break;
			case JobStatus.DONE:
				// attachment for DONE (Failure) jobs is allowed
				switch (     status.getValInt ( JobStatus.DONE_CODE)    ){
					case 1: // attach && cancel OK
						if ( jobFinished  )   thrown = true ;
						break ;
					case 0: // getOutput OK
						if ( !jobFinished  )   thrown = true ;
						break ;
					default: // always an error
						 thrown = true ;
				}
				break;
			default:  // CLEARED, ABORTED , CANCELLED , UNKNOWN,  PURGE
 				thrown = true ;
		}  // end switch ( status.code()  ){
		if (thrown)
			throw new UnsupportedOperationException ("Status code '" + status.code[status.code()] +  "' doesn't allow operation" );
		// Notice that the getStatus operation had already initialized the Api constructor
		//                              LB API INITIALISATION
		if (lbApi) api.lb_init ( new Url  (status.getValString (  JobStatus.NETWORK_SERVER  )   ).getHost ()  ) ;
		//                              NS API INITIALISATION
		if (nsApi)  {
			nsAddress = new Url  (status.getValString (  JobStatus.NETWORK_SERVER  )   ) ;
			api.ns_init ( nsAddress.getHost() , nsAddress.getPort() , nsLoggerLevel  ) ;
		}
		//                              JOBAD INITIALISATION
		if (loadJdl)
			try{
				jad = new JobAd (    status.getValString(JobStatus.JDL )   ) ;
			}catch (java.text.ParseException exc) {
				throw new JobAdException ( exc.getMessage()  );
			}
		return status ;

	}

	/**Empty Constructor Job*/
	public static final int  JOB_NONE             = 0 ;
	/**JobId Constructor utilised*/
	public static final int  JOB_ID                   = 1 ;
	/**JobAd Constructor Utilised*/
	public static final int  JOB_AD                  = 2 ;
	/** JobAd constructor and then submitted, both JobAd and JobId are avaliable*/
	public static final int  JOB_SUBMITTED     = 3 ;
	/**DagAd Constructor Utilised*/
	public static final int  DAG_AD                  = 4 ;

	/************************************************************
	*    PRIVATE-PACKAGE MEMBERS:
	************************************************************/
	int jobType;
	private int nsLoggerLevel =0 ;
	/* Internal JobId instance */
	JobId jid ;
	/* Internal JobAd instance   */
	JobAd jad ;
	/* Internal dagad file instance*/
	String dagad ;
	boolean uiLogDefault = true ;
	private Url nsAddress , lbAddress ;
	/** This Variable is used while submitting an Interactive Job */
	Shadow shadow;
	/** This api instance perform the operations:*/
	private Api api =null;
	/** Stores the path of the proxy (if different from the default)*/
	private UserCredential userCred ;
	/**  JobCollection managment*/
	boolean jCollect =false;    // To check if the job belongs to a Collection
};

/*
* JobCollection.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
import java.util.* ;
import java.io.File ;
import  org.glite.wms.jdlj.JobAd ;
import  org.glite.wms.jdlj.Ad ;
import java.util.Random ;    // Randomize onto lb Addresses
import org.globus.gsi.GlobusCredentialException;
import java.io.IOException ;

/**
 * The JobCollection Class is a container class for Job objects .
 * <p>
 A JobCollection has the main purpose of allowing the execution of collective operations on sets of independent jobs.
 * The JobCollection class is just a logical container, and both not yet submitted and already
 * submitted jobs can be inserted in it. A job collection is somehow orthogonal wrt a job
 * cluster being a set of dependent jobs (e.g. all jobs spawned by the same father process).
 * The allowed operations are:
 * <ul>
 * <li> Submitting a collection of jobs to a network server
 * <li> Cancelling a collection of  jobs
 * <li> Retrieving jobs status information from LB server
 * <li> Retrieving the output sandbox files from a collection of jobs
 * </ul>
 *
 * @see  Job
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class JobCollection implements Runnable{
	/** Instantiates an  empty  JobCollection object*/
	public JobCollection()   {  this.jobs = new Vector() ;  }   ;
	/** Instantiates a collection with n copies of a job (the Job has to be of JOB__AD type)
	*@param   job  the source Job (of JOB_TYPE) instancies
	*@param   n the number of copies to be filled in the collection
	*@throws JobCollectionException  The job is not of  JOB__AD type  */
	public JobCollection( Job job , int n) throws JobCollectionException {
		this () ;
		if (    job.jobType != Job.JOB_AD)       //The Job isn't of AD type, cannot insert
			throw new JobCollectionException (job.jid + ": unable to insert several non-ad jobs") ;
		job.jCollect = true ;
		for (int i = 0 ; i< n ; i++){
			jobs.add( job.copy() );
		}
	} ;
	/**
	* Instantiates a JobCollection object from a vector of Job
	* @param   jobs  the vector of Job instances that have to be inserted
	* @throws JobCollectionException Unable to add duplicate JobId
	* @throws NoSuchFieldException One of the Job is of JOB_NONE type */
	public JobCollection( Vector jobs )throws  JobCollectionException, NoSuchFieldException{
		this();
		for (int i = 0 ; i< jobs.size() ; i++ )
			insert (   (Job) jobs.get(i)  );
	};
	/**
	* Instantiates a JobCollection object from an array of Job
	*@param   jobs  the array of Job instances that have to be inserted
	* @throws JobCollectionException Unable to add duplicate JobId
	* @throws NoSuchFieldException One of the Job is of JOB_NONE type*/
	public JobCollection( Job[] jobs )throws JobCollectionException, NoSuchFieldException {
		this();
		for (int i = 0 ; i< jobs.length ; i++ ) insert (   jobs[i]  );
	};
	/**
	* Check the size of the collection
	* @return true if the collection's size is empty (length = 0)*/
	public boolean empty(){ return jobs.isEmpty();  };
	/**
	* @return the lenght of the inserted Jobs */
	public   int size(){   return jobs.size();  };
	/** Test wheter the specified job is already present in the collection */
	public boolean  contains ( Job job  ) {
		switch ( job.jobType){
			case Job.JOB_NONE:
				return false ;
			case Job.JOB_ID :
			case Job.JOB_SUBMITTED:
				String id  =( job.jid ).toString()   ;
				for ( int i = 0  ;  i< jobs.size() ; i++)  if    (   (  ( (Job) jobs.get(i) ).jid   ).equals(  job.jid   )   )  return true ;
			default:
				return false ;
		}
	}
	/**
	* Insert a new Job (of JobvId type) into the collection
	*@param jobId  the JobId instance that has to be inserted
	* @throws JobCollectionException Unable to add duplicate JobId */
	public void insertId( JobId jobId  )throws JobCollectionException {
		Job job  =  new Job (jobId) ;
		if ( contains (job) ) throw new JobCollectionException ( job.jid.toString() + ": unable to add duplicate Id-jobs") ;
		job.jCollect = true ;
		jobs.add( job );
	}
	/**
	* Insert a new Job (of JobAd type) into the collection
	*@param jobAd the JobAd instance that has to be inserted
	*@throws JobCollectionException Unable to insert the Job */
	public void insertAd( JobAd jobAd  ) {
		Job job  =  new Job (jobAd) ;
		job.jCollect = true ;
		jobs.add( job ) ;
	}
	/**
	* Insert a new Job to the collection
	* @param job tht Job instance that has to be inserted
	* @throws JobCollectionException Unable to add duplicate JobId
	* @throws NoSuchFieldException If the Job is of JOB_NONE type
	*/
	public void insert( Job job  )throws JobCollectionException , NoSuchFieldException {
		if ( contains (job) ) throw new JobCollectionException ( job.jid.toString() + ": unable to add duplicate Id-jobs") ;
		else if ( job.jobType == Job.JOB_NONE ) throw new  NoSuchFieldException ("the Job instance  is empty");
		job.jCollect = true ;
		jobs.add( job );
	};

	/** Remove a specified Job from the collection
	* Delete the specified job from the collection (if the id has been set)
	* Delete the last occurrence of the job from the collection (if the ad has not been set)
	*@param job the Job that has to be removed
	*@throws  NoSuchFieldException Unable to remove the Job
	*/
	synchronized public void remove( Job job)throws  NoSuchFieldException{
		for (int i = 0 ; i< jobs.size()  ; i++ )
			if (       ((Job) (  jobs.get(i)   )).equals (  job)  ){
				jobs.remove ( i ) ;
				return ;
		}
		throw new  NoSuchFieldException (job.jid + ": unable to remove. Job not found inside the collection");
	};
	/** Deletes all elements from the collection.*/
	public void clear()   {  jobs.setSize(0);   };
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
	Set a different  Proxy certificate from the default one
	* @param cp The full path of the proxy certificate file to be set
	* @throws IOException Unable to find-load-read the specified path
	* @throws GlobusCredentialException Unable to get the specified proxy certificate*/
	public void setCredPath( File cp) throws  GlobusCredentialException, IOException {
		userCred = new UserCredential (cp );
	};
	/**
	Set the Proxy certificate as default
	* @throws IOException Unable to find-load-read the default proxy file
	* @throws GlobusCredentialException Unable to get the default proxy certificate */
	public void unsetCredPath() throws   GlobusCredentialException, IOException  {
		userCred = new UserCredential () ;
	};
	//@}
				/**@name   Iteration action*/
	//@{
	/** @return an iterator pointing to the beginning of the collection*/
	public Iterator jobs(){   return jobs.iterator();   };

	/**
	* Submit method
	* @param  ns The Network Server   address
	* @param  lbs a vector of Urls containing all the logging and bookkeeping addresses
	* @param   ceId The Computing Element Identificator where to perform the job
	* @return a Vector of Result, one for each Job in the collection (with the same order as inserted)
	* @throws GlobusCredentialException Unable to get proxy certificate information
	* @throws FileNotFoundException - Unable to find proxy certificate file.
	* @throws JobCollectionException Some error occurred while executing the operation
	* @throws InterruptedException An Error occurred while waiting for a thread to finish
	* @throws NoSuchFieldException if the LB vector is empty
	*/
	public Vector submit( Url ns ,  Vector lbs , String ceId ) throws NoSuchFieldException , JobCollectionException ,  
		java.lang.InterruptedException,org.globus.gsi.GlobusCredentialException , java.io.FileNotFoundException{
		//Initialise parameter for Thread call:
		operation = Result.SUBMIT ; //operation required
		nsAddress = ns ;
		nsAddress.checkNS() ;
		if ( lbs.size()!=0 )  lbAddresses =lbs ;
		else throw new NoSuchFieldException ("Unable to submit: Empty LbAddress Vector") ;
		// Randomise LB Starting server
		Double rndbl = new Double(Math.random()*Long.MAX_VALUE);
		Random rnd = new Random(rndbl.longValue());
		lbPosition =   rnd.nextInt()%lbs.size() ;
		if (lbPosition <0 )  lbPosition = -lbPosition ;
		paramTh    =  ceId ;
		// Start executing threads
		launchThreads();
		return vect ;
	}
	/** Cancel the job from the NS
	* @return a Vector of Result, one for each Job in the collection (with the same order as inserted)
	* @throws JobCollectionException Some error occurred while executing the operation
	* @throws InterruptedException An Error occurred while waiting for a thread to finish
	* @throws GlobusCredentialException Unable to get proxy certificate information
	* @throws FileNotFoundException - Unable to find proxy certificate file.
	*/
	public Vector cancel( ) throws JobCollectionException ,  java.lang.InterruptedException,org.globus.gsi.GlobusCredentialException , java.io.FileNotFoundException  {
		//Initialise parameter for Thread call:
		operation = Result.CANCEL ; //operation required
		// Start executing threads
		launchThreads();
		return vect ;
	}   ;
	/**
	* Retrieves the status of the first Jobs of the collection once it enters one of the specified status code
	* @param states an array of JobStatus status codes
	* @param timeout seconds the method has to wait for notification. After that period the methods returns anyway. If 0 specified it hangs untill notify arrives
	* @return a Success Result containing the JobStatus (or  empty   when timeout reached)
	* @throws UnsupportedOperationException  The Operation required is not allowed for the Job
	* @throws FileNotFoundException Unable to find certificates files
	* @throws gsi.GlobusCredentialException error while checking certificates validity
	* @see Result#TIMEOUT_REACHED
	* */
	public Result notify   ( int [] states  , int timeout ) throws JobCollectionException ,  java.lang.InterruptedException ,org.globus.gsi.GlobusCredentialException , java.io.FileNotFoundException, java.lang.NoSuchFieldException{
		if (    size() ==0  )
			throw new JobCollectionException ("Unable perform the operation over an empty collection") ;
		Ad jobStates = new Ad () ;
		Job job ;
		try{
			for (int i = 0  ; i < size()   ;   i++ ) {
				job = (Job) ( jobs.get(i) ) ;
				if ((job.jobType != Job.JOB_NONE) && (job.jobType != Job.JOB_AD))
					jobStates.addAttribute ("JOBIDS" , job.getJobId().toString() );
			}
			for (int i =0 ; i< states.length; i++ ){
				if(   ( states[i]< JobStatus.SUBMITTED) ||  ( states[i] > JobStatus.PURGED)   )
					throw new UnsupportedOperationException ("Unable to notify status code out of limits: "+ states[i]);
				jobStates.addAttribute ("STATES" , states[i] );
			}
		} catch ( javax.naming.directory.InvalidAttributeValueException exc ) {
			throw new UnsupportedOperationException ("notify: Fatal Error"); /* never happens */
		}
		// Initialise context and check permission
		Api api = new Api();
		api.initialise();
		if (userCred == null) userCred = new UserCredential();
		userCred.checkProxy();
		JobStatus status = api.lbNotification ( jobStates.toString() , timeout)  ;
		if ( status.size()>0 )
			return new Result ( status.getValString (JobStatus.JOB_ID)  ,  status ,Result.STATUS , Result.SUCCESS ) ;
		else
			return new Result ( "jobid not found"  ,   
			new UnsupportedOperationException ("Timeout reached without any requested Status Change" ),
			Result.STATUS , Result.TIMEOUT_REACHED) ;
	}
	/** Retrieve the status information from the LB
	* @return a Vector of Result, one for each Job in the collection (with the same order as inserted)
	* @throws JobCollectionException Some error occurred while executing the operation
	* @throws InterruptedException An Error occurred while waiting for a thread to finish
	* @throws GlobusCredentialException Unable to get proxy certificate information
	* @throws FileNotFoundException - Unable to find proxy certificate file.
	* */
	public Vector getStatus(  ) throws JobCollectionException ,  java.lang.InterruptedException ,org.globus.gsi.GlobusCredentialException , java.io.FileNotFoundException {
		//Initialise parameter for Thread call:
		operation = Result.STATUS ; //operation required
		// Start executing threads
		launchThreads();
		return vect ;
	};
	/**    Get the output files of the jobs (SYNC)
	* @param  dirPath the path where to retrieve the OutputSandbox files
	* @return a Vector of Result, one for each Job in the collection (with the same order as inserted)
	* @throws JobCollectionException Some error occurred while executing the operation
	* @throws InterruptedException An Error occurred while waiting for a thread to finish
	* @throws GlobusCredentialException Unable to get proxy certificate information
	* @throws FileNotFoundException - Unable to find proxy certificate file.
	*/
	public Vector getOutput( String dirPath)  throws JobCollectionException ,  java.lang.InterruptedException,org.globus.gsi.GlobusCredentialException , java.io.FileNotFoundException {
		//Initialise parameter for Thread call:
		operation = Result.OUTPUT ; //operation required
		paramTh    =  dirPath ;
		// Start executing threads
		launchThreads();
		return vect ;
	};
	/** @return the maximum number of Thread*/
	private   int getMaxThreadNumber (){
		if (maxThreadNumber>0) return maxThreadNumber;
		else return  MAX_THREAD_NUMBER ;
	};
	/** This method is used to override the MAX_THREAD_NUMBER macro variable
	*@param maxThread the max number or simultaneous threads allowed  (unless the -DWITHOUT_THREAD option is specified while compiling)
	*/
	public void setMaxThreadNumber (  int  maxThread){
		maxThreadNumber = maxThread>MAX_THREAD_NUMBER?MAX_THREAD_NUMBER:maxThread  ;
	};
	private void launchThreads() throws JobCollectionException ,  java.lang.InterruptedException , org.globus.gsi.GlobusCredentialException, java.io.FileNotFoundException {
		if (    size() ==0  )
			throw new JobCollectionException ("Unable perform the operation over an empty collection") ;
		// Initializing a new vector
		vect = new Vector ( ) ;
		vect.setSize( size()  )  ;
		// Recoursive Thread variables managing
		int maxTh = (getMaxThreadNumber() > size()) ? ( size() ) : (getMaxThreadNumber()) ;  // The largest number between MAX_number of threads and collection size
		int freeTh = maxTh ; //pointing to the first free thread
		int numTh = 0 ;
		threads = new Thread [ maxTh  ]  ;
		Api api = new Api ();
		api.initialise() ;
		if (operation == Result.SUBMIT  )
			api.ns_init( nsAddress.getHost() , nsAddress.getPort() , nsLoggerLevel ) ;
		// Check User Credential
		userCred=new UserCredential(  new File (userCred.getDefaultProxy()  ) ) ;
		userCred.checkProxy();
		for (int i = 0  ; i < size()   ;   i++ ) {
			threads[ numTh%maxTh ] = new Thread(  this ,  new Integer(i).toString()   ) ;
			threads[ numTh%maxTh ].start() ;
			numTh++ ;
			//CHECK if max Thread has been reached:
			if (   numTh%maxTh ==  freeTh%maxTh ){
				userCred.checkProxy();
				threads[ freeTh%maxTh  ].join() ;
				freeTh++ ;
			}
		}
		while (freeTh%maxTh != numTh%maxTh) {
			threads[  freeTh%maxTh  ].join() ;
			freeTh++ ;
		}
	};
	/** Update the Job Result info into the Collection inner result vector.
	* this method can be overriden in order to graphicalli manipulate each result
	* @param number the number of the job insied the collection
	* @param result the result of thte required operation
	*/
	synchronized protected void appendJob (  int number , Result result  ){
		try { vect.set(   number , result ) ;
		} catch ( ArrayIndexOutOfBoundsException exc) {
			System.out.println("JobCollection::appendJob->    Warning, array index out of bound exception for: " + number +"Result= " +result) ;
		} catch (Exception exc) {
			System.out.println("JobCollection::appendJob->    Warning, Standard exception for: " + number +"Result= " +result) ;
		}
	} ;
	/*
	*  Thread Execution This method execute the passed funcion as a separate thread
	*/
	public void run () {
		//  Initial check
		int jobNumber = Integer.parseInt ( (Thread.currentThread() ).getName()  )   ;
		int jobType = ((Job) (    jobs.get( jobNumber )   )).jobType ;
		Job job = (Job) (    jobs.get( jobNumber )   ) ;
		job.setLoggerLevel (nsLoggerLevel) ;
		job.logDefaultValues(  uiLogDefault   ) ;
		Result result = null ;
		// Perform the operation
		try{
		switch (operation) {
			case Result.SUBMIT:
				lbPosition = (lbPosition+1) % lbAddresses.size();
				result=  job.submit(  nsAddress  , (Url) lbAddresses.get(lbPosition), paramTh  ) ;
				break;
			case Result.STATUS:
				result = job.getStatus( ) ;
				break;
			case Result.CANCEL:
				result = job.cancel( ) ;
				break;
			case Result.OUTPUT:
				String dirPath = paramTh  + "/"  + System.getProperty( "user.name" ) +  "_" +    job.jid.getUnique ()  ;
				File path = new File (dirPath);
				/* Do nothing, The path already exist and it is a directory: OK */
				if ( path.isDirectory() ) {
                                   /** OK */
                                }
				// The path exists but it is not a directory
				else  if ( path.exists() )
					throw new UnsupportedOperationException ("The path " +dirPath + " is already existing but it is not a directory" );
				// The path does not exist. Try to create the directory
				else
					if  (!path.mkdir() )
						throw new UnsupportedOperationException ("Unable to create the path: " + dirPath );
				result = job.getOutput(  dirPath  ) ;
				break;
			default :
				break;
		} //end switch
		appendJob ( jobNumber , result ) ;
		}catch (UnsupportedOperationException exc){
			String id = job.jid!=null?job.jid.toString():"jobid not created" ;
			appendJob( jobNumber,   new Result (id, exc ,  operation , operation + Result.FORBIDDEN  )      );
		}catch (Exception exc){
			String id = job.jid!=null?job.jid.toString():"jobid not created" ;
			appendJob( jobNumber,   new Result (id, exc ,  operation , operation + Result.FAILURE  )      );
		}
	};
	/**In order to perform JobCollection Actions, a certificate proxy file is needed */
	String credPath  ;
	/*This variable indicates the number of maximum simultaneous executing threads */
	int maxThreadNumber =1;

	/** The Job in the collection are stored in a vector */
	protected  Vector jobs;
	/** To Check if the credential are OK   */
	private UserCredential userCred ;
	private int operation;
	/** The set of paramether used by thread*/
	private Url nsAddress ;
	private Vector lbAddresses ;
	private int lbPosition ;
	/** generic paramether used by thread*/
	private String paramTh ;
	/** Store Thread information*/
	private Vector vect  ;
	/** Store Thread array instances*/
	private Thread threads[] ;
	private static final int MAX_THREAD_NUMBER = 1 ;
	private int nsLoggerLevel =0 ;
	private boolean uiLogDefault = true ;
}

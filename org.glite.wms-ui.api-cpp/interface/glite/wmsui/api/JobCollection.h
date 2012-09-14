#ifndef  GLITE_WMS_UI_CLIENT_JOBCOLLECTION_H
#define GLITE_WMS_UI_CLIENT_JOBCOLLECTION_H
/*
 * JobCollection.h
 * Copyright (c) 2004 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */
#include "Job.h"
#include "glite/lb/JobStatus.h"
#define MAX_THREAD_NUMBER  10

// JobId Definition
namespace glite { 

namespace wmsutils { 
namespace jobid { 
class JobId ; 
} 
} 

namespace wmsui {
namespace api {

class JobCollection ;
/**
 * This Class is used to return the ResultCode back to the main function  */
class resultStruct {
	typedef glite::wms::common::utilities::ResultCode ResultCode ;
	public:
		/**Default Success Constructor */
		resultStruct ( ){ set(glite::wms::common::utilities::SUCCESS) ; };
		/** Used to check the result of the operation, in case of success it returns SUCCES
		@return the result code of the operation */
		ResultCode  get () { return  result;  }
		/** Retrieve the status information from the job
		* If the required operation was a getStatus then the JobStatus instance is returned. NULL is returned otherwise.
		@return the Status attributes information of the job
		*/
		glite::lb::JobStatus* getStatus() { return &status ; }
		/** If some error occurred (FAILURE result code) retrieve the error message
		* @return A detailed description of the error of the operation */
		std::string getError (){      if (result == glite::wms::common::utilities::SUCCESS) return "" ;    else return error ;  }
		/** Retrieve the message of this result instance
		*@return the string exception (if any) representatio, smpty string otherwise
		*/
		std::string getMessage (){if (result == glite::wms::common::utilities::SUCCESS) return error; else return   ""   ;  }
		/** Destructor */
		virtual ~resultStruct () {
		}
	private:
		/**Constructor tiwh code */
		resultStruct (ResultCode res  ){ set(res) ; }
		/**Constructor with Status*/
		resultStruct (ResultCode res ,  glite::lb::JobStatus stat ){ set(res,stat) ;}
		/**Constructor with Error */
		resultStruct (ResultCode res ,  std::string err){ set(res,err); }
		/**Set  the Result code*/
		void set (  ResultCode res  );
		/**Set the Result Code and the JobStatus (if getStatus required)*/
		void set (  ResultCode res ,  glite::lb::JobStatus stat){ set(res);  status = stat; }
		/**  Set the Result Code (if failure) and the Error String */
		void set (  ResultCode res ,  std::string err ) {  set (res) ; error = err ; }
		friend class JobCollection ;
		ResultCode result ;
		glite::lb::JobStatus status ;
		std::string error ;
};

/*
 * This Class is used to store all the required paramethers needed by the threads
*/
class paramStruct {
	private:
		int jobNumber ;
		/** This Variable stores a pointer to the job that is going to be processed */
		std::vector<Job>::iterator job ;
		/** Depending on the required operation this variable can contain a string attribute*/
		std::string parA, parB ,parC ;
		int parX , parY , parZ ; //integer parameter
		friend class JobCollection ;
     };

/**
* This Struct implements the result of a JobCollection.
* its public result member contains the results of all the collection's jobs stored as a vector of jobid strings */
struct CollectionResult {
	std::vector < std::pair <std::string, resultStruct> >   result ;
};

/**
 * The JobCollection Class is a container class for Job objects .
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
 * @brief  Container class for Job objects
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class JobCollection{
 public:
				/**@name Constructors/Destructor */
	//@{
	/**Destructor*/
	virtual ~JobCollection() {};
	/** Instantiates an  empty  JobCollection object */
	JobCollection();
	/** Instantiates a collection with n copies of a job (the Job has to be of JOB__AD type)
	*@param   job   the source Job (of JOB_TYPE) instancies
	*@param   n   the number of copies to be filled in the collection */
	JobCollection(const Job& job , unsigned int n) ;
	/**
	* Instantiates a JobCollection object from a vector of Job
	*@param   jobs   the vector of Job instances that have to be inserted
	*/
	JobCollection(const std::vector<Job>& jobs);
	//@}
   
   


				/**@name Jobs insertion/remotion handling*/
	//@{
	/** Check the size of the collection
	* @return True if the collection's size is 0*/
	bool empty(){ return jobs.empty();  };
	/** @return the lenght of the inserted Jobs */
	unsigned int size(){  return jobs.size();  };
	/** Insert a new Job to the collection
	*@param job   tht Job instance that has to be inserted  */
	void insert(const Job& job);
	/** Remove a specified Job from the collection
	* Delete the specified job from the collection (if the id has been set)
	* Delete the last occurrence of the job from the collection (if the ad has not been set)
	*@param job   the Job that has to be removed*/
	void remove(const Job& job);
	/** Deletes all elements from the collection.*/
	void clear()   { jobs.clear(); };
	/** Se the verbosity level for NS debug
	*  default value = 0 (no verbosity)
	* max value = 6 (dreadful verbosity, print screen)
	* @param level NS verbosity (0-6)
	*/
	void setLoggerLevel ( int level ) ;
	/**Set a different  Proxy certificate from the default one
	* @param cp  The full path of the proxy certificate file to be set    */
	void setCredPath(const std::string cp) ;
	/**Set the Proxy certificate as default*/
	void unsetCredPath();
	/** This method is used to override the MAX_THREAD_NUMBER macro variable
	*@param maxThread   the max number or simultaneous threads allowed  (unless the -DWITHOUT_THREAD option is specified while compiling) */
	void setMaxThreadNumber (unsigned int  maxThread){ maxThreadNumber = maxThread ; };
	//@}
				/**@name   Iteration action*/
	//@{
	/** @return an iterator pointing to the beginning of the collection*/
	std::vector<Job>::iterator begin(){   return jobs.begin();   };
	/** @return an iterator pointing to the end of the collection.*/
	std::vector<Job>::iterator end()  {   return jobs.end();     };
	//@}
	

                       /**@name Operation*/
   //@{
   /** Submit method.
   * @param  ns_host  The Network Server host name
   * @param ns_port the Network Server port value
   * @param lbAddrs a vector containing all the LB knonw by the user. The LB for each job is selected randomly
   * @param  ce_id   The Computing Element Identificator where to perform the job
   * @return a JobCollection::CollectionResult instance */
   CollectionResult submit(const std::string& ns_host , int ns_port , std::vector < std::pair < std::string , int > > lbAddrs , const std::string& ce_id = "") ;
   /** Cancel the job from the NS
   * Cancel all the jobs belonging to the collection. A job can be cancelled only when it is not yet finished
   * @return a JobCollection::CollectionResult instance**/
   CollectionResult cancel( );
   /** Retrieve the status information from the LB
   * @param  statusVector   A vector that will be filled with LB status information for each job
   * @return a  CollectionResult instance*/
    CollectionResult getStatus( );
   /**    Get the output files of the jobs 
   * @param dir_path   the path where to retrieve the OutputSandbox files
   * @return a JobCollection::CollectionResult instance */
   CollectionResult getOutput(const std::string& dir_path);
   //@}
private:
	/**
	* The method which actually manages the thread execution
	*/
	CollectionResult launch( const paramStruct  &params ) ;
	/** @return the maximum number of Thread*/
	unsigned int getMaxThreadNumber (){
		if (maxThreadNumber>0)   return maxThreadNumber;
		else  return  MAX_THREAD_NUMBER ;
	};
	/**
	*  Thread Execution   This method execute the passed funcion as a separate thread (unless the -DWITHOUT_THREAD option is specified while compiling)
	*/
	pthread_t ExecuteThread(void* (*fn)(void*), void *arg) ;
	/**
	* Join the Thread and retrieve Parameters
	*/
	resultStruct   retrieve           (pthread_t    tid ,  int jobNumber )  ;
	/** The Job in the collection are stored in a vector */
	std::vector<Job> jobs ;
	/**   * To Check if the credential are OK  */
	UserCredential userCred ;
	
	enum collection_operation {   SUBMIT,   STATUS ,   CANCEL ,   OUTPUT ,   LOGINFO ,   LISTMATCH };
	collection_operation operation ;	
	std::string nsHost  ;
	int nsPort ;
	std::vector< std::pair<std::string , int > > lbAddresses ;
	int lbPosition ;
	//Static Thread Job methods:
	static void* submitTo    (void* paramStruct);
	static void* cancelTo    (void* paramStruct) ;
	static void* statusTo      (void* paramStruct) ;
	static void* getOutputTo (void* paramStruct) ;
	/**In order to perform JobCollection Actions, a certificate proxy file is needed */
	std::string cred_path  ;
	/*This variable indicates the number of maximum simultaneous executing threads */
	unsigned int maxThreadNumber ;
	unsigned int loggerLevel ;
};

} // api
} // wmsui
} // glite

#endif


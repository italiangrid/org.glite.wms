#ifndef  GLITE_WMS_UI_CLIENT_JOB_H
#define GLITE_WMS_UI_CLIENT_JOB_H
/*
 * Job.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */
#include <map>
#include <vector>
#include "glite/wms/common/utilities/result_codes.h"
#include "glite/wmsui/api/UserCredential.h"
#include "glite/wmsui/api/Shadow.h"
#include "glite/lb/consumer.h"
#define EMPTY_STRING              ""
#define LB_DGLOG_PROT           "x-dglog://"
#define LB_DEFAULT_PROT        "https://"
#define DGLB_SOURCE_UI          "UserInterface"
#define EDG_WLU_SOURCE_NS "NetworkServer"
#define CHAR_BUFFER_SIZE      1024
#define JOB_CANCEL_ERR             -1
#define GLITE_LB_LOG_DESTINATION "EDG_WL_LOG_DESTINATION"

namespace glite { 

namespace wmsutils {
namespace jobid {	
class JobId ;      
}
}

namespace wms {

namespace jdl { 		
class JobAd ;   
class Ad ;    
}

// NS
namespace manager {    
namespace ns { 
namespace client {
class NSClient ;         
}      
}
}

namespace checkpointing {   
class JobState ; 
}

} // wms

// lB
namespace lb { 
class JobStatus ; 
class Event ; 
} 

} //namespace glite

namespace glite {
namespace wmsui {
namespace api {

	/** Current situation of the Job   */
	enum jType {
		/** Non type. The job has been initilised with the default ctor*/
		JOB_NONE,
		/** Ad type. The job has been initialised with the JobAd ctor*/
		JOB_AD,
		/** Id type. The job has been initialised with the JobId*/
		JOB_ID,
		/** Submitted type. The job has been initialised with the JobAd ctor and then submitted*/
		JOB_SUBMITTED,
		/** type. The job has been initialised with*/
		JOB_CANCELLED
	} ;
class Listener;
class Shadow ;
/** Logging info Event type definition*/
typedef std::vector <glite::lb::Event> Events ;
/** Job Status info type definition*/
typedef glite::lb::JobStatus Status ;

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
 */
class Job{
	public:
	/**@name Constructors/Destructor */
	//@{
		/** Instantiates an  empty  Job object */
		Job();
		/** Instantiates an  Job object with a JobId
		* @param id a JobId instance
		* @exception JobOperationException If the JobId is empty  */
		Job(const glite::wmsutils::jobid::JobId& id);
		/** Instantiates an  Job object with a JobAd
		* @param ad a JobAd instance		
		* @exception JobOperationException If the JobAd is empty  */
		Job(const glite::wms::jdl::JobAd& ad);
		/** Copy constructor*/
		Job(const Job& job) ;
		/** destructor*/
		~Job();
		/**Assignment operator*/
		void operator=(const Job& job );
		/**
		* Static Open ssl initialisation, called before any operation
		* Prepare the
		*/
		static void initialise () ;
	//@}

	/**@name Get/Set Methods */
	//@{
		/** Get the JobId instance
		* @return a pointer to the JobId intance */
		glite::wmsutils::jobid::JobId* getJobId() ;
		/** Get the JobAd instance
		* @return a pointer to the JobAd intance*/
		glite::wms::jdl::JobAd* getJobAd() ;
		/**Set a different  Proxy certificate from the default one
		* @param cp The full path of the proxy certificate file to be set*/
		void setCredPath(const std::string cp) ;
		/**Set the Proxy certificate as default*/
		void unsetCredPath() ;
		/** Se the verbosity level for NS debug
		* @param level default value = 0 (no verbosity), max value = 6 (dreadful verbosity, print screen) */
		void setLoggerLevel ( int level ) ;
		/** set  the JobAd instance
		* @param ad the JobAd Instance to be set  */
		void setJobAd(const glite::wms::jdl::JobAd& ad);
		/** set  the JobId instance
		* @param id the JobId Instance to be set */
		void setJobId(const glite::wmsutils::jobid::JobId& id);
		/** Set the JobAd member attribute of the Job instance to the job description got from the LB */
		void retrieveJobAd() ;
		/** returns the type of the job _jobType
		*@return the type of the job
		*@see jType
		*/
		jType getType ()  {  return jobType ; };
	//@}
	/**@name Job Special Action Methods */
	//@{
		/**
		* Allow to retrieve back the state of a Job in the specified step (default value is last reached step, 1 means lust but one ect etc...)
		*  @param  step an positive integer number representing the JobState step we want to retrieve:
		* step= 0 (default):  last State of the job. step=1: last but one. step=2: last but two... etc etc
		* @return the statue of the job in the specified step   */
		glite::wms::checkpointing::JobState getState(unsigned int step = 0) ;
		/** Attach the job to a new listener (if possible) and log new information to LB
		* @param  port the local machine port to be forced as listener
		* @param ls A pointer to a Listener interface implementation which manages input/output/error streams
		* @return the pid corresponding to the launched listener process id (in a new window)
		* @exception JobOperationException The Operation required is not allowed for the Job  */
		int attach ( Listener* ls, int port = 0)  ;
	//@}
	/**@name LB retrieve info Methods */
	//@{
		/** Retrieve the status of the job
		* @param ad if set to false only basic info are retrieved
		* @return the status of the job
		* @see  glite::lb::JobStatus class documentation */
		Status getStatus(bool ad = true)  ;
		/**  Retrieve the bookkeeping information of the job
		* @return all the events logged during the job life
		* @see  glite::lb::Event class documentation */
		Events getLogInfo() ;
	//@}
	/**@name Job NS require operation Methods */
	//@{
		/**
		*Submit the job to the Network Server
		* @param  nsHost The Network Server host name
		* @param  nsPort The Network Server port value
		* @param  lbHost The LB host name
		* @param  lbPort The LB port value
		* @param ce_id the specific atddress of the resource where the job has to be executed */
		void submit(const std::string& nSHost , int nsPort , const std::string& lbHost , int lbPort ,const std::string& ce_id= "" )  ;
		/**
		* Submit the job to the Network Server starting from an intermediate step specified in the JobState
		* @param state the step where to begin the submission (only for Checkpointable jobs)
		* @param ls a Listener implementation which will perform the job interaction (only for Interactive jobs)
		* @param  nsHost The Network Server host name
		* @param  nsPort The Network Server port value
		* @param  lbHost The LB host name
		* @param ce_id the specific atddress of the resource where the job has to be executed		
		* @param  lbPort The LB port value */
		void submit (const std::string& nsHost, int nsPort, const std::string& lbHost , int lbPort ,
			glite::wms::checkpointing::JobState *state , Listener *ls , const std::string& ce_id="" )  ;
		/**Look for matching Computing Element available resources
		* @param  host The Network server host name
		* @param port the Network server port number
		* @return all the Computing Elemets that match with the given JDL togheter with their rank */
		std::vector<std::pair <std::string , double> > listMatchingCE(const std::string& host , int port) ;
		/**Cancel the job from the Network Server
		* @return  The Result of the operation*/
		glite::wms::common::utilities::ResultCode cancel ( ) ;  //Result code?

		/**Retrieve output files of a submitted job ( Success Done status has to be reached)
		* @param dir_path the path where to copy the OutputSandbox files (throws exception if it does not exist)*/
		void getOutput(const std::string& dir_path);

	//@}

	/**@name Static Methods */
	//@{
		/**
		* Submits and get the output of a simple Job once it's ready.
		* once the job has been submitted, a cycle of status retrieval is done untill
		* the status code is reached. Then the output files are retrieved and stored in the specified output directory
		* @return  The JobId pointer to the submitted job
		* @param  nsHost The Network Server host name
		* @param  nsPort The Network Server port value
		* @param  lbHost The LB host name
		* @param  lbPort The LB port value
		* @param  executable The value of the Executable JDL attribute
		* @param  stdOutput  The value of the standard Output JDL attribute
		* @param stdErr  The value of the standard Error JDL attribute
		* @param  ce_id The Computing Element Identificator where to perform the jo
		* @param outputDir the directory where to retrieve the output files from the job once it is ready
		* @param time_interval amount of time to wait before perform next check
		* @param timeout lentgh of status cycle retrieval. Each 30 seconds a getStatus is called for timeout times */
		static glite::wmsutils::jobid::JobId* submit(
					const std::string& nsHost , int nsPort ,
					const std::string& lbHost , int lbPort,
					const std::string& executable ,
					const std::string& stdOutput ,
					const std::string& stdErr ,
					const std::string& outputDir= "/tmp",
					const std::string& ce_id = "",
					int timeout =  10,
					int time_interval = 60) ;
	//@}

	private:
		/** Perform LB   dgLogListener*/
		void shadow     (char*  shHost , int shPort )     ;
		/** Perform NS initalisation*/
		void lbInit   (const std::string& nsHost)     ;
		/** Perform NS initalisation*/
		void nsInit   (const std::string& host, int port)     ;
		/** Retrieve resources matching*/
		void nsList   (std::vector<std::pair <std::string , double> > &resources)   ;
		/** Submit the job*/
		void nsSubmit (const std::string&  lb_addr  )  ;
		/** Retrieve job output files*/
		void nsOutput ( const std::string& dir )   ;
		jType jobType;
		static pthread_mutex_t dgtransfer_mutex;
		/* Internal JobId instance pointer*/
		glite::wmsutils::jobid::JobId* jid;
		/*Internal JobAd instance pointer  */
		glite::wms::jdl::JobAd* jad;
		/*Stores the path of the proxy (if different from the default)*/
		std::string cred_path  ;
		/* Allow perform interactive Jobs */
		Shadow sh ;
		/*Allow to perform NS operation    */
		glite::wms::manager::ns::client::NSClient *nsClient ;
		/*Allow to perform LB operation*/
		edg_wll_Context ctx ;
		/* To Check if the credential are OK */
		UserCredential userCred ;
		/* JobCollection managment*/
		bool jCollect ;
		unsigned int loggerLevel ;
		// NS host:port
		std::string nsHost ;
		//LB contact:
		std::string lbHost ; // keep the lb host retrieved from NS
		int lbPort ,nsPort; // keep the lb port retrieved from NS
		void setCollect()  {   jCollect = true ;  }
		/**JobCollection has full access to all Job private members */
		friend class JobCollection ;
};

} // api
} // wmsui
} // glite

#endif


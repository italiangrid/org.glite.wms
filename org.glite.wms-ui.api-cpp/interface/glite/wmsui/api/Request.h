#ifndef  GLITE_WMS_UI_CLIENT_REQUEST_H
#define GLITE_WMS_UI_CLIENT_REQUEST_H
/*
 * Request.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */
 // DagAd
#include "glite/wms/jdl/ExpDagAd.h"
#include <map>
#include <vector>
// JobId
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/common/utilities/result_codes.h"
// Credential Information
#include "glite/wmsui/api/UserCredential.h"
#include "Logging.h"

/************************Logging and Bookkeeping  *******************/
#include "glite/lb/consumer.h"
#include "glite/lb/JobStatus.h"
#include "glite/lb/Event.h"

#define GLITE_LB_LOG_DESTINATION "EDG_WL_LOG_DESTINATION"
// Namespace definition:
namespace glite {

namespace  wms {
namespace  manager {
namespace ns {
namespace client { 
class NSClient ;  
} 
}
}
}

namespace wmsui {
namespace api {

/**
 * Allow controlling the Dag
 * The Job class provides methods that allow controlling the job during its lifetime.
 * It currently encompasses routines for cancelling a job and retrieving its output,
 * but if needed it will be extended to provide other features such as job checkpointing, holding, releasing etc.
 *
 * @brief Allow creating the job and controlling it during its lifetime.
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/

class Request{
   public:
	/**@name Constructors/Destructor */
	//@{
	/** Instantiates an  empty  Job object */
	Request();
	/** Instantiates an  Job object with a JobId
	* @exception JobOperationException If the JobId is empty  */
	Request(const glite::wmsutils::jobid::JobId& id);
	/** Instantiates an  Job object with a ExpDagAd
	* @exception JobOperationException If the DagAd is empty  */
	Request(const glite::wms::jdl::ExpDagAd& ad);
	/** Instantiates an  Job object with a JobAd
	* @exception JobOperationException If the JobAd is empty  */
	Request(const glite::wms::jdl::JobAd& ad);
	/** Copy constructor*/
	Request(const Request& Request) ;
	/** destructor*/
	virtual ~Request() throw();
	/**Assignment operator*/
	void operator=(const Request& dag );
	//@}

	/**@name Get/Set Methods */
	//@{
	/**Set a different  Proxy certificate from the default one
	* @param cp The full path of the proxy certificate file to be set*/
	void setCredPath(const std::string cp) ;
	/**Set the Proxy certificate as default*/
	void unsetCredPath() ;
	/** Se the verbosity level for NS debug
	*  default value = 0 (no verbosity)
	* max value = 6 (dreadful verbosity, print screen) */
	void setLoggerLevel ( unsigned int level )  { loggerLevel = level ; }
	/** set  the JobAd instance
	* @param ad the JobAd Instance to set  */
	void setJobAd(const glite::wms::jdl::JobAd& ad);
	/** set  the JobAd instance
	* @param ad the JobAd Instance to set  */
	void setDagAd(const glite::wms::jdl::ExpDagAd& ad);
	/** set  the JobId instance
	* @param id the JobId Instance to set*/
	void setDagId(const glite::wmsutils::jobid::JobId& id);
	//@}

	/**@name Job Action Methods */
	//@{

	/** Retrieve the status of the job
	* @param ad if set to false only basic info are retrieved
	* @return the status of the requested component
	* @see  glite::lb::JobStatus class documentation */
	glite::lb::JobStatus getStatus(  bool ad = true )  ;
	/**  Retrieve the bookkeeping information of the job
	* @return all the events logged during the job life
	* @see  glite::lb::Event class documentation */
	std::vector <glite::lb::Event> getLogInfo() ;
	// std::vector <glite::lb::Event> getLogInfo( std::vector < std::pair <std::string , std::string> > userTags, bool owner = true, bool ad = true) ; 



	/**
	*Submit the job to the Network Server
	* @param  ns_host The Network Server host address
	* @param  ns_port The Network Server port
	* @param  lb_host The LB Server host address
	* @param  lb_port The LB Server port
	* @return the JobId representing the submitted job
	*/
	glite::wmsutils::jobid::JobId submit(const std::string& host , int port , const std::string& lbHost , int lbPort, const std::string& ceid="" )  ;
	/**
	* Look for matching resources
	* @param  ns_host The Network Server host address
	* @param  ns_port The Network Server port
	* @return the Computing elements that match with the specified JDL  */
	std::vector<std::string> listMatchingCE(const std::string& host , int port );
	/**Cancel the job from the Network Server
	* @return  The Result of the operation
	* @exception JobOperationException The Operation required is not allowed for the Job
	* @see exception returned from NS  */
	void cancel ( ) ;
	/**Retrieve output files of a submitted job
	* @param dir_path the path where to retrieve the OutputSandbox files
	* @exception JobOperationException The Operation required is not allowed for the Job
	* @see exception returned from NS  */
	void getOutput(const std::string& dir_path);
	//@}
private:
	enum dagType {
		/** Non type. The job has been initilised with the default ctor*/
		EWU_TYPE_NONE,
		/** Ad type. The job has been initialised with the ExpDagAd ctor*/
		EWU_TYPE_DAG_AD,
		/** Partitioning type. The job has been initialised with the JOb Partitioning ctor*/
		EWU_TYPE_JOB_AD,
		/** Id type. The job has been initialised with the JobId*/
		EWU_TYPE_ID,
		/** Submitted type. The job has been initialised with the JobAd ctor and then submitted*/
		EWU_TYPE_SUBMITTED
	} ;
	// Private methods:
	void submit ( ) ;
	void regist() ;
	void getOutput(const std::string& dir_path , const std::string& nsRootPath, const  glite::lb::JobStatus& status );
	// Private Members:
	glite::wmsutils::jobid::JobId* jid;
	/*Internal DAGAd instance pointer  */
	glite::wms::jdl::ExpDagAd* dag;
	/*Internal JobAd instance pointer  */
	glite::wms::jdl::JobAd* jad;
	/*Stores the path of the proxy (if different from the default)*/
	// std::string cred_path  ;
	/* To Check if the credential are OK */
	UserCredential userCred ;
	Logging log ;
	glite::wms::manager::ns::client::NSClient * nsClient ;
	/* JobCollection managment*/
	unsigned int loggerLevel ;

/*  TBD try to decomment those lines...*/
		// std::string nsHost , lbHost ;
		// int lbPort ,nsPort;
/*  TBD try to decomment those lines...*/
	dagType type ;
};

} // api
} // wmsui
} // glite

#endif


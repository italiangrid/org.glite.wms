#ifndef  ORG_GLITE_WMSUI_WRAPY_LBWRAPPER_H
#define ORG_GLITE_WMSUI_WRAPY_LBWRAPPER_H
/*
 * LbWrapper.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */
#include "glite/lb/JobStatus.h"
#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include <list>


/**
 * Provide a wrapper for the glite::wms::lb::JobStatus class
 * it allows the user to retrieve bookkeeping information over a sequence of jobs as well as to
 * perform a query on the LB server and get back all the jobs that match
 *
 * @brief LB JobStatus wrapper class
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/

class Status {
  public:
	/**
	* Status instance constructor
	*/
	Status () ;
	/**
	* Status instance destructor
	*/
	~Status () ;
	/**
	* retrieve the number of states downloaded so far for the current instance
	*/
	int size () ;
	/**
	* This method is DEPRECATED
	*/
	int size (int status_number) ;
	/**
	* retrieve the status for the specified jobid
	*@param jobid the id of the job whose info are to be retrieved
	*@param level a number in the interval {0,1,2,3} specifying the verbosity of the LB info to be downloaded
	*/
	int getStatus (const std::string& jobid, int level = 0) ;
	/**
	* retrieve the states of all the user Job
	*@param host the LB server host name
	*@param port the LB server listening port
	*@param level a number in the interval {0,1,2,3} specifying the verbosity of the LB info to be downloaded
	*/
	int getStates (const std::string& host , int port , int level = 0) ;
	/** Retrieve the attribute value for the specified JobStatus
	* @param field the index of the field as in Job.py JobStatus class
	* @param status_number the status number to be taken into account
	* @param attrValue parameter passed by reference , returned by python
	* @return a couple of strings [ <attribute name > , <attribute value> ]
	*
	*/
	std::string getVal (int field, std::string& attrValue, int status_number = 0);
	/**Set an attribute value (of string type)
	* @param err the string description passed by reference. Python will consider it as a returning parameter
	* @return a couple of [ int , string ] representing the error code (0 for success) and the error string representation
	*/     
	int get_error (std::string& err) ;
	/**
	* retrieve a Job Status from the downloaded ones
	*@param status_number : the status to be retrieved
	*@return a vector of strings representing all the Status intance values as specified in Job.py, JobStatus class
	*/
	std::vector< std::string  > loadStatus( int status_number= 0) ;
	/**
	* Perform a query to the Lb Server allowing the user to specify several parameters
	*@param host the LB server host name
	*@param port the LB server listening port
	*@param tagNames a vector containing all the user tag names to be searched
	*@param tagValues a vector containing all the user tag values for the tagNames names. the size of tagNames must be the same as tagValues
	*@param excludes an integer representing all the states that do not have to be retrieved (used togheter with includes returns empty set)
	*@param includes an integer representing only the states that have to be retrieved (used togheter with excludes returns empty set)
	*@param issuer retrieves only job belonging to the specified proxy certificate issuer
	*@param from retrieves only job submitted after specified time ( in seconds after epoch)
	*@param to retrieves only job submitted before specified time ( in seconds after epoch)
	*@param ad if different from 0, retrieves Ad Status information as well
	*/
	int queryStates (const std::string& host , int port ,
	const std::vector<std::string>& tagNames,const std::vector<std::string>& tagValues,
	const std::vector<int>& excludes,const std::vector<int>& includes,
	std::string issuer,int from , int to ,int ad );
  private:
	std::list<glite::wms::lb::JobStatus> states ;
	std::string error ;
	bool error_code ;
	void log_error ( const std::string& err) ;
};
/**
 * Provide a wrapper for the glite::wms::lb::Event class
 * it allows the user to retrieve logging information over one or more jobs
 *
 * @brief LB Event wrapper class
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class Eve{
  public:
	/**
	* Event Wrapper class Constructor
	*/
	Eve () ;
	/**
	* Event Wrapper class Destructor
	*/
	~Eve();
	/**
	* Return the number of events retrieved
	*/
	int size () ;
	/**
	* This method is DEPRECATED
	*/
	int size (int status_number) ;
	/**
	* retrieve the events for the specified jobid
	*@param jobid the id of the job whose info are to be retrieved
	*/
	int getEvents ( const std::string& jobid) ;
	/** Retrieve the attribute value for the specified event
	* @param field the index of the field as in Job.py Event class
	* @param event_number the event where to retrieve the value from
	* @param attrValue parameter passed by reference , returned by python
	* @return a couple of strings [ <attribute name > , <attribute value> ]
	*/
	std::string getVal (int field , std::string& attrValue , int event_number);
	/**Set an attribute value (of string type)
	* @param err the string description passed by reference. Python will consider it as a returning parameter
	* @return a couple of [ int , string ] representing the error code (0 for success) and the error string representation
	*/
	int get_error (std::string& err) ;
  private:
	std::list<glite::wms::lb::Event> events ;
	std::string error;
	bool error_code ;
	void log_error ( const std::string& err) ;
};

#endif

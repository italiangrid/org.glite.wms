#ifndef  GLITE_WMS_UI_CLIENT_JOBEXCEPTIONS_H
#define GLITE_WMS_UI_CLIENT_JOBEXCEPTIONS_H
/*
 * JobExceptions.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */
#include "glite/wms/common/utilities/Exceptions.h"
#include "glite/wms/ui/client/exception_codes.h"

USERINTERFACE_NAMESPACE_BEGIN //Defining UserInterFace NameSpace
/**
*  JobCollectionException
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
/**
* Update all mandatory Exception Information*/
class JobCollectionException : public glite::wms::common::utilities::Exception {
public:
   /**
   * Update all mandatory Exception Information*/
   JobCollectionException  (const std::string& file,
			     int line,
			     const std::string& method,
			     int code,
			     const std::string& exception_name);
// protected:
   // std::string          error_description ;
};//End Class JobCollectionException

/**
 * Thrown when an error is found while inserting/deleting jobs
*/
class JobCollectNoJobException : public  JobCollectionException {
public:
/**
* Update all mandatory Exception Information
*/
JobCollectNoJobException(const std::string& file,
			     int line,
			     const std::string& method,
			     int code,
			     const std::string& job= "");
};//End CLass  JobCollectNoJobException
/**
 * Job Exception could be raised from
 * Broker  Error
 * Logging Error
*/
class JobException : public glite::wms::common::utilities::Exception {
/**
* Update all mandatory Exception Information
*/

protected:
JobException(const std::string& file,
		 int line,
		 const std::string& method,
		 int code,
		 const std::string& exception_name);
};//End CLass  JobException



/**
* Operation timeout raised
*/
class JobTimeoutException : public JobException {
public:
    /**
     * Update all mandatory Exception Information
     */
    JobTimeoutException(const std::string& file,
			int line,
			const std::string& method,
			int code);
};//End Class    JobTimeoutException

/**
* Operation not admitted
*/
class JobOperationException : public JobException {
public:
    /**
     * Update all mandatory Exception Information
     */
    JobOperationException(const std::string& file,
			  int line,
			  const std::string& method,
			  int code,
			  const std::string& reason);
};//End CLass JobOperationException

// EWC_END_NAMESPACE; //Close the NameSpace
USERINTERFACE_NAMESPACE_END } //Closing  UserInterFace NameSpace
#endif

/* **************************************************************************
*  filename  : JobExceptions.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include "glite/wms-ui/api/JobExceptions.h"

USERINTERFACE_NAMESPACE_BEGIN //Defining UserInterFace NameSpace
using namespace std ;
using namespace glite::wmsustils::exception ;

JobCollectionException::JobCollectionException  (const std::string& file,
			     int line,
			     const std::string& method,
			     int code,
			     const std::string& exception_name)
	:Exception (file,line,method,code ,exception_name )  {};
JobCollectNoJobException::JobCollectNoJobException(const std::string& file,
			     int line,
			     const std::string& method,
			     int code,
			     const std::string& job)
	: JobCollectionException(file, line, method, code,
				 "JobCollectNoJobException" ){
	switch (code){
	case  WL_NOSUCHJOB : //Deprecated Code
	    error_message = "No such job found, unable to cancel"  ;
	    break;
	case  ENOENT :
	    error_message = "No such job found, unable to cancel"  ;
	    break;
	case  WL_DUPLICATE_JOB:
	    error_message = "Duplicate JobId value, unable to insert";
	    break;
	default:    //WL_DUPLICATE_JOB
	    error_message = "Duplicate JobId value, unable to insert";
	    break;
	}
	if (job!= "")
	    error_message+= " Job : " + job ;
}

JobException::JobException(const std::string& file,
		 int line,
		 const std::string& method,
		 int code,
		 const std::string& exception_name)
	: Exception(file, line, method, code, exception_name){ }

JobTimeoutException::JobTimeoutException(const std::string& file,
			int line,
			const std::string& method,
			int code)
	: JobException(file, line, method, code,
		       "JobTimeoutException")
{
	error_message = "Submit notification timeout expired";
}

JobOperationException::JobOperationException(const std::string& file,
			  int line,
			  const std::string& method,
			  int code,
			  const std::string& reason)
	:JobException(file, line, method, code,
		 "JobOperationException")
    {
	error_message = "The Operation is not allowed: " +reason;
	error_message += reason;
}

//EWC_END_NAMESPACE; //Close the NameSpace
USERINTERFACE_NAMESPACE_END } //Closing  UserInterFace NameSpace


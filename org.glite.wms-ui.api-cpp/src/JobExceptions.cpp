/* **************************************************************************
*  filename  : JobExceptions.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include "glite/wmsui/api/JobExceptions.h"

namespace glite {
namespace wmsui {
namespace api {

using namespace std ;
using namespace glite::wmsutils::exception ;

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
	case  WMS_NOSUCHJOB : //Deprecated Code
	    error_message = "No such job found, unable to cancel"  ;
	    break;
	case  ENOENT :
	    error_message = "No such job found, unable to cancel"  ;
	    break;
	case  WMS_DUPLICATE_JOB:
	    error_message = "Duplicate JobId value, unable to insert";
	    break;
	default:    //WMS_DUPLICATE_JOB
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

} // api
} // wmsui
} // glite

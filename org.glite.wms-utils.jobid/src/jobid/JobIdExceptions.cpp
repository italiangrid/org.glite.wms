/* **************************************************************************
*  filename  : JobIdExecptions.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/

#include "JobIdExceptions.h"

namespace glite {
namespace wms {
namespace jobid {

using namespace std;
using namespace glite::wms::common::utilities;

/*****************************
* JobIdException
*****************************/
JobIdException::JobIdException (const string& file,
				    int line,
				    const string& method,
				    int code,
				    const string& exception_name)
	: Exception(file, line, method, code, exception_name)
{
}

/*****************************
* WrongIdException
*****************************/
WrongIdException::WrongIdException(const string& file,
				       int line,
				       const string& method,
				       int code )
	: JobIdException(file, line, method, code,
			 "WrongIdException")
{
	error_message = "Wrong Field caught while parsing Job Id" ;
}

/*****************************
* EmptyIdException
*****************************/
EmptyIdException::EmptyIdException(const string& file,
				       int line,
				       const string& method,
				       int code ,
				       const string& field )
	: JobIdException(file, line, method, code,
			 "EmptyIdException")
{
	error_message = "Unable to retrieve " + field + ": the instance has not been initialized yet";
}

} // namespace jobid
} // namespace wms
} // namespace glite


/* **************************************************************************
*  filename  : Exceptions.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/

#include "glite/wmsutils/exception/Exceptions.h"
#include "glite/wmsutils/exception/exception_codes.h"
namespace glite { 
namespace wmsutils{ 
namespace exception {
pthread_mutex_t METHOD_MUTEX  ;  // This mutex is used in order to lock the file for writing log infornation
/* *********************************
* Exception Class Implementation
************************************/
Exception::Exception () {
	stack= "";
	line = 0;
} ;
Exception::~Exception() throw(){ }
/**
* Exception chainig
*/
Exception::Exception (  const string& source,   const string& method,    Exception *e){
	source_file = source ;
	method_name    = method;
	error_message = "";
	stack= e->printStackTrace();
	ancestor = e->what() ;
	line = 0;
	error_code= 0;
};
Exception::Exception( const std::string& file,  int line_number,    
	const std::string& method,   int code,  const std::string& name): error_code(code), exception_name(name){
		source_file    = file;
		line           = line_number;
		method_name    = method;
		stack= "";
};
Exception::Exception (const string& source,
	const string& method,
	int code,
	const string& exception)
	: error_code(code), exception_name(exception){
		source_file    = source;
		method_name    = method;
		stack= "";
		line = 0;
};

int Exception::getCode(){
	if  (error_code != 0) return error_code ;
	else return WL_COMMON_BASE;
};

const char* Exception::what() const throw(){
	if (!ancestor.empty() ) return ancestor.c_str() ;
	if  ( error_message != "") return error_message.c_str() ;
	else return "" ;
};

string Exception::getExceptionName(){
	if  (exception_name!= "") return exception_name;
	else return "" ;
};
void Exception::log(const std::string& logfile){
	if (  logfile == "") syslog (  LOG_PERROR,   (char *)  (dbgMessage()).c_str()  );
	else{
		pthread_mutex_lock( &METHOD_MUTEX);                   //   LOCK
		ofstream fout ((char *)   logfile.c_str()  , ios::app );  //Open the file for writing (if it doesn't exist then it will be created)
		fout << "\n" << dbgMessage() ;  //write (append) the message
		fout.close();  //close the file
		pthread_mutex_unlock( &METHOD_MUTEX);               //   UNLOCK
	}
};
string Exception::printStackTrace(){
	return stack + "\n" + dbgMessage();
};

string Exception::dbgMessage(){
	string result ;
	//Adding exception Name
	if ( exception_name!="") result = exception_name ;
	//Adding error msg
	if (error_message!="") result +=": " + string(what());
	// Appending newline if needed
	if (result != "") result+="\n";
	//Adding  Source
	result +="         at " + source_file ;
	//Adding line number
	if (line!=0){
		char buffer [1024] ;
		sprintf (buffer, "%i" ,  line) ;
		result += " Line: " + string ( buffer );
	}
	result +=" " ;
	//Adding Method Name
	if (method_name != "") result += "Method: " +method_name ;
	return result;
}

}}}  // Closing namespace


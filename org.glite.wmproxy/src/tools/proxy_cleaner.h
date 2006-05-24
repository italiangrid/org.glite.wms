
#ifndef GLITE_WMS_WMPROXY_TOOLS_CONVERTER_H
#define GLITE_WMS_WMPROXY_TOOLS_CONVERTER_H

#include "log.h"

#include <openssl/pem.h>

#include <string>
#include <vector>

extern "C" {
#include <getopt.h>
}

namespace glite {
namespace wms {
namespace wmproxy {
namespace tools {

struct cleanerOpts {
	std::string *proxycache ;
	std::string* log;
	bool debug ;
	bool version ;
	bool help;
};

class ProxyCleaner {
        public:

		/**
		* Default constructor
		*/
		ProxyCleaner();

		/**
		* Default destructor
		*/
		~ProxyCleaner () ;
		/**
		* Returns a message with the version of this tool
		*/
		const std::string version() ;
		/**
		* Prints on the standard error
		* a message with the usage instructions for this tool
		*/
		void ProxyCleaner::usage(char *exe);

		/**
		* Creates the log file to the specified path
		*@param path the location of the log file
		*/
		Log* createLogFile (const std::string &path);
		/**
		* Creates the log file to the default-path
		*@param path the location of the log file
		*/
		Log* createLogFile ( );


		std::string removeExpiredProxies(std::string& proxycache);
		std::string getResultMsg (std::string &msg) ;

		cleanerOpts* getUserOptions( ) ;
		/**
		* Reads the input options
		* @param arg the number of input string
		* @param opts this method put the read values of the user options in this struct
		* @param msg at the end of the execution this string contains the list of the read options
		* @param errors the description of any error occurred during the execution
		* @return a negative value in case of any error occured,
		* ("errors" contains the description of the error), 0 otherwise
		*/
		const int readOptions(int argc,char **argv, cleanerOpts &opts, std::string &msg, std::string &errors);
		/**
		* Handles the errors occuring during the excution: prints the error messages
		* and stop the execution of the program
		* @param msg the string with error message
		* @param desc the string with some details about the error
		*/
		void error (const std::string &msg, const std::string &details="") ;
	private:
		/**
		* Checks whether the input string corresponds to one of the user options
		* @param opt the name of the input option to be checked
		* @return a negative value if the inputs string doesn't match any user option strings
		*/
		const int checkOpts(const std::string& opt) ;
		int listFiles (const std::string &path, std::vector<std::string>& v, std::string& err) ;

		std::string twoDigits(unsigned int d) ;

		std::string getTimeStamp() ;

		time_t ASN1_UTCTIME_get(const ASN1_UTCTIME *s) ;

		const long get_time_left(const std::string &file);

		/**
		* Pointer to the log file
		*/
		Log* logFile;
		/**
		* User Options Handlers
		*/
		cleanerOpts usrOpts;
		char* shortOpts  ;
		static const struct option longOpts[];
};
}}}} // ending namespaces
#endif

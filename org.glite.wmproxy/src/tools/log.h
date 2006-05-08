/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Author:	Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#ifndef GLITE_WMS_WMPROXY_TOOLS_LOG_H
#define GLITE_WMS_WMPROXY_TOOLS_LOG_H


/*
 * logman.h
 */
#include <string>
#include <iomanip>
#define TIMESTAMP_FILLING std::setw(2) << std::setfill('0')

namespace glite {
namespace wms {
namespace wmproxy {
namespace tools {


enum LogLevel{
	WMSLOG_UNDEF,
	WMSLOG_DEBUG,
	WMSLOG_INFO,
	WMSLOG_WARNING,
	WMSLOG_ERROR,
	WMSLOG_FATAL,
	WMSLOG_NOMSG
};
enum severity{
	WMS_UNDEF,
	WMS_DEBUG,
	WMS_INFO,
	WMS_WARNING,
	WMS_ERROR,
	WMS_FATAL
};


enum errCode{
	DEFAULT_ERR_CODE
};

 enum WmcStdStream {WMC_OUT, WMC_ERR};

class Log {
        public:// LOG LEVEL

                /*
                * default constructor (if no pathname is specified as input for the logfile, default file is <applName>_<UID>_<PID>_<timestamp>.log )
		* @param path the log file pathname
                * @param verbosity level
                */	

                Log(std::string path, LogLevel level = WMSLOG_WARNING);
                /*
		* Default destructor
		*/
		~Log( ) ;
                /*
		* prints a formatted error description of the input exception
                * @param sev severity of the message (WMS_NONE, WMS_INFO, WMS_WARNING, WMS_ERROR, WMS_FATAL)
                *@param title the title of the message
                *@param msg the message string
                * @param debug flag indicating whether the formatted message have to be printed on the std-output
                */
                void print (severity sev, const std::string &header, const std::string &msg, const bool debug=true);
		/*
                * get the absolute pathname of the log file
                *@return the pathname string
                */
                std::string* getPathName ( );


	private :

		/*
		* gets a formatted error message text according to the input information;
		* it can be printed on the std-output/error (see "debug" input parameter) and/or into a file as well (see "path" input parameter)
		* Messages which severity are "none" and "info" are printed on the std-output; the other ones on the std-error (see "sev" input parameter)
		* @param sev severity of the message (WMS_NONE, WMS_INFO, WMS_WARNING, WMS_ERROR, WMS_FATAL)
		* @param header header text
		* @param err the error message
		* @param exc the exception
		* @param debug flag indicating whether the messages have to be printed on the std-output
		* @param path the pathname of the file
		* @return the formatted message
		*/
		void errMsg(severity sev, const std::string &header, const std::string& err, const bool &debug, std::string* path);
        	/**
                * log-file pathname
                */
                std::string* logFile ;
                /**
                * debugging messages on the std-output
                */
		LogLevel dbgLevel ;
};

}}}} // ending namespaces
#endif //GLITE_WMS_WMPROXY_TOOLS_LOG_H

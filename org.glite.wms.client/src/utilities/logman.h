/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#ifndef GLITE_WMS_CLIENT_LOGMAN_H
#define GLITE_WMS_CLIENT_LOGMAN_H
/*
 * logman.h
 */
#include "excman.h"

namespace glite{
namespace wms{
namespace client{
namespace utilities{

// LOG LEVEL
enum LogLevel{
	WMSLOG_UNDEF,
        WMSLOG_DEBUG,
        WMSLOG_INFO,
        WMSLOG_WARNING,
        WMSLOG_ERROR,
        WMSLOG_FATAL,
        WMSLOG_NOMSG
};

class Log {
        public:
                /*
                * default constructor (if no pathname is specified as input for the logfile, default file is <applName>_<UID>_<PID>_<timestamp>.log )
		* @param path the log file pathname
                * @param verbosity level
                */
                Log(std::string* path, LogLevel level = WMSLOG_WARNING);
                /*
		* Default destructor
		*/
		~Log( ) ;

		void createLogFile(const std::string &path);
                /*
                * prints the exception messages
                * @param exc the exception
                * @param debug flag indicating whether the formatted message have to be printed on the std-output
                */
                void print (severity sev, const std::string &header,glite::wmsutils::exception::Exception &exc, const bool debug=true, const bool cache=false);
                /*
		* prints a formatted error description of the input exception
                * @param sev severity of the message (WMS_NONE, WMS_INFO, WMS_WARNING, WMS_ERROR, WMS_FATAL)
                *@param title the title of the message
                *@param msg the message string
                * @param debug flag indicating whether the formatted message have to be printed on the std-output
                */
                void print (severity sev, const std::string &header, const std::string &msg, const bool debug=true, const bool cache=false);
		/*
                * get the absolute pathname of the log file
                *@return the pathname string
                */
                std::string* getPathName ( );


	private :
        	/**
                * log-file pathname
                */
                std::string* logFile ;
        	/**
                * log-cache
                */
		std::string logCache;
                /**
                * debugging messages on the std-output
                */
		glite::wms::client::utilities::LogLevel dbgLevel ;
};

}}}} // ending namespaces
#endif

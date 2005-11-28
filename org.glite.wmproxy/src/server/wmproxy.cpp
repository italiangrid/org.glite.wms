/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmproxy.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <string>
#include <vector>
#include <iostream>

// Fast CGI
#include <fcgi_stdio.h>
#include "fcgio.h"
#include "fcgi_config.h"

// gSOAP
#include "WMProxy.nsmap"
#include "soapWMProxyObject.h"

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/configuration/WMPConfiguration.h"

// fstream
#include "glite/wms/common/utilities/FileList.h"

#include "wmpconfiguration.h"
#include "utilities/wmputils.h"

#include "wmpgsoapfaultmanipulator.h"

namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace wmputilities  = glite::wms::wmproxy::utilities;

using namespace std;
using namespace boost::details::pool;

//namespace glite {
//namespace wms {
//namespace wmproxy {

const std::string opt_conf_file("glite_wms.conf");

// Global variables for configuration attributes
std::string sandboxdir_global;

void
sendFault(WMProxy &proxy, const string &method, const string &msg, int code)
{
	setSOAPFault(&proxy, code, method, time(NULL), code, msg);
	soap_print_fault(&proxy, stderr);
	soap_send_fault(&proxy); 
	soap_destroy(&proxy);
	soap_end(&proxy); 
	soap_done(&proxy);
}

int
main(int argc, char* argv[])
{
	try {
		// Debug only
		//wmputilities::waitForSeconds(10);
					
		std::fstream edglog_stream;
		singleton_default<WMProxyConfiguration>::instance().init(opt_conf_file,
        	configuration::ModuleType::workload_manager_proxy);
        string log_file = singleton_default<WMProxyConfiguration>
			::instance().wmp_config->log_file();

		// Checking for log file
		if (!log_file.empty()) {
			if (!std::ifstream(log_file.c_str())) {
		    	std::ofstream(log_file.c_str());
		    }
			edglog_stream.open(log_file.c_str(), std::ios::in | std::ios::out
				| std::ios::ate);
		}
		if (edglog_stream) {
			logger::threadsafe::edglog.open(edglog_stream,
				static_cast<logger::level_t>(singleton_default<WMProxyConfiguration>
				::instance().wmp_config->log_level()));
		}
		
		edglog_fn("wmproxy::main");
		edglog(info)
			<<"------- Starting Server Instance -------"
			<<endl;
		
		// Opening log file destination
		logger::threadsafe::edglog.activate_log_rotation (
			singleton_default<WMProxyConfiguration>::instance()
				.wmp_config->log_file_max_size(),
			singleton_default<WMProxyConfiguration>::instance()
				.wmp_config->log_rotation_base_file(),
			singleton_default<WMProxyConfiguration>::instance()
				.wmp_config->log_rotation_max_file_number());
		edglog(debug)<<"Log file: "<<singleton_default<WMProxyConfiguration>::instance()
			.wmp_config->log_rotation_base_file()<<endl;
		
		// Initializing signal handler for 'graceful' stop/restart
		//wmputilities::initsignalhandler();
		
		extern string sandboxdir_global;
		sandboxdir_global = "";
		
		// Running as a Fast CGI application
		edglog(info)<<"Running as a FastCGI program"<<endl;
		edglog(info)<<"Entering the FastCGI accept loop..."<<endl;
		edglog(info)
			<<"----------------------------------------"
			<<endl;
		
		for (;;) {
			WMProxy proxy;
			proxy.serve();
		}
		edglog(info)<<"Exiting the FastCGI loop..."<<endl;
		
    } catch (configuration::CannotOpenFile &file) {
    	string msg = "Cannot open file: " + string(file.what());
	    edglog(fatal)<<msg<<std::endl;
	    WMProxy proxy;
	   	sendFault(proxy, "main", msg, -4);
    } catch (configuration::CannotConfigure &error) {
    	string msg = "Cannot configure: " + string(error.what());
	    edglog(fatal)<<msg<<std::endl;
        WMProxy proxy;
	   	sendFault(proxy, "main", msg, -3);
	} catch (exception &ex) {
		string msg = "Exception caught: " + string(ex.what());
	    edglog(fatal)<<msg<<std::endl;
		WMProxy proxy;
	   	sendFault(proxy, "main", msg, -2);
 	} catch (...) {
    	string msg = "Uncaught exception";
	    edglog(fatal)<<msg<<std::endl;
		WMProxy proxy;
	   	sendFault(proxy, "main", msg, -1);
  	}
	return 0;
}

//} // wmproxy
//} // wms
//} // glite


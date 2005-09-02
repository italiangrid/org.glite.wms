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

int
logRemoteHostInfo()
{
	try {
		string msg = "Remote Host IP: ";
		string msg2 = "Remote CLIENT S DN: ";
		string msg3 = "Remote GRST CRED: ";
		edglog(info)
			<<"-------------------------------- Incoming Request "
				"--------------------------------"
			<<endl;
		
		if (getenv("REMOTE_ADDR")) {
			msg += string(getenv("REMOTE_ADDR"));
			if (getenv("REMOTE_PORT")) {
				msg += ":" + string(getenv("REMOTE_PORT"));
			}
		} else {
			msg += "Not Available";
		}
		msg += " - Remote Host Name: ";
		if (getenv("REMOTE_HOST")) {
			msg += string(getenv("REMOTE_HOST"));
		} else {
			msg += "Not Available";
		}
		if (getenv("SSL_CLIENT_S_DN")) {
			msg2 += string(getenv("SSL_CLIENT_S_DN"));
		} else {
			msg2 += "Not Available";
		}
		if (getenv("GRST_CRED_2")) {
			msg3 += string(getenv("GRST_CRED_2"));
		} else {
			msg3 += "Not Available";
		}
		
		edglog(info)<<msg<<endl;
	    edglog(info)<<msg2<<endl;
		edglog(info)<<msg3<<endl;
		edglog(info)
			<<"----------------------------------------"
				"------------------------------------------"
		<<endl;
		
		return 0;
	} catch (exception &ex) {
		edglog(fatal) << "Exception caught: " << ex.what() << endl;
		return -1;
	}
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
		
		// Running as a Fast CGI application
		edglog(info) << "Running as a FastCGI program" << endl;
		edglog(info) << "Entering the FastCGI accept loop..." << endl;
		edglog(info)
			<<"----------------------------------------"
			<<endl;
		while (FCGI_Accept() >= 0) {
			WMProxy proxy;
			if (logRemoteHostInfo()) {
				setSOAPFault(&proxy, -1, "logRemoteHostInfo", time(NULL),
					-1, "Unable to log remote host info\n(please contact server administrator)");
				soap_print_fault(&proxy, stderr);
				soap_send_fault(&proxy); 
				soap_destroy(&proxy);
				soap_end(&proxy); 
				soap_done(&proxy);
				continue; 
			}
			proxy.serve();
		}
		edglog(info) << "Exiting the FastCGI loop..." << endl;
		
    } catch (configuration::CannotOpenFile &file) {
	    edglog(fatal) << "Cannot open file: " << file << std::endl;
    } catch (configuration::CannotConfigure &error) {
        edglog(fatal) << "Cannot configure: " << error << std::endl;
	} catch (exception &ex) {
		edglog(fatal) << "Exception caught: " << ex.what() << endl;
 	} catch (...) {
    	edglog(fatal) << "Uncaught exception...." << endl;
  	}
	return 0;
}

//} // wmproxy
//} // wms
//} // glite


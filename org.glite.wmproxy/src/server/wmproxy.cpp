/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

//
// File: wmproxy.cpp
// Author: Giuseppe Avellino <egee@datamat.it>
//

#include <string>

// Fast CGI
#include <fcgi_stdio.h>
#include "fcgio.h"
#include "fcgi_config.h"

#include <signal.h>  // sig_atomic

// gSOAP
#include "WMProxy.nsmap"
#include "wmproxyserve.h"
#include "soapWMProxyObject.h"

// Boost singleton
#include <boost/pool/detail/singleton.hpp>

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

// Configuration
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/exceptions.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

// File List
#include "glite/wms/common/utilities/FileList.h"

#include "wmpconfiguration.h"
#include "wmpgsoapfaultmanipulator.h"


#include "utilities/wmputils.h" // waitForSeconds()
#include "eventlogger/wmplbselector.h"	// lbselectioninfo struct

// Exceptions
#include "glite/wmsutils/exception/Exception.h"


// Global variable for configuration
WMProxyConfiguration conf;

// Global variable for server instance served request count
long servedrequestcount_global;

// Global variables for configuration attributes (ENV dependant)
std::string sandboxdir_global;
std::string dispatcher_type_global;
std::string filelist_global;
glite::wms::wmproxy::eventlogger::WMPLBSelector lbselector;
bool globusDNS_global;
volatile sig_atomic_t handled_signal_recv ;

namespace logger        = glite::wms::common::logger;
namespace wmsexception  = glite::wmsutils::exception;
namespace wmputilities  = glite::wms::wmproxy::utilities;
namespace eventlogger   = glite::wms::wmproxy::eventlogger;
namespace configuration = glite::wms::common::configuration;

using namespace std;

//namespace glite {
//namespace wms {
//namespace wmproxy {

const string opt_conf_file("glite_wms.conf");

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
		extern WMProxyConfiguration conf;
		conf = boost::details::pool::singleton_default<WMProxyConfiguration>
			::instance();

		fstream edglog_stream;
		conf.init(opt_conf_file,
			configuration::ModuleType::workload_manager_proxy);
		string log_file = conf.wmp_config->log_file();
		// Checking for log file
		if (!log_file.empty()) {
			if (!ifstream(log_file.c_str())) {
		    	ofstream(log_file.c_str());
		    }
			edglog_stream.open(log_file.c_str(), ios::in | ios::out
				| ios::ate);
		}
		if (edglog_stream) {
			logger::threadsafe::edglog.open(edglog_stream,
				static_cast<logger::level_t>(conf.wmp_config->log_level()));
		}
		
		edglog_fn("wmproxy::main");
		edglog(info)
			<<"------- Starting Server Instance -------"
			<<endl;
		
		// Opening log file destination
		logger::threadsafe::edglog.activate_log_rotation (
			conf.wmp_config->log_file_max_size(),
			conf.wmp_config->log_rotation_base_file(),
			conf.wmp_config->log_rotation_max_file_number());
		edglog(debug)<<"Log file: "<<conf.wmp_config->log_rotation_base_file()
			<<endl;

		extern string sandboxdir_global;
		sandboxdir_global = "";
		
		extern string dispatcher_type_global;
		dispatcher_type_global
			= configuration::Configuration::instance()->wm()->dispatcher_type();
		edglog(debug)<<"DispatcherType: "<<dispatcher_type_global<<endl;
		
		extern string filelist_global;
		filelist_global
			= configuration::Configuration::instance()->wm()->input();

		extern eventlogger::WMPLBSelector lbselector;
		lbselector = eventlogger::WMPLBSelector(conf.getLBServerAddressesPorts(),
			conf.getWeightsCachePath(),
			conf.getWeightsCacheValidity(),
			conf.isServiceDiscoveryEnabled(),
			conf.getServiceDiscoveryInfoValidity(),
			conf.getLBServiceDiscoveryType());

		// check Globus Version to determine whether to convert DNS
		extern bool globusDNS_global;
		//globusDNS_global = wmputilities::checkGlobusVersion();
		extern long servedrequestcount_global;
		servedrequestcount_global = 0;

		// Running as a Fast CGI application
		edglog(info)<<"Running as a FastCGI program"<<endl;
		edglog(info)<<"Entering the FastCGI accept loop..."<<endl;
		edglog(info)
			<<"----------------------------------------"
			<<endl;

                openlog("glite_wms_wmproxy_server", LOG_PID || LOG_CONS, LOG_DAEMON);	
	
		for (;;) {
			WMProxyServe proxy;
			proxy.serve();
		}
		
		closelog();
		edglog(info)<<"Exiting the FastCGI loop..."<<endl;
		
    } catch (configuration::CannotOpenFile &file) {
    	string msg = "Cannot open file: " + string(file.what());
	    edglog(fatal)<<msg<<endl;
	    WMProxy proxy;
	   	sendFault(proxy, "main", msg, -5);
    } catch (configuration::CannotConfigure &error) {
    	string msg = "Cannot configure: " + string(error.what());
	    edglog(fatal)<<msg<<endl;
        WMProxy proxy;
	   	sendFault(proxy, "main", msg, -4);
	} catch (wmsexception::Exception &exc) {
		string msg = "Exception caught: " + string(exc.what());
	    edglog(fatal)<<msg<<endl;
		WMProxy proxy;
	   	sendFault(proxy, "main", msg, -3);
	} catch (exception &ex) {
		string msg = "Standard Exception caught: " + string(ex.what());
	    edglog(fatal)<<msg<<endl;
		WMProxy proxy;
	   	sendFault(proxy, "main", msg, -2);
 	} catch (...) {
    	string msg = "Uncaught exception";
	    edglog(fatal)<<msg<<endl;
		WMProxy proxy;
	   	sendFault(proxy, "main", msg, -1);
  	}
	return 0;
}

//} // wmproxy
//} // wms
//} // glite


/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmproxy.h"
// Fast CGI
#include <fcgi_stdio.h>
#include "fcgio.h"
#include "fcgi_config.h"
// gSOAP
#include "WMProxy.nsmap"
#include "soapWMProxyObject.h"
// Logging
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/utilities/edgstrstream.h"
#include "commands/logging.h"

#include "NS2WMProxy.h"
// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
// Exceptions
#include "utilities/wmpexception_codes.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "wmpoperations.h"
#include "wmpdispatcher.h"
#include "wmpconfiguration.h"
#include "utilities/wmputils.h"
#include <iostream>
#include <string>
#include <vector>

namespace utilities  	= glite::wms::common::utilities;
namespace task          = glite::wms::common::task;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace wmproxyserver = glite::wms::wmproxy::server;

using namespace boost::details::pool;
using namespace std;
//namespace glite {
//namespace wms {
//namespace wmproxy {

std::string opt_conf_file("glite_wms.conf");
int
main(int argc, char* argv[])
{
	int m, s;
	struct soap *soap;
	char msg[100];
	try {
		glite::wms::wmproxy::utilities::waitForSeconds(15);
		
		singleton_default<WmproxyConfiguration>::instance().init(opt_conf_file, configuration::ModuleType::network_server);
		logger::threadsafe::edglog.open( singleton_default<WmproxyConfiguration>::instance().wmp_config->log_file(),
				static_cast<logger::level_t>(singleton_default<WmproxyConfiguration>::instance().wmp_config->log_level()) );
		edglog_fn("   WMProxy::main");
		edglog(fatal) << "--------------------------------------" << endl;
		// Open log file destination:
		if (logger::threadsafe::edglog.activate_log_rotation (
			singleton_default<WmproxyConfiguration>::instance().wmp_config->log_file_max_size(),
			singleton_default<WmproxyConfiguration>::instance().wmp_config->log_rotation_base_file(),
			singleton_default<WmproxyConfiguration>::instance().wmp_config->log_rotation_max_file_number())) {
				cout << "Unable to create default log file: " << endl <<  singleton_default<WmproxyConfiguration>::instance().wmp_config->log_rotation_base_file() << endl ;
				cerr << "System exiting..."<< endl ;
				return 1;
		}
		if (argc < 3) {
            // Run as a FastCGI script
			edglog(fatal) << "Running as a FastCGI program" << endl;
			
			cerr<<"---- NS2WMProxy"<<endl;
			/*string file_name = "glite_wms.conf";
			std::auto_ptr<configuration::Configuration> conf;
			conf.reset(new configuration::Configuration(file_name, "NetworkServer"));
			singleton_default<wmproxyserver::NS2WMProxy>::instance()
				.init(configuration::Configuration::instance()->wm()->input());*/
			cerr<<"---- NS2WMProxy END"<<endl;
			
			int thread_number;
			try {
				thread_number =  singleton_default<WmproxyConfiguration>::instance()
					.wmp_config->dispatcher_threads() ;
	    	} catch ( configuration::InvalidExpression &error ) {
	      		edglog(fatal) << "ERROR: Unable to read value from Configuration file" << error << std::endl;
			return 1;
	    	}
			task::Pipe<classad::ClassAd*> pipe;
			WMPDispatcher dispatcher;
			cerr<<"Launching thread(s)..."<<endl;
			edglog(fatal) << "Launching " << thread_number << " dispatcher thread(s)" << endl;
			task::Task dispatchers(dispatcher, pipe, thread_number);
			cerr<<"Launching thread(s)... finished"<<endl;
			
			// Running as a Fast CGI application
			edglog(fatal) << "Entering the FastCGI accept loop..." << endl;
			while (FCGI_Accept() >= 0) {
				WMProxy proxy;
				proxy.serve();
			}
			edglog(fatal) << "Exiting the FastCGI loop..." << endl;

		}else{
			edglog(fatal) << "Running as a gSoap standalone Service" << endl;
        		soap = soap_new();
   			soap->accept_timeout = 60;     /* server times out after 10 minutes of inactivity */
                	soap->recv_timeout = 30;       /* if read stalls, then timeout after 60 seconds */
                	edglog(fatal) << "Binding socket... " << argv[2] << endl;
        		m = soap_bind(soap, argv[1], atoi(argv[2]), 100);
        		if (m < 0)
        		{
				edglog(fatal) << "Failed to bind socket " << argv[2] << endl;
                		soap_print_fault(soap, stderr);
                		exit(-1);
        		}
        		edglog(fatal) << "Socket connection successful: master socket = " << m << endl;
        		for (int i = 1; ; i++)
        		{
                		s = soap_accept(soap);
                		if (s < 0){
					edglog(fatal) << "Failed to accept soap request " << endl;
                        		soap_print_fault(soap, stderr);
					exit(-1);
                		}
                		sprintf(msg, "%d: Accepted connection from IP = %d.%d.%d.%d socket = %d ... ", i, (int)(soap->ip>>24)&0xFF, 				(int)(soap->ip>>16)&0xFF, (int)(soap->ip>>8)&0xFF, (int)soap->ip&0xFF, s);
				edglog(fatal) << msg << endl;
                		soap_serve(soap);         // process request
				edglog(fatal) << "Request served" << endl;
                		soap_destroy(soap);       // delete class instances
                		soap_end(soap);           // clean up everything and close socket
        		} //for
		} //else
        } catch( configuration::CannotOpenFile &file ) {
                    edglog(fatal) << "Cannot open file: " << file << std::endl;
                    std::cout << "Cannot open file: " << file << std::endl;
        } catch( configuration::CannotConfigure &error ) {
                    edglog(fatal) << "Cannot configure: " << error << std::endl;
                    std::cout << "Cannot configure: " << error << std::endl;
	} catch (exception &ex) {
		edglog(fatal) << "Exception caught: " << ex.what() << endl;
 	} catch ( ... ) {
    		edglog(fatal) << "Uncaught exception...." << endl;
  	}
	return 0;
}



//} // wmproxy
//} // wms
//} // glite


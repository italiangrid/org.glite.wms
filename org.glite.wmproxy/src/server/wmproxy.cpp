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

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"

//#include "glite/wms/common/utilities/FileList.h"
//#include "glite/wms/common/utilities/FileListLock.h"

// Exceptions
#include "wmpexception_codes.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "wmpoperations.h"

#include "wmpdispatcher.h"

#include "wmputils.h"


//#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>

// Boost
#include <boost/pool/detail/singleton.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace utilities  = glite::wms::common::utilities;
namespace task          = glite::wms::common::task;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;


using namespace std;


//namespace glite {
//namespace wms {
//namespace wmproxy {

std::string opt_conf_file("glite_wms.conf");



int
main(int argc, char* argv[])
{
	int bind;
	int accept;
	int ssl_accept;
	struct soap *soap;
	struct soap *tsoap;

	try {
		configuration::Configuration config(opt_conf_file, configuration::ModuleType::network_server);
  		//configuration::WMConfiguration const* const wmp_config(config.wmp());
                configuration::NSConfiguration const* const wmp_config(config.ns());

		logger::threadsafe::edglog.open( wmp_config->log_file(), 
						static_cast<logger::level_t>(wmp_config->log_level()) );

		edglog_fn("   WMProxy::main");
		edglog(fatal) << "--------------------------------------" << endl;
		edglog(fatal) << "Starting WM Proxy..." << endl;
		logger::threadsafe::edglog.activate_log_rotation (
			wmp_config->log_file_max_size(),
			wmp_config->log_rotation_base_file(),
			wmp_config->log_rotation_max_file_number());

		if (argc < 3) {
			waitForSeconds(15);

			cerr<<"ISB Size: " << wmp_config->max_input_sandbox_size() << endl;

			int thread_number;
			try {
				thread_number =  wmp_config->dispatcher_threads() ;
		    	} catch ( configuration::InvalidExpression &error ) {
		      		edglog(fatal) << "Invalid Expression: " << error << std::endl;
		    	}
			task::Pipe<classad::ClassAd*> pipe;
			Dispatcher dispatcher;
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
	    } else {}
			/*cerr<<"Starting service..."<<endl;
		    soap = soap_new();
	        soap_init(soap);*/

	        //if (soap_ssl_server_context(soap,
	          //  SOAP_SSL_DEFAULT,
	            //"x509up_u500", /* keyfile: required when server must authenticate to clients (see SSL docs on how to obtain this file) */
	            //"10greenbottles", /* password to read the key file */
	            //NULL, /* optional cacert file to store trusted certificates */
	            //"/etc/grid-security/certificates", /* optional capath to directory with trusted certificates */
	            //NULL, /* DH file, if NULL use RSA */
	            //NULL, /* if randfile!=NULL: use a file with random data to seed randomness */
	            //NULL /* optional server identification to enable SSL session cache (must be a unique name) */    ))
	        /*{
	            soap_print_fault(soap, stderr);
	            exit(1);
	        }

	        bind = soap_bind(soap, argv[1], atoi(argv[2]), 100);
	    	if (bind < 0) {
				soap_print_fault(soap, stderr);
	      	   	exit(-1);
	    	}
		
			cerr<<"Socket Connection Successful"<<endl;
			cerr<<"Master Socket: "<<bind<<endl;
	
			for (;;) {
				accept = soap_accept(soap);
	      		if (accept < 0) {
					soap_print_fault(soap, stderr);
	        		exit(-1);
	      		}
	            tsoap = soap_copy(soap);
	            if (!tsoap) {
	            	break;
	            }
	            ssl_accept = soap_ssl_accept(tsoap);
	            if (ssl_accept < 0) {
	                //cerr << "Fail SSL accept" << endl;
	                soap_print_fault(tsoap, stderr);
	                soap_done(tsoap);
	                free(tsoap);
	                continue; 
	            } 
	      		soap_serve(tsoap);         // process request
	      		soap_destroy(tsoap);       // delete class instances
	      		soap_end(tsoap);           // clean up everything and close socket
	    	}
	  		soap_destroy(soap);       // delete class instances
	  		soap_end(soap);           // clean up everything and close socket
			//cerr<<"Service stopped"<<endl;
		}*/
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


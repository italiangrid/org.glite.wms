/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmproxy.h"

// gSOAP
#include "soapH.h"
#include "WorkloadManagerProxy_USCOREbinding.nsmap"
#include "soapWorkloadManagerProxy_USCOREbindingObject.h"
//#include "wmproxynameH.h"
//#include "wmproxyname.nsmap"

// Grid security infrastructure
#include "gsi.h"

// Logging
#include "logging.h"
#include "glite/wms/common/logger/edglog.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"

#include "exception_codes.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "wmpoperations.h"

#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>

namespace logger = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;

using namespace std;

//namespace wmproxyname {

int
gsi_authorization_callback(struct soap * soap)
{
    struct gsi_plugin_data *data = NULL;
    data = (struct gsi_plugin_data*)soap_lookup_plugin(soap, GSI_PLUGIN_ID);
    cout<<"Connected to: "<<data->server_identity<<endl;
    return 0;
}

void *
process_request(void *arg)
{
	struct gsi_plugin_data *data;
	struct soap *soap;

	soap = (struct soap *) arg;
	data = (struct gsi_plugin_data *) soap_lookup_plugin(soap, GSI_PLUGIN_ID);

	// Timeout after 5 minutes stall on recv
	soap->recv_timeout = 300;
	// Timeout after 1 minute stall on send
	soap->send_timeout = 60;

	int rc = gsi_accept_security_context(soap);
	if (rc == 0) {

		fprintf(stderr, "Established security context with: %s\n",
			data->client_identity);

		rc = data->gsi_authorization_callback(soap);
		if (rc == 0) {
			soap_serve(soap);
		} else {
			fprintf(stderr, "User %s is not authorized\n", data->client_identity);
			soap_receiver_fault(soap, "Authorization error\n", NULL);
			soap_send_fault(soap);
		}
	} else {
		fprintf(stderr, "gsi_accept_security_context ERROR\n");
	}
	soap_destroy(soap);
	soap_end(soap);
	soap_done(soap);
}

//namespace glite {
//namespace wms {
//namespace wmproxyname {

int
main(int argc, char* argv[])
{
	try {
		/*logger::threadsafe::edglog.open(
			configuration::Configuration::instance()->ns()->log_file(),
			static_cast<logger::level_t>
			(configuration::Configuration::instance()->ns()->log_level()));
		edglog_fn("   NS::main");
		edglog(fatal) << "--------------------------------------" << endl;
		edglog(fatal) << "Starting Network Server..." << endl;
		logger::threadsafe::edglog.activate_log_rotation (
			configuration::Configuration::instance()->ns()->log_file_max_size(),
			configuration::Configuration::instance()->ns()->log_rotation_base_file(),
	        configuration::Configuration::instance()->ns()->log_rotation_max_file_number());
	*/
	if (argc < 3) {
     	// Running as CGI application
        WorkloadManagerProxy proxy;
        proxy.serve();
    } else {
		cout<<"Starting service..."<<endl;
		struct soap soap;
		struct soap *soapThreadArray[MAX_THREAD_NUM];
		static struct gsi_plugin_data *data = NULL;
		pthread_t tid[MAX_THREAD_NUM];
	
		if (globus_module_activate(GLOBUS_COMMON_MODULE) != GLOBUS_SUCCESS) {
			cerr<<"Failing in GLOBUS_COMMON_MODULE initialization"<<endl;
		};
	    	if (globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE) != GLOBUS_SUCCESS) {
			cerr<<"Failing in GLOBUS_GSI_GSSAPI_MODULE initialization"<<endl;
		};
		if (globus_module_activate(GLOBUS_GSI_GSS_ASSIST_MODULE) != GLOBUS_SUCCESS) {
			cerr<<"Failing in GLOBUS_GSI_GSS_ASSIST_MODULE initialization"<<endl;
		};
	
	    soap_init2(&soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
	    //soap_set_namespaces(&soap, wmproxyname_namespaces);
	
	    if (soap_register_plugin(&soap, globus_gsi)) {
	    	soap_print_fault(&soap, stderr);
	        soap_destroy(&soap);
	        soap_end(&soap);
	        soap_done(&soap);
	        exit(1);
	    }
	
	    // we begin acquiring our credential
	    int rc = gsi_acquire_credential(&soap);
	    cerr<<"return value: "<<rc<<endl;
	    if (rc < 0) {
	    	soap_print_fault(&soap, stderr);
	        soap_destroy(&soap);
	        soap_end(&soap);
	        //soap_done(&soap);
	        cerr<<"(Try checking if your proxy is valid)"<<endl;
	        exit(1);
	    }
	
	    // setup of authorization callback
	    data = (struct gsi_plugin_data *) soap_lookup_plugin (&soap, GSI_PLUGIN_ID);
	    data->gsi_authorization_callback = gsi_authorization_callback;
	
	    // listen for incoming connections
	    //SERVER_ADDRESS = argv[1];
	    //SERVER_PORT = atoi(argv[2]);
	    data->listening_fd = gsi_listen(&soap, SERVER_ADDRESS, SERVER_PORT, BACKLOG);
	    if (data->listening_fd == -1) {
	    	cerr<<"Failing in gsi_listen, now exiting"<<endl;
	    	soap_destroy(&soap);
	    	soap_end(&soap);
	    	soap_done(&soap);
	    	exit(-1);
	    }
	
		cout<<"Socket Connection Successful"<<endl;
		
		// SSL initialization
		/*if( edg_wlc_SSLInitialization() != 0) {
		    //edglog(fatal) << "Failed to initialize SSL" << std::endl;
		    exit(-2);
		}
		// SSL locking
		if (edg_wlc_SSLLockingInit() != 0) {
		    //edglog(fatal) << "Failed to initialize SSL Locking" << std::endl;
		    exit(-3);
		}*/
		
		// soap array initialization to NULL
		for (int i = 0; i < MAX_THREAD_NUM; i++) {
			soapThreadArray[i] = NULL;
		}
	
		for (;;) {
			for (int i = 0; i < MAX_THREAD_NUM; i++) {
				// accepting incoming connections
				data->connected_fd = gsi_accept(&soap);
				if (data->connected_fd == -1) {
					cerr<<"Failing in gsi_accept, now exiting"<<endl;
					soap_destroy(&soap);
			    	soap_end(&soap);
			    	soap_done(&soap);
					exit(-1);
				}
			
				// retrieving information about the peer
				cout<<"Accepted connection from: "<<soap.host<<endl;
	
				if (!soapThreadArray[i]) {
					// spawning a new thread to serve the client's request
					soapThreadArray[i] = soap_copy(&soap);
					if (!soapThreadArray[i]) {
						exit(1);
					}
				} else {
					pthread_join(tid[i], NULL); // Check it??
					soap_destroy(soapThreadArray[i]);
					soap_end(soapThreadArray[i]);
					
					soapThreadArray[i] = soap_copy(&soap);
				}
	
				soapThreadArray[i]->socket = data->connected_fd;
				cerr<<"SOAP struct copied for thread index: -> "<<i<<endl;
				pthread_create(&tid[i], NULL, &process_request,
					(void*)soapThreadArray[i]);
				cerr<<"Thread started"<<endl;
			}
		}
		//soap_done(&soap);
		cout<<"Service stopped"<<endl;
		}
	}
	catch (exception &ex) {
		//edglog(fatal) << "Exception caught: " << ex.what() << endl;
 	} 
  	catch ( ... ) {
    	//edglog(fatal) << "Uncaught exception...." << endl;	 
  	}
	return 0;
}

//} // wmproxy
//} // wms
//} // glite


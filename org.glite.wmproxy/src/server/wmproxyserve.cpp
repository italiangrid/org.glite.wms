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
// File: wmproxyserve.cpp
// Author: Giuseppe Avellino <egee@datamat.it>
//

#include "soapH.h"
#include "wmproxyserve.h"

// Configuration
#include "wmpconfiguration.h"

//Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"

#include "wmpsignalhandler.h"


// Global variable for configuration
extern WMProxyConfiguration conf;

// Global variable for server instance served request count
extern long servedrequestcount_global;
// Global variable for signal handling 
extern bool handled_signal_recv;


const long MIN_SERVED_REQUESTS_PLAFOND = 40;
const long BASE_SERVED_REQUESTS_PLAFOND = 100;


SOAP_FMAC5 int SOAP_FMAC6
WMProxyServe::wmproxy_soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	unsigned int k = soap->max_keep_alive;
#endif

	do
	{
#ifdef WITH_FASTCGI
                // Handling termination signal
		if (handled_signal_recv>0) {
			edglog(info)<<"-------- Exiting Server Instance -------"<< std::endl;
			edglog(info)<<"Signal code received: "<< handled_signal_recv << std::endl;
			edglog(info)<<"----------------------------------------"<< std::endl;
			exit(0);
		}
		// Checking for server instance served request count. If the maximum
		// value is reached, the server instance is exited in order to avoid
		// possible memory leaks.
		long plafond = BASE_SERVED_REQUESTS_PLAFOND;
		char * env_plafond = getenv("GLITE_WMS_WMPROXY_MAX_SERVED_REQUESTS");
		if (env_plafond) {
			plafond = atol(env_plafond);
		} else if (conf.getMaxServedRequests()) {
			plafond = conf.getMaxServedRequests();
		}
		// plafond <= 0 means infinite requests i.e. no process exit
		if (plafond > 0) {
			plafond = std::max(plafond, MIN_SERVED_REQUESTS_PLAFOND);
			if (servedrequestcount_global >= plafond) {
	    		edglog(info)<<"-------- Exiting Server Instance -------"<<std::endl;
	    		edglog(info)<<"Maximum core request count reached: "<<plafond
	    			<<std::endl;
	    		edglog(info)<<"----------------------------------------"<<std::endl;
                        FCGI_Finish();
	    		exit(0);	
	    	}
		}
		edglog(debug)<<"Resetting signals handler"<<std::endl;
		glite::wms::wmproxy::server::resetsignalhandler();
		if (FCGI_Accept() < 0)
		{
			soap->error = SOAP_EOF;
			return soap_send_fault(soap);
		}
#endif
		soap_begin(soap);

#ifndef WITH_FASTCGI
		if (!--k)
			soap->keep_alive = 0;
#endif

		if (soap_begin_recv(soap))
		{	if (soap->error < SOAP_STOP)
			{
#ifdef WITH_FASTCGI
				soap_send_fault(soap);
#else 
				return soap_send_fault(soap);
#endif
			}
			soap_closesock(soap);

			continue;
		}

		if (soap_envelope_begin_in(soap)
		 || soap_recv_header(soap)
		 || soap_body_begin_in(soap)
		 || soap_serve_request(soap)
		 || (soap->fserveloop && soap->fserveloop(soap)))
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}

#ifdef WITH_FASTCGI
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

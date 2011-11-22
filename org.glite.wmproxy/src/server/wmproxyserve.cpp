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
// Author: Giuseppe Avellino <egee@datamat.it>
//

#include <fcgi_stdio.h>

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"
#include "wmproxyserve.h"
#include "configuration.h"
#include "signalhandler.h"

namespace server = glite::wms::wmproxy::server;

SOAP_FMAC5 int SOAP_FMAC6
WMProxyServe::wmproxy_soap_serve(struct soap *soap)
{
   unsigned int requests = soap->max_keep_alive; // HTTP keep-alive settings in httpd
   while (requests >= 0 && 0 == server::handled_signal_recv) {
      if (FCGI_Accept() >= 0 ) {
         edglog(info)<<"0------- fcgi accept"<< std::endl;
         soap_begin(soap);
         edglog(info)<<"1------- after soap_begin "<< std::endl;
         edglog(info)<<"2------- keep alive"<< requests<< std::endl;
         --requests;

         if (soap_begin_recv(soap)) {
            if (soap->error < SOAP_STOP) {
               return soap_send_fault(soap);
            }
            soap_closesock(soap);
            edglog(info)<<"4------- continue :"<< server::handled_signal_recv << "requests" << requests<< std::endl;
            continue;
         }

         if (soap_envelope_begin_in(soap)
               || soap_recv_header(soap)
               || soap_body_begin_in(soap)
               || soap_serve_request(soap)
               || (soap->fserveloop && soap->fserveloop(soap))) {
            edglog(info)<<"55555--- fault"<< std::endl;
            return soap_send_fault(soap);
         }
      } else {
         soap->error = SOAP_EOF;
         return soap_send_fault(soap);
      }
   } // while

   if (server::handled_signal_recv > 0) {
      edglog(info) << "-------- Exiting Server Instance -------" << std::endl;
      edglog(info) << "Signal code received: "<< server::handled_signal_recv << std::endl;
   } else {
      edglog(info) << "-------- Exiting Server Instance -------" << std::endl;
      edglog(info) << "Maximum number of requests allowed by HTTP keepalive "
                   "reached (" << soap->max_keep_alive << ')' << std::endl;
   }

   soap_destroy(soap);
   soap_end(soap);
   soap_free(soap);

   return SOAP_OK;
}

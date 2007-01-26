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
// File: wmpsignalhandler.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpsignalhandler.h"

#include "fcgios.h"

#include <signal.h>
#include <fcgi_stdio.h>

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"


namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

using namespace std;

// Handler prototype
void handler(int code);

void
initsignalhandler()
{
    // Ignoring PIPE signals, mod_fastcgi already does it
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGPIPE"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGPIPE -> SIG_IGN"<<endl;
    }

    // not needed mod_fastcgi sends SIGUSR1 when signal has been received?
    if (signal(SIGTERM, handler) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGTERM"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGTERM -> handler"<<endl;
    }

    // Apache request for a "graceful" process shutdown (e.g. restart)
    if (signal(SIGUSR1, handler) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGUSR1"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGUSR1 -> handler"<<endl;
    }
    
    if (signal(SIGHUP, handler) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGHUP"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGHUP -> handler"<<endl;
    }
    
    if (signal(SIGINT, handler) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGINT"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGINT -> handler"<<endl;
    }
    
    if (signal(SIGQUIT, handler) == SIG_ERR) {
    	edglog(debug)<<"Unable to catch signal SIGQUIT"<<endl;
    } else {
    	edglog(debug)<<"Signal SIGQUIT -> handler"<<endl;
    }
}

//extern "C" {

void handler(int code) {
	initsignalhandler();
	edglog(debug)<<"Handling sinal: "<<code<<endl;
	OS_ShutdownPending();            
	//FCGI_Finish();
	exit(0);
}

//} // extern "C"


} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

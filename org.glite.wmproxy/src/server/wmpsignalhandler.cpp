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
// Author: Giuseppe Avellino <egee@datamat.it>
//

#include "wmpsignalhandler.h"

#include "fcgios.h"

#include <signal.h>
#include <fcgi_stdio.h>

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"


// Global variable for signal handling
extern volatile sig_atomic_t handled_signal_recv;


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
	signal(SIGPIPE, SIG_IGN);
	// not needed mod_fastcgi sends SIGUSR1 when signal has been received?
	signal(SIGTERM, handler);
	// Apache request for a "graceful" process shutdown (e.g. restart)
	signal(SIGUSR1, handler);
	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
}

void
resetsignalhandler()
{
	// Ignoring PIPE signals, mod_fastcgi already does it
	signal(SIGPIPE, SIG_IGN);
	// not needed mod_fastcgi sends SIGUSR1 when signal has been received?
	signal(SIGTERM, SIG_DFL);
	// Apache request for a "graceful" process shutdown (e.g. restart)
	signal(SIGUSR1, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}


void handler(int code) {
	handled_signal_recv = code;
}

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

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

typedef void handler_func(int);
void install_signal(int signo, handler_func* func)
{
  struct sigaction act, old_act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = func;
  sigaction(signo, &act, &old_act);
}

void
initsignalhandler()
{
  install_signal(SIGUSR1, handler);
  install_signal(SIGPIPE, SIG_IGN);
  install_signal(SIGTERM, handler);
  install_signal(SIGXFSZ, handler);
  install_signal(SIGHUP, handler);
  install_signal(SIGINT, handler);
  install_signal(SIGUSR1, handler);
  install_signal(SIGQUIT, handler);

	handled_signal_recv = 0;
}

void handler(int code) {
  edglog(info)<<"received signal " << code << endl;

	handled_signal_recv = code;
}

} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite

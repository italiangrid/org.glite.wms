/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <glite/security/proxyrenewal/renewal_core.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cerrno>

extern int errno;

using namespace std;

static struct option const long_options[] = {
   { "server",   required_argument, 0, 's' },
   { "proxy",    required_argument, 0, 'p' },
   { "help",     no_argument,       0, 'h' },
   { "output",   required_argument, 0, 'o'},
   { NULL, 0, NULL, 0}
};

static char short_options[] = "s:p:h:o:";

int
main(int argc, char *argv[])
{
   char *server = NULL;
   char *proxy = NULL;
   char *new_proxy = NULL;
   extern int optind;
   char arg;
   glite_renewal_core_context ctx = NULL;
   int ret;

   char *outputfile = NULL;

   while ((arg = getopt_long(argc, argv, short_options, long_options, NULL)) != EOF) {
      switch(arg) {
      case 's':
	server = optarg; break;
      case 'p':
	proxy = optarg; break;
      case 'o':
	outputfile = optarg;break;
      case 'h':
	//fprintf(stdout, "Usage: %s --server <myproxy server> --proxy <filename>\n", argv[0]);
	cerr << "Usage: "
	     << argv[0] << " --server <myproxy server> --proxy <filename> --output <output file proxy>\n";
	return 1;
      
	
      }
   }

   if (server == NULL || proxy == NULL || outputfile == NULL) {
     cout << "both server and proxy parameters must be given and output file name\n";
     return 1;
   }

  struct stat buf;
  int rc = stat(proxy, &buf);
  if( rc ) {
    cerr << strerror( errno ) << endl;
    return 1;
  }


   ret = glite_renewal_core_init_ctx(&ctx);
   if (ret) {
     fprintf(stderr, "glite_renewal_core_init_ctx() failed\n");
     cout << "glite_renewal_core_init_ctx() failed\n";
     return 1;
       //       exit(1);
   }

   ctx->log_dst = GLITE_RENEWAL_LOG_NONE;

   ret = glite_renewal_core_renew(ctx, server, 0, proxy, &new_proxy);
   if (ret) {
     //      fprintf(stderr, "%s: glite_renewal_core_renew() failed: %s",
     //              argv[0], ctx->err_message);
     cout << argv[0] <<": glite_renewal_core_renew() failed: "
	  << ctx->err_message << endl;
       
     //      exit(1);
     return 1;
   }

   ret = glite_renewal_core_destroy_ctx(ctx);

   //   printf("%s\n", new_proxy);
   cout <<new_proxy<<endl;

   ::rename( new_proxy, outputfile );

   return 0;
}

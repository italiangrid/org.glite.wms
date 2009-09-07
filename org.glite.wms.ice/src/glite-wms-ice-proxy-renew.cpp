#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <glite/security/proxyrenewal/renewal_core.h>

#include <iostream>

using namespace std;

static struct option const long_options[] = {
   { "server",   required_argument, 0, 's' },
   { "proxy",    required_argument, 0, 'p' },
   { "help",     no_argument,       0, 'h' },
   { NULL, 0, NULL, 0}
};

static char short_options[] = "s:p:h";

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

   while ((arg = getopt_long(argc, argv, short_options, long_options, NULL)) != EOF) {
      switch(arg) {
	case 's':
	   server = optarg; break;
	case 'p':
	   proxy = optarg; break;
	case 'h':
	  //fprintf(stdout, "Usage: %s --server <myproxy server> --proxy <filename>\n", argv[0]);
	  cerr << "Usage: "
	       << argv[0] << " --server <myproxy server> --proxy <filename>\n";
	   return 1;
      }
   }

   if (server == NULL || proxy == NULL) {
     cout << "both server and proxy parameters must be given\n";
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

   return 0;
}















// #include "iceConfManager.h"
// #include "glite/security/proxyrenewal/renewal_core.h"
// //#include "glite/security/proxyrenewal/renewal.h"
// #include "glite/wms/common/configuration/ICEConfiguration.h"
// #include "glite/wms/common/configuration/CommonConfiguration.h"

// #include <iostream>

// using namespace std;

// /**
//    argv[1]: myproxyserver name
//    argv[2]: proxy certificate file
// */
// int main( int argc, char* argv[] )
// {
//   if(argc<3)
//     {
//       cerr << "too few arguments. Usage: " << argv[0] << " myproxyserver_name certfile [glite wms conf file]" << endl;
//       return 1;
//     }

//   glite_renewal_core_context ctx;

//   glite_renewal_core_init_ctx( &ctx );

//   if(argv[3] == NULL)
//     glite::wms::ice::util::iceConfManager::init( "glite_wms.conf" );
//   else
//     glite::wms::ice::util::iceConfManager::init( argv[3] );

//   string proxy = glite::wms::ice::util::iceConfManager::getInstance()->getConfiguration()->common()->host_proxy_file().c_str();

//   cout << "host proxy=[" << proxy << "]" <<endl;

//   //setenv("X509_USER_CERT", proxy.c_str(), 1);
  
//   //setenv("X509_USER_KEY", proxy.c_str(), 1);

//   char* new_proxy = NULL;

//   int rc = glite_renewal_core_renew(ctx,
// 				    argv[1],
// 				    0,
// 				    argv[2],
// 				    &new_proxy);
  
//   //unsetenv("X509_USER_CERT");
//   //unsetenv("X509_USER_KEY");
  
//   if( rc ) {
//     //string errmex = edg_wlpr_GetErrorText( rc );
//     cerr << "Couldn't download a new proxy for user from MyProxy server ["
// 	 << argv[1]
// 	 << "] using cert file [" 
// 	 << argv[2] 
// 	 <<"]: " 
// 	 << ctx->err_message << endl;

    
//     return 1;
//   }
  
//   cout << new_proxy<<endl;
//   glite_renewal_core_destroy_ctx( ctx );
//   return 0;
// }

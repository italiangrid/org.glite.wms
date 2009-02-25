/*############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################
#
# AUTHORS: Dimitar Shiyachki <Dimitar.Shiyachki@cern.ch>
#
############################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <serrno.h>
#include <dpm_api.h>
#include <dpns_api.h>

int main (int argc, char** argv)
{
   time_t now;
   struct dpm_putfilereq *request;
   struct dpm_filestatus *donestatus;
   struct dpm_putfilestatus *filestatus;
   char *protocols [] = { "gsiftp" };
   char req_token [CA_MAXDPMTOKENLEN+1];
   char *turl;
   int i, stcount, debug = 1;
   pid_t guc_pid, wait_pid;

   if ( argc != 6 )
   {
      printf ("Usage: putabort <filename> <s_url> <do_abort[0|1]> <file_type[V|P]> <lifetime>\n");
      exit (1);
   }

   request = malloc (sizeof(struct dpm_putfilereq));
   if ( request == NULL ) 
   {
      perror ("malloc");
      exit (1);
   }

   memset (request, '\0', sizeof(struct dpm_putfilereq));
   request->to_surl = argv[2];

   if ( strcmp (argv[5], "MAX_LIFETIME") == 0 )
   {
      if ( debug )
         printf ("File lifetime: MAX_LIFETIME\n");
      request->f_lifetime = MAX_LIFETIME;
   }
   else
   {
      if ( debug )
         printf ("File lifetime: %d\n", atoi(argv[5]));
      request->f_lifetime = atoi(argv[5]);
   }

   if ( strcmp (argv[4], "P") == 0 )
   {
      if ( debug ) 
         printf ("File type: PERMANENT\n");
      request->f_type = 'P';
   }
   else
   {
      if ( debug )
         printf ("File type: VOLATILE\n");
      request->f_type = 'V';
   }

   int ret = dpm_put (1, request, 1, protocols, NULL, 0, 0, 
                      req_token, &stcount, &filestatus);

   if ( ret < 0 )
   {
      sperror ("dpm_put");
      exit (1);
   }

   ret = dpm_getstatus_putreq (req_token, 0, NULL, 
                               &stcount, &filestatus);

   if ( ret < 0 )
   {
      sperror ("dpm_getstatus_putreq");
      exit (1);
   }
	
   for ( i = 0; i < 10 && (ret == DPM_QUEUED || ret == DPM_ACTIVE); ++i )
   {
      if ( filestatus->to_surl )
         free (filestatus->to_surl);

      if ( filestatus->turl )
         free (filestatus->turl);

      ret = dpm_getstatus_putreq (req_token, 0, NULL,
                                  &stcount, &filestatus);
      if ( ret < 0 )
      {
         sperror ("dpm_getstatus_putreq");
         exit (1);
      }

      sleep (1);
   }

   if ( ret != DPM_SUCCESS || filestatus->status != DPM_READY )
   {
      printf ("File was not prepared for put in the required timeframe\n");
      exit (1);
   } 

   if ( filestatus->turl == NULL )
   {
      printf ("Transport URL is NULL. Abort.\n");
      exit (1);
   }

   turl = filestatus->turl;

   if ( debug )
   {
      printf ("Transport URL is: %s\n", turl);
      printf ("Going to fork\n");
   }

   guc_pid = fork();

   if ( guc_pid == 0 )
   {
      ret = execlp ("globus-url-copy", "globus-url-copy", argv[1], turl, (char *) 0);
      if ( ret == -1 )
      {
         printf ("Cannot execute globus-url-copy\n");
         perror ("execlp");
         exit (1);
      }
   }

   wait_pid = waitpid (guc_pid, NULL, 0);

   if ( strcmp (argv[3], "0" ) != 0 )
   {
      ret = dpm_abortreq(req_token);
      if (debug)
      {
         printf("DPM Abort returned: %d\n", ret);
      }

      ret = dpm_getstatus_putreq (req_token, 0, NULL,
                                  &stcount, &filestatus);
      if ( ret < 0 )
      {
         sperror ("dpm_getstatus_putreq");
        exit (1);
      }

      if ( ret == DPM_ABORTED )
      {
         printf ("Request aborted successfully.\n");
         exit(0);
      }
      else
      {
         printf ("Abort request failed.\n");
         exit (1);
      }
   }
   else
   {
      ret = dpm_putdone (req_token, 1, &argv[2], &stcount, &donestatus);
      if ( ret < 0 )
      {
         sperror ("dpm_putdone");
         exit (1);
      }

      printf ("%s\n", turl);
      exit (0);
   }
}

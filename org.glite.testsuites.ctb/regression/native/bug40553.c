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
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dpm_api.h>
#include <serrno.h>

int main (int argc, char** argv)
{
   struct dpm_putfilereq *request;
   struct dpm_filestatus *donestatus;
   struct dpm_putfilestatus *filestatus;
   char *protocols [] = { "gsiftp" };
   char req_token [CA_MAXDPMTOKENLEN+1];
   char *turl;
   int i, stcount, debug = 0;
   pid_t guc_pid, wait_pid;

   if ( argc == 1 || argc > 3 )
   {
      printf ("Usage: put <s_url> [<space_token>]\n");
      exit (1);
   }

   request = malloc (sizeof(struct dpm_putfilereq));
   if ( request == NULL ) 
   {
      perror ("malloc");
      exit (1);
   }

   memset (request, '\0', sizeof(struct dpm_putfilereq));
   request->to_surl = argv[1];
   request->f_lifetime = MAX_LIFETIME;
   request->f_type = 'V';

   if ( argc > 2 ) 
   {
      strncpy (request->s_token, argv[2], CA_MAXDPMTOKENLEN);
      *(request->s_token + CA_MAXDPMTOKENLEN + 1) = '\0';
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
	
   // 
   // Wait untill we are ready to transfer the file to the DPM server
   // 

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

   //
   // Transport URL is now available. Execute globus-url-copy and wait for it
   // to complete.
   //

   if ( debug )
   {
      printf ("Transport URL is: %s\n", turl);
      printf ("Going to fork\n");
   }

   guc_pid = fork();

   if ( guc_pid == 0 )
   {
      ret = execlp ("globus-url-copy", "globus-url-copy", "file:///bin/bash", turl, (char *) 0);
      if ( ret == -1 )
      {
         printf ("Cannot execute globus-url-copy\n");
         perror ("execlp");
         exit (1);
      }
   }

   wait_pid = waitpid (guc_pid, NULL, 0);
   if ( wait_pid < 0 )
   {
      printf ("Child did not completed successfully!\n");
      exit (1);
   }

   ret = dpm_putdone (req_token, 1, &argv[1], &stcount, &donestatus);
   if ( ret < 0 )
   {
      sperror ("dpm_putdone");
      exit (1);
   }

   printf ("%s\n", turl);
   exit (0);
}


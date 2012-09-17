#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dpm_api.h>
#include <dpns_api.h>
#include <serrno.h>

int main (int argc, char** argv)
{
   int ret;
   int rxcount;
   int i;
   struct dpns_filereplicax *rxarray;

/*
       struct dpns_filereplicax {
            u_signed64     fileid;
            u_signed64     nbaccesses;
            time_t         ctime;          replica creation time 
            time_t         atime;          last access to replica 
            time_t         ptime;          replica pin time 
            time_t         ltime;          replica lifetime 
            char      r_type;         'P' for Primary, 'S' for Secondary
            char      status;
            char      f_type;         'V' for Volatile, 'P' for Permanent
            char      setname[37];
            char      poolname[CA_MAXPOOLNAMELEN+1];
            char      host[CA_MAXHOSTNAMELEN+1];
            char      fs[80];
            char      sfn[CA_MAXSFNLEN+1];
       };
*/

   if ( argc < 2 )
   {
      printf ("Usage: getreplicax <path>\n");
      printf ("DPNS_HOST should be set appropriately.\n");
      return 1;
   }

   ret = dpns_getreplicax ( argv[1], 
                            NULL,
                            NULL,
                            &rxcount,
                            &rxarray);

   if ( ret == 0 )
   {
      for ( i = 0; i < rxcount; ++i )
      {
         printf ("%s;%s;%s;%s\n", (rxarray+i)->poolname, 
                                  (rxarray+i)->setname,
                                  (rxarray+i)->host,
                                  (rxarray+i)->fs);
      }
   }
   else
   {
      printf ("Error %d while calling dpns_getreplicax\n", ret); 
      return 1;
   }

   return 0;
}

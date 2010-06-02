#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>
#include <limits.h>
#include <unistd.h>

int main (int argc, char** argv)
{
    int i, ret;

    if ( argc != 2 )
    {
        printf ("Usage: dpmerase <file>\n");
        return -1;
    }

    int nbentries;
    struct dpns_filereplicax *rep_entries;
    printf ("Trying to get replicas for: %s\n", argv[1]);
    ret = dpns_getreplicax (argv[1], NULL, NULL, &nbentries, &rep_entries);
    if ( ret != 0 )
    {
        printf ("dpns_getreplicax: %s\n", sstrerror(serrno));
        return -1;
    }

    printf ("Number of replicas found: %d\n", nbentries);

    for ( i = 0; i < nbentries; ++i )
    {
       ret = dpns_setrstatus (rep_entries[i].sfn, 'X');
       if ( ret != 0 )
       {
           printf ("dpns_setrstatus: %s; sfn: %s\n", sstrerror(serrno), rep_entries[i].sfn);
           continue;
       }

       ret = dpns_modreplicax (rep_entries[i].sfn, NULL, rep_entries[i].poolname, rep_entries[i].host, rep_entries[i].fs, 'S');
       if ( ret != 0 )
       {
           printf ("dpns_modreplicax: %s; sfn: %s\n", sstrerror(serrno), rep_entries[i].sfn);
           continue;
       }

       ret = dpm_delreplica (rep_entries[i].sfn);
       if ( ret != 0 )
       {
           printf ("dpm_delreplica: %s\n", sstrerror(serrno));
           continue;
       }

       printf ("Replica %s deleted successfully\n", rep_entries[i].sfn);
    }

    return 0;
}


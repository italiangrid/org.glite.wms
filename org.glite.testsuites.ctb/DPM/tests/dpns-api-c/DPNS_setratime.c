#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>
#include <limits.h>
#include <unistd.h>
#include <openssl/pem.h>

#include "utils.h"

int main (int argc, char** argv)
{
    int ret;
    int error = 0;
    dpns_DIR* dir;
    char testdesc[256];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char * envvar;
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_setratime <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setratime tests...");

    // Test 1: Register two replicas and set their access time
    strcpy (testdesc, "Register two replicas and set their access time");
    sprintf (filename, "%s/file_setratime", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs1/email/file_setratime.1", '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs2/email/file_setratime.2", '-', 'V', "pool2", "/fs2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    int ecount = 0;
    struct dpns_filereplica * entries;
    ret = dpns_getreplica (filename, NULL, NULL, &ecount, &entries);
    if ( ecount != 2 )
    {
        error = 1;
        reportComponent (testdesc, "Replicas count does not match", sstrerror(serrno), 1);
        if ( ret == 0 )
            free (entries);
        goto test1_end;
    }

    time_t atime1, atime2;
    if ( strcmp (entries[0].sfn, "disk64.cern.ch:/fs1/email/file_setratime.1") == 0 )
    {
        atime1 = entries[0].atime;
        atime2 = entries[1].atime;
    }
    else
    {
        atime1 = entries[1].atime;
        atime2 = entries[0].atime;
    }
    free (entries);

    ret = dpns_setratime ("disk64.cern.ch:/fs1/email/file_setratime.1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot set access time for replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    sleep (3);
    ret = dpns_setratime ("disk64.cern.ch:/fs2/email/file_setratime.2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot set access time for replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_getreplica (filename, NULL, NULL, &ecount, &entries);
    if ( ecount != 2 )
    {
        error = 1;
        reportComponent (testdesc, "Replicas count does not match(2)", sstrerror(serrno), 1);
        if ( ret == 0 )
            free (entries);
        goto test1_end;
    }

    time_t atime3, atime4;
    if ( strcmp (entries[0].sfn, "disk64.cern.ch:/fs1/email/file_setratime.1") == 0 )
    {
        atime3 = entries[0].atime;
        atime4 = entries[1].atime;
    }
    else
    {
        atime3 = entries[1].atime;
        atime4 = entries[0].atime;
    }
    free (entries);

    if ( (atime3 - atime1) < 0 || (atime3 - atime1) > 7 )
    {
        error = 1;
        reportComponent (testdesc, "Error in access time difference for replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( (atime4 - atime2) < 3 || (atime4 - atime2) > 10 )
    {
        error = 1;
        reportComponent (testdesc, "Error in access time difference for replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    test1_end:

        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs1/email/file_setratime.1");
        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs2/email/file_setratime.2");
        sprintf (filename, "%s/file_setratime", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



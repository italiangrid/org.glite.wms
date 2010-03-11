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
        printf ("Usage: DPNS_setrstatus <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setrstatus tests...");


    // Test 1: Create a file and register a replica which is being populated
    strcpy (testdesc, "Change the status of a replica which had been populated to available");
    sprintf (filename, "%s/file_setrstatus", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "head64.cern.ch", "head64.cern.ch:/fs1/email/file_setrstatus.1", 'P', 'V', "file_setrstatus", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_setrstatus ("head64.cern.ch:/fs1/email/file_setrstatus.1", '-');
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot change status to available", sstrerror(serrno), 1);
        goto test1_end;
    }

    int ecount = 0;
    struct dpns_filereplicax * xentries;
    ret = dpns_getreplicax (filename, NULL, NULL, &ecount, &xentries);
    if ( ecount != 1 )
    {
        error = 1;
        reportComponent (testdesc, "More than one replica was found", sstrerror(serrno), 1);
        if ( ret == 0 )
            free (xentries);
        goto test1_end;
    }
    char r_status = 0;
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getreplicax call failed", sstrerror(serrno), 1);
        goto test1_end;
    }
    else
    {
        r_status = xentries[0].status;
        free (xentries);
    }
    if ( r_status != '-' )
    {
        error = 1;
        reportComponent (testdesc, "Replica status verification failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    test1_end:

        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs1/email/file_setrstatus.1");
        sprintf (filename, "%s/file_setrstatus", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}


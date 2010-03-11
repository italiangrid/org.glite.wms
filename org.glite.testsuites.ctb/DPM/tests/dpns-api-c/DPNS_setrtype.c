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
        printf ("Usage: DPNS_setrtype <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setrtype tests...");

    // Test 1: Create a file and register a permanent replica
    strcpy (testdesc, "Register a permanent replica and convert it to volatile");
    sprintf (filename, "%s/file_setrtype", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "head64.cern.ch", "head64.cern.ch:/fs1/email/file_setrtype.1", 'P', 'P', "file_setrtype", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_setrtype ("head64.cern.ch:/fs1/email/file_setrtype.1", 'V');
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot convert replica type to VOLATILE", sstrerror(serrno), 1);
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
    char f_type = 0;
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getreplicax call failed", sstrerror(serrno), 1);
        goto test1_end;
    }
    else
    {
        f_type = xentries[0].f_type;
        free (xentries);
    }
    if ( f_type != 'V' )
    {
        error = 1;
        reportComponent (testdesc, "Replica type verification failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    test1_end:
        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs1/email/file_setrtype.1");
        sprintf (filename, "%s/file_setrtype", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}


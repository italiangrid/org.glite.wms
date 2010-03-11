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
        printf ("Usage: DPNS_setptime<DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setptime tests...");

    // Test 1: Create a file in <BASE_DIR>, register a replica and set the pin time.
    strcpy (testdesc, "Test 1:Create a file, register a replica and set pin time");
    sprintf (filename, "%s/file_setptime", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs1/email/file_setptime.1", '-', 'P', "siteX", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_setptime ("disk64.cern.ch:/fs1/email/file_setptime.1", (time_t) 1268014402);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setptime returns an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    // Check the result with dpns_listreplica
    serrno = 0;
    struct dpns_filereplica * reps;
    dpns_list rlist;
    int repcount = 0;
    int rerror = 0;
    reps = dpns_listreplica (filename, NULL, CNS_LIST_BEGIN, &rlist);
    if ( reps == NULL )
    {
        error = 1;
        reportComponent (testdesc, "dpns_listreplica does not list one", sstrerror(serrno), 1);
        goto test1_end;
    }
    if ( reps->ptime != 1268014402 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_listreplica returns an unexpected value for ptime", "", 1);
        goto test1_end;
    }
    dpns_listreplica (filename, NULL, CNS_LIST_END, &rlist);

    reportComponent (testdesc, "dpns_setptime works as expected", "", 0);

    test1_end:

        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs1/email/file_setptime.1");
        sprintf (filename, "%s/file_setptime", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



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
    char sfn[256], sfn2[256];
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_delreplica <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_delreplica tests...");

    // Test 1: Delete a primary, permanent, populated replica, while secondary exists
    strcpy (testdesc, "Test 1:Create a file and try register some replicas");
    sprintf (filename, "%s/file_delrep", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn, "%s:/fs1/file_delrep.1", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn, '-', 'P', "pool1", "/fs1", 'P', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot registering replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn2, "%s:/fs2/file_delrep.2", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn2, '-', 'P', "pool1", "/fs1", 'S', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot registering replica (2)", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_delreplica (NULL, &dpns_fid, sfn2);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot remove primary replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_delreplica call completed successfully", "", 0);

    test1_end:
        dpns_delreplica (NULL, NULL, sfn2);
        sprintf (sfn2, "%s:/fs2/file_delrep.2", dpns_host);
        sprintf (sfn, "%s:/fs1/file_delrep.1", dpns_host);
        dpns_delreplica (NULL, NULL, sfn);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



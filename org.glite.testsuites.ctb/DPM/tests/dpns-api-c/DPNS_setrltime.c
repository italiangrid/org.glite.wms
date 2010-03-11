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
        printf ("Usage: DPNS_setrltime <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setrltime tests...");

    // Test 1: Register two replicas and set their access time
    strcpy (testdesc, "Register two replicas and set their access time");
    sprintf (filename, "%s/file_setrltime", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "head64.cern.ch", "head64.cern.ch:/fs1/email/file_setrltime.1", '-', 'V', "file_setrltime", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica", sstrerror(serrno), 1);
        goto test1_end;
    }

    time_t now = time (NULL);
    ret = dpns_setrltime ("head64.cern.ch:/fs1/email/file_setrltime.1", now + 60*60 );
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error setting replica lifetime", sstrerror(serrno), 1);
        goto test1_end;
    }

    int ecount = 0;
    struct dpns_filereplicax * xentries;
    ret = dpns_getreplicax (filename, NULL, NULL, &ecount, &xentries);
    if ( ecount != 1 )
    {
        error = 1;
        reportComponent (testdesc, "Replicas count does not match", sstrerror(serrno), 1);
        if ( ret == 0 )
            free (xentries);
        goto test1_end;
    }
    time_t r_lifetime = xentries[0].ltime;
    free (xentries);

    if ( now + 60*60 != r_lifetime )
    {
        error = 1;
        reportComponent (testdesc, "Set lifetime does not match", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_setrltime ("head64.cern.ch:/fs1/email/file_setrltime.1", now - 60*60 );
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error setting replica lifetime in the past", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filereplicax * xentry;
    dpns_list rlist;
    int count = 0;
    xentry = dpns_listrep4gc ("file_setrltime", CNS_LIST_BEGIN, &rlist);
    if ( xentry == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Replica not listed for GC", sstrerror(serrno), 1);
        goto test1_end;
    }
    if ( strcmp (xentry->sfn, "head64.cern.ch:/fs1/email/file_setrltime.1") != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Found unexpected replica", sstrerror(serrno), 1);
        goto test1_end;
    }
    else
    {
        if ( xentry->ltime != now - 60*60 )
        {
            error = 1;
            reportComponent (testdesc, "Replica for GC found but lifetime does not match", sstrerror(serrno), 1);
            goto test1_end;
        }
    }
    xentry = dpns_listrep4gc ("file_setrltime", CNS_LIST_CONTINUE, &rlist);
    if ( xentry != NULL )
    {
        error = 1;
        reportComponent (testdesc, "Replicas for GC more than expected", sstrerror(serrno), 1);
        goto test1_end;
    }
    dpns_listrep4gc ("file_setrltime", CNS_LIST_END, &rlist);

    test1_end:

        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs1/email/file_setrltime.1");
        sprintf (filename, "%s/file_setrltime", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}


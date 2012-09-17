#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
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

    if ( argc != 4 )
    {
        printf ("Usage: DPNS_getgrpbygid <DPNS_HOST> <BASE_DIR> <MY_GROUP>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group = argv[3];

    setenv ("DPNS_HOST", dpns_host, 1);
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);

    reportHeader ("* Executing dpns_getgrpbygid tests...");

    strcpy (testdesc, "Test 1:Locate my groupname from the id");
    gid_t gid;
    ret = dpns_getgrpbynam (my_group, &gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam retrns an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    char groupname[512];
    memset (groupname, 0, 512);
    ret = dpns_getgrpbygid (gid, groupname);
    if ( ret != 0 )
    {
        if ( serrno != EINVAL )
        {
            error = 1;
            reportComponent (testdesc, "Error during dpns_getgrpbygid", sstrerror (serrno), 1);
            goto test1_end;
        }
    }

    if ( strcmp (my_group, groupname) == 0 )
       reportComponent (testdesc, "dpns_getgrpbygid returned the correct groupname", "", 0);

    test1_end:

    strcpy (testdesc, "Test 2:Call dpns_getgrpbygid with NULL pointer for groupname");
    ret = dpns_getgrpbygid (gid, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 0);
            goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:gid does not exist in table (EINVAL)");
    int i;
    int found = 0;
    char uname[256];
    for ( i = 1; i < 65535; ++i )
    {
        ret = dpns_getgrpbygid (i, uname);
        if ( ret != 0 )
        {
            if ( serrno == EINVAL )
            {
                found = 1;
                sprintf (errorstr, "dpns_getgrpbygid returned with EINVAL for gid=%d", i);
                reportComponent (testdesc, errorstr, "", 0);
                goto test3_end;
            }
            else
            {
                error = 1;
                reportComponent (testdesc, "dpns_getgrpbygid returned with unexpected serrno",  sstrerror(serrno), 1);
                goto test3_end;
            }
        }
    }
    reportComponent (testdesc, "dpns_getgrpbygid could not find unused id.", "WARNING:This may not be an error", 1);

    test3_end:

    strcpy (testdesc, "Test 4:Call dpns_getgrpbygid with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getgrpbygid (gid, groupname);
    if ( ret != 0 )
    {
        if ( serrno == SENOSHOST )
            reportComponent (testdesc, "Returns -1, serrno OK (SENOHOST)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    reportFooter ();
    reportOverall (error);

    return error;
}



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
        printf ("Usage: DPNS_getgrpbynam <DPNS_HOST> <BASE_DIR> <MY_GROUP>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group = argv[3];

    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_getgrpbynam tests...");

    // Test 1:
    strcpy (testdesc, "Test 1:Obtain the GID of my_group");
    gid_t gid;
    ret = dpns_getgrpbynam (my_group, &gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam retrns an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (errorstr, "dpns_gergrpbyname returned 0. GID=%d", gid);
    reportComponent (testdesc, errorstr, "", 0);

    test1_end:

    strcpy (testdesc, "Test 2:Call dpns_getgrpbynam with NULL pointer for groupname");
    ret = dpns_getgrpbynam (NULL, &gid);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns 0. Unexpected.", "", 0);
            goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:Call dpns_getgrpbynam with NULL pointer for gid");
    char groupname[1024];
    ret = dpns_getgrpbynam (groupname, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test3_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns 0. Unexpected.", "", 0);
            goto test3_end;
    }

    test3_end:

    strcpy (testdesc, "Test 4:The length of groupname exceeds 255 (EINVAL)");
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);
    memset (groupname, 70, 1023); groupname[1023] = '\0';
    gid_t gidtemp;
    ret = dpns_getgrpbynam (groupname, &gidtemp);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno is EINVAL", sstrerror(serrno), 0);
            goto test4_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test4_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns 0. Unexpected.", "", 0);
            goto test4_end;
    }

    test4_end:

    strcpy (testdesc, "Test 5:group does not exist in table (EINVAL)");
    ret = dpns_getgrpbynam ("Some really weird groupname around here/there/where", &gidtemp);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno is EINVAL", sstrerror(serrno), 0);
            goto test5_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test5_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbynam returns 0. Unexpected.", "", 0);
            goto test5_end;
    }

    test5_end:

    // Test 6: DPNS_HOST unknown
    strcpy (testdesc, "Test 6:Call dpns_getgrpbynam with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getgrpbynam (my_group, &gidtemp);
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


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

    if ( argc != 5 )
    {
        printf ("Usage: DPNS_getgrpbygids <DPNS_HOST> <BASE_DIR> <MY_GROUP1> <MY_GROUP2>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group1 = argv[3];
    char* my_group2 = argv[4];

    setenv ("DPNS_HOST", dpns_host, 1);
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);

    reportHeader ("* Executing dpns_getgrpbygids tests...");

    strcpy (testdesc, "Test 1:Locate my groupnames from the ids");
    gid_t gids[2];

    ret = dpns_getgrpbynam (my_group1, gids);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam returns an error for group 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_getgrpbynam (my_group2, gids + 1);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam returns an error for group 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    char * groupnames[2];
    ret = dpns_getgrpbygids (2, gids, groupnames);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygids returns an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( ( strcmp ( groupnames[0], my_group1 ) == 0 && strcmp ( groupnames[1], my_group2 ) == 0 )  ||
         ( strcmp ( groupnames[0], my_group2 ) == 0 && strcmp ( groupnames[1], my_group1 ) == 0 ) )
    {
        reportComponent (testdesc, "dpns_getgrpbygids returns OK and names match", "", 0);
    }
    else
    {
        reportComponent (testdesc, "dpns_getgrpbygids returns OK and names don't match", "", 1);
        error = 1;
    }
    free (groupnames[0]);
    free (groupnames[1]);

    test1_end:

    strcpy (testdesc, "Test 2:nbgroups is zero (EINVAL)");
    ret = dpns_getgrpbygids (0, gids, groupnames);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getgrpbygids returned with EINVAL", "", 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returned with unexpected serrno",  sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygids returned 0", "", 1);
    }

    test2_end:

    strcpy (testdesc, "Test 2:nbgroups is zero");


    strcpy (testdesc, "Test 3:One of the GIDs does not exist in the table (EINVAL)");
    int i;
    int found = 0;
    char gname[256];
    for ( i = 1; i < 65535; ++i )
    {
        ret = dpns_getgrpbygid (i, gname);
        if ( ret != 0 )
        {
            if ( serrno == EINVAL )
            {
                found = 1;
                break;
            }
            else
            {
                error = 1;
                reportComponent (testdesc, "dpns_getgrpbygid returned with unexpected serrno",  sstrerror(serrno), 1);
                goto test3_end;
            }
        }
    }

    if ( found == 0 )
    {
        reportComponent (testdesc, "dpns_getgrpbygid could not find unused id. Cannot prepare for scenario.", "WARNING:This may not be an error", 1);
        error = 1;
        goto test3_end;
    }

    int gids2[4];
    gids2[0] = 1; gids2[1] = i; gids2[2] = 130; gids2[3] = 65000;
    char * groupnames2[4];
    ret = dpns_getgrpbygids (4, gids2, groupnames2);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getgrpbygids returned with EINVAL", "", 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returned with unexpected serrno",  sstrerror(serrno), 1);
            goto test3_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygids returned 0", "", 1);
    }
    reportComponent (testdesc, "dpns_getgrpbygid could not find unused id.", "WARNING:This may not be an error", 1);

    test3_end:

    strcpy (testdesc, "Test 4:Call dpns_getgrpbygids with groupsname NULL (SENOSHOST)");
    int gidsi[2]; gidsi[0] = 1; gidsi[1] = 203;
    ret = dpns_getgrpbygids (2, gidsi, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpbygids returns an error. serrno is EFAULT", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returns 0. Unexpected.", "", 0);
    }

    strcpy (testdesc, "Test 5:Call dpns_getgrpbygids with gids NULL (SENOSHOST)");
    char * groupsnamei[2];
    ret = dpns_getgrpbygids (2, NULL, groupsnamei);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpbygids returns an error. serrno is EFAULT", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygids returns 0. Unexpected.", "", 0);
    }

    strcpy (testdesc, "Test 6:Call dpns_getgrpbygid with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getgrpbygids (2, gids, groupnames);
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



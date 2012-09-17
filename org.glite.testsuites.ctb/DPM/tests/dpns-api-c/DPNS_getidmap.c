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
        printf ("Usage: DPNS_getidmap <DPNS_HOST> <BASE_DIR> <MY_GROUP1> <MY_GROUP2>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group1 = argv[3];
    char* my_group2 = argv[4];

    setenv ("DPNS_HOST", dpns_host, 1);
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);

    reportHeader ("* Executing dpns_getidmap tests...");

    strcpy (testdesc, "Test 1:Obtain the uid and gid of the exec user using the subject only");

    char subject[1024];
    ret = getSubject ( getenv ("X509_USER_PROXY"), subject, 1024);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot read the subject from user proxy", NULL, 1);
        goto test1_end;
    }

    uid_t uid = 0;
    gid_t gids[4] = { 0, 0, 0, 0 };

    ret = dpns_getidmap (subject, 0, NULL, &uid, gids);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getidmap returned with error", sstrerror (serrno), 1);
        goto test1_end;
    }

    char userdn[256];
    ret = dpns_getusrbyuid (uid, userdn);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot recheck. dpns_getusrbyuid returned an error.", sstrerror (serrno), 1);
        goto test1_end;
    }

    char group[256];
    ret = dpns_getgrpbygid (gids[0], group);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot recheck. dpns_getgrpbyuid returned an error.", sstrerror (serrno), 1);
        goto test1_end;
    }

    if ( strcmp (subject, userdn) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Returned userdn does not match.", sstrerror (serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_getidmap returned 0. Returned uid OK", sstrerror (serrno), 0);

    test1_end:

    strcpy (testdesc, "Test 2:Obtain the uid and gid of the exec user using the subject and two groups");
    gid_t gid1;
    ret = dpns_getgrpbynam (my_group1, &gid1);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot find gid of MY_GROUP1 with dpns_getgrpbynam", sstrerror (serrno), 1);
        goto test2_end;
    }

    gid_t gid2;
    ret = dpns_getgrpbynam (my_group2, &gid2);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot find gid of MY_GROUP2 with dpns_getgrpbynam", sstrerror (serrno), 1);
        goto test2_end;
    }

    const char * ugroups[2] = { my_group1, my_group2 };
    ret = dpns_getidmap (subject, 2, ugroups, &uid, gids);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getidmap returned an error", sstrerror (serrno), 1);
        goto test2_end;
    }

    if ( strcmp (subject, userdn) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Returned userdn does not match.", sstrerror (serrno), 1);
        goto test2_end;
    }

    if ( ( gid1 == gids[0] && gid2 == gids[1] ) || ( gid1 == gids[1] && gid2 == gids[0] ) )
    {
        reportComponent (testdesc, "dpns_getidmap returned 0. Returned gids match", sstrerror (serrno), 0);
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getidmap returned 0. Returned gids does not match", sstrerror (serrno), 1);
    }

    test2_end:

    strcpy (testdesc, "Test 3:Username is a NULL pointer");
    ret = dpns_getidmap (NULL, 0, NULL, &uid, gids);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EFAULT", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 4:User ID is a NULL pointer");
    ret = dpns_getidmap (subject, 0, NULL, NULL, gids);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EFAULT", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 5:gids is a NULL pointer");
    ret = dpns_getidmap (subject, 0, NULL, &uid, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EFAULT", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 6:nbcount is less than zero");
    ret = dpns_getidmap (subject, -1, ugroups, &uid, gids);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EINVAL", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 7:Subject is more than 255 characters");
    char subjecto[512];
    memset (subjecto, 70, 511); 
    subjecto[511] = '\0';
    ret = dpns_getidmap (subjecto, 2, ugroups, &uid, gids);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EINVAL", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 8:One of the groups is more than 255 characters");
    char ogroup[512];
    memset (ogroup, 70, 511);
    ogroup[511] = '\0';
    const char * ogroups[4] = { "test", "test2", ogroup, "test3" };
    ret = dpns_getidmap (subject, 4, ogroups, &uid, gids);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getidmap returns an error. serrno is EINVAL", sstrerror(serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpbygid returns an error. serrno unexpected.", sstrerror(serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbygid returns 0. Unexpected.", "", 1);
    }

    strcpy (testdesc, "Test 9:Call dpns_getidmap with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getidmap (subject, 0, NULL, &uid, gids);
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



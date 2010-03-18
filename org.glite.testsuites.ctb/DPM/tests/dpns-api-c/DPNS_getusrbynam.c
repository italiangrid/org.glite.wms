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

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_getusrbynam <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_getusrbynam tests...");

    // Test 1:
    strcpy (testdesc, "Test 1:Obtain the UID of the executing user");
    char subject[4096];
    ret = getSubject (getenv ("X509_USER_PROXY"), subject, 4096);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot obtain user subject from certificate", "", 1);
        goto test1_end;
    }

    uid_t uid;
    ret = dpns_getusrbynam (subject, &uid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrbynam retrns an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (errorstr, "dpns_gerusrbyname returned 0. UID=%d", uid);
    reportComponent (testdesc, errorstr, "", 0);

    test1_end:

    strcpy (testdesc, "Test 2:Call dpns_getusrbynam with NULL pointer for username");
    ret = dpns_getusrbynam (NULL, &uid);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns 0. Unexpected.", "", 0);
            goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:Call dpns_getusrbynam with NULL pointer for uid");
    ret = dpns_getusrbynam (subject, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test3_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns 0. Unexpected.", "", 0);
            goto test3_end;
    }

    test3_end:

    strcpy (testdesc, "Test 4:The length of username exceeds 255 (EINVAL)");
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);
    char username[1024];
    memset (username, 70, 1023); username[1023] = '\0';
    uid_t uidtemp;
    ret = dpns_getusrbynam (username, &uidtemp);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno is EINVAL", sstrerror(serrno), 0);
            goto test4_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test4_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns 0. Unexpected.", "", 0);
            goto test4_end;
    }

    test4_end:

    strcpy (testdesc, "Test 5:User does not exist in table (EINVAL)");
    ret = dpns_getusrbynam ("Some really weird username around here/there/where", &uidtemp);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno is EINVAL", sstrerror(serrno), 0);
            goto test5_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test5_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbynam returns 0. Unexpected.", "", 0);
            goto test5_end;
    }

    test5_end:

    // Test 6: DPNS_HOST unknown
    strcpy (testdesc, "Test 6:Call dpns_getusrbynam with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getusrbynam (subject, &uidtemp);
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



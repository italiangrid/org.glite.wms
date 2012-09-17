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
        printf ("Usage: DPNS_getusrbyuid <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);
    char errorbuf[4096];
    dpns_seterrbuf (errorbuf, 4096);

    reportHeader ("* Executing dpns_getusrbyuid tests...");

    strcpy (testdesc, "Test 1:Locate my username from the id");
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

    char username[512];
    memset (username, 0, 512);
    ret = dpns_getusrbyuid (uid, username);
    if ( ret != 0 )
    {
        if ( serrno != EINVAL )
        {
            error = 1;
            reportComponent (testdesc, "Error during dpns_getusrbyuid", sstrerror (serrno), 1);
            goto test1_end;
        }
    }

    if ( strcmp (subject, username) == 0 )
       reportComponent (testdesc, "dpns_getusrbyuid returned the correct username", "", 0);

    test1_end:

    strcpy (testdesc, "Test 2:Call dpns_getusrbyuid with NULL pointer for username");
    ret = dpns_getusrbyuid (uid, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getusrbyuid returns an error. serrno is EFAULT", sstrerror(serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbyuid returns an error. serrno unexpected.", sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
            error = 1;
            reportComponent (testdesc, "dpns_getusrbyuid returns 0. Unexpected.", "", 0);
            goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:UID does not exist in table (EINVAL)");
    int i;
    int found = 0;
    char uname[256];
    for ( i = 1; i < 65535; ++i )
    {
        ret = dpns_getusrbyuid (i, uname);
        if ( ret != 0 )
        {
            if ( serrno == EINVAL )
            {
                found = 1;
                sprintf (errorstr, "dpns_getusrbyuid returned with EINVAL for UID=%d", i);
                reportComponent (testdesc, errorstr, "", 0);
                goto test3_end;
            }
            else
            {
                error = 1;
                reportComponent (testdesc, "dpns_getusrbyuid returned with unexpected serrno",  sstrerror(serrno), 1);
                goto test3_end;
            }
        }
    }
    reportComponent (testdesc, "dpns_getusrbyuid could not find unused id.", "WARNING:This may not be an error", 1);

    test3_end:

    strcpy (testdesc, "Test 4:Call dpns_getusrbyuid with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_getusrbyuid (uid, username);
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



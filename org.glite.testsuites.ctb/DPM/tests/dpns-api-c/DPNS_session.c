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
        printf ("Usage: DPNS_session <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns operations within a session...");

    // Test 1: Start a session
    strcpy (testdesc, "Carry on a dpns session");

    time_t now = time (NULL);
    sprintf (errorstr, "Test session %d", (int)now);
    ret = dpns_startsess (dpns_host, errorstr);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot start session", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (filename, "%s/file_session", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file inside the session", sstrerror(serrno), 1);
        goto test1_end;
    }

    int i;
    for ( i = 0; i < 100; ++i )
    {
        sprintf (dirname, "%s/dir_%d_session", base_dir, i);
        ret = dpns_mkdir (dirname, 0775);
        if ( ret != 0 )
        {
            error = 1;
            sprintf (errorstr, "Error creating directory %d inside the session", i);
            reportComponent (testdesc, errorstr, sstrerror(serrno), 1);
            goto test1_end;
        }
    }

    ret = dpns_endsess ();
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_endsess returned an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    test1_end:
        sprintf (filename, "%s/file_session", base_dir);
        dpns_unlink (filename);
        for ( i = 0; i < 100; ++i )
        {
            sprintf (dirname, "%s/dir_%d_session", base_dir, i);
            dpns_rmdir (dirname);
        }

    reportFooter ("");
    reportOverall (error);

    return error;
}



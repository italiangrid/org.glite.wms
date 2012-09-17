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
    strcpy (testdesc, "Test 1: Start a DPNS session");

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

    // Change access rights to an existing object and replace the X509_USER_PROXY with another
    // proxy which should not have access to the object. Verify the we can still access it 
    // (the session had been authenticated in the start).

    ret = dpns_chmod (filename, 0700);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error executing dpns_chown", sstrerror (serrno), 1);
        goto test1_end;
    }

    char* proxyl =  getenv ("X509_USER_PROXY");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access primary proxy", "", 1);
        goto test1_end;
    }
    char proxyp[PATH_MAX+1];
    if ( strcpy (proxyp, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store primary proxy location", "", 1);
        goto test1_end;
    }

    char proxys[PATH_MAX+1];
    proxyl = getenv ("X509_USER_PROXY_2");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access secondary proxy", "", 1);
        goto test1_end;
    }

    if ( strcpy (proxys, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store secondary proxy location", "", 1);
        goto test1_end;
    }

    ret = setenv ("X509_USER_PROXY", proxys, 1);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error setting new value for X509_USER_PROXY", "", 1);
        goto test1_end;
    }

    reportComponent (testdesc, "Session started OK. dpns_creat and dpns_mkdir operations OK", "", 0);
    strcpy (testdesc, "Test 2: Authentication marker check (1)");

    ret = dpns_access (filename, R_OK);
    if ( ret != 0 && serrno == EACCES )
    {
        error = 1;
        reportComponent (testdesc, "dpns_access executed with the credentials of X509_USER_PROXY_2", "", 1);
        goto test1_px;
    }
    else
    {
        if ( ret == 0 )
        {
            reportComponent (testdesc, "dpns_access executed without new authentication (we are in session)", "", 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_access exited with an unexpected error", sstrerror (serrno), 1);
            goto test1_px;
        }
    }

    ret = dpns_endsess ();
    if ( ret != 0 )
    {
        error = 1;
        reportComponent ("Test 3: dpns_endsess exit code", "dpns_endsess returned an error", sstrerror(serrno), 1);
        goto test1_px;
    }
    else
        reportComponent ("Test 3: dpns_endsess exit code", "dpns_endsess OK", "", 0);

    strcpy (testdesc, "Test 4: Authentication marker check (2)");
    ret = dpns_access (filename, R_OK);
    if ( ret != 0 && serrno == EACCES )
    {
        reportComponent (testdesc, "Access denied (expected; we are outside the session scope)", "", 0);
        goto test1_px;
    }
    else
    {
        if ( ret == 0 )
        {
            error = 1;
            reportComponent (testdesc, "Access allowed (unexpected; we are outside the session scope)", "", 0);
            goto test1_px;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_access unexpected error", sstrerror (serrno), 1);
            goto test1_px;
        }
    }

    test1_px:
        setenv ("X509_USER_PROXY", proxyp, 1);

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



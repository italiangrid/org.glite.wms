#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>
#include <limits.h>
#include <unistd.h>

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

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_chmod <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_chmod tests...");

    // Test 1: Create a subdirectory in <BASE_DIR> and perform chdir to it
    strcpy (testdesc, "Test 1:Perform chmod to a newly created directory");
    sprintf (dirname, "%s/dir_chmod", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create test directory dir_chmod", sstrerror(serrno), 1);
        goto test1_end;
    }
    else
    {
        ret = dpns_chmod (dirname, 0700);

        if ( ret != 0 )
        {
            error = 1;
            reportComponent (testdesc, "dpns_chmod call failed for dir_chmod", sstrerror(serrno), 1);
            goto test1_end;
        }

        // Create a file in dir_chmod
        sprintf (filename, "%s/dir_chmod/file_chmod", base_dir);
        ret = dpns_creat (filename, 0664);
        if ( ret != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Error creating file in the owned dir_chmod (0700)", sstrerror(serrno), 1);
            goto test1_end;
        }

        ret = dpns_chmod (filename, 0600);
        if ( ret != 0 )
        {
            error = 1;
            reportComponent (testdesc, "dpns_chmod call fialed for file_chmod", sstrerror(serrno), 1);
            goto test1_end;
        }

        // Impersonate as another user (which is not an owner) and test access
        envvar = getenv ("X509_USER_PROXY");
        if ( envvar == NULL )
        {
            error = 1;
            reportComponent (testdesc, "Cannot find proxy location in X509_USER_PROXY", "", 1);
            goto test1_end;
        }
        strcpy (cert1, envvar);

        envvar = getenv ("X509_USER_PROXY_2");
        if ( envvar == NULL )
        {
            error = 1;
            reportComponent (testdesc, "Cannot find secondary proxy location in X509_USER_PROXY_2", "", 1);
            goto test1_end;
        }
        strcpy (cert2, envvar);

        if ( access (cert2, R_OK) )
        {
            error = 1;
            reportComponent (testdesc, "Cannot access secondary proxy", "", 1);
            goto test1_end;
        }

        sprintf (filename, "%s/dir_chmod/file_chmod", base_dir);
        ret = dpns_access (filename, R_OK);

        setenv ("X509_USER_PROXY", cert2, 1);

        sprintf (filename, "%s/dir_chmod/file_chmod", base_dir);
        ret = dpns_access (filename, R_OK);
        if ( ret == 0 || ( ret != 0 && serrno != EACCES ) )
        {
            error = 1;
            sprintf (errorstr, "serrno is %d", serrno);
            reportComponent (testdesc, "User 2 has access (unexpected)", ( ret == 0 ? "" : errorstr), 1);
            goto test1_end;
        }

        setenv ("X509_USER_PROXY", cert1, 1);
        reportComponent (testdesc, "dpns_chmod call successfull", "", 0);
    }

    test1_end:

        sprintf (filename, "%s/dir_chmod/file_chmod", base_dir);
        dpns_unlink (filename);
        sprintf (dirname, "%s/dir_chmod", base_dir);
        dpns_rmdir (dirname);

    reportFooter ("");
    reportOverall (error);

    return error;
}


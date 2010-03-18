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
        printf ("Usage: DPNS_access <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_access tests...");

    strcpy (testdesc, "Environment preparation");

    // Get the certificate subject of the secondary user
    char subject[PATH_MAX];
    ret = getSubject ( getenv ("X509_USER_PROXY_2"), subject, PATH_MAX );
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot read the subject from the proxy", NULL, 1);
        goto test11_end;
    }

    // Locate the UID using the subject
    uid_t uid;
    ret = dpns_getusrbynam (subject, &uid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrbynam cannot locate uid", sstrerror(serrno), 1);
        goto test11_end;
    }

    // Switch the identity in use
    char* proxyl =  getenv ("X509_USER_PROXY");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access primary proxy", "", 1);
        goto test11_end;
    }
    char proxyp[PATH_MAX+1];
    if ( strcpy (proxyp, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store primary proxy location", "", 1);
        goto test11_end;
    }

    proxyl = getenv ("X509_USER_PROXY_2");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access secondary proxy", "", 1);
        goto test11_end;
    }
    char proxys[PATH_MAX+1];
    if ( strcpy (proxys, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store secondary proxy location", "", 1);
        goto test11_end;
    }

    // Prepare a directory with mode 700
    sprintf (dirname, "%s/dpns_access", base_dir);
    ret = dpns_mkdir (dirname, 0700);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create directory dir_access_1", sstrerror(serrno), 1);
        goto test11_end;
    }

    // Test 1: Check whether the owner has access as expected
    strcpy (testdesc, "Test 1:Directory access mode 700, owner access check");
    ret = dpns_access (dirname, R_OK | W_OK | X_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            error = 1;
            reportComponent (testdesc, "Returns -1, Unexpectedly access is denied", sstrerror (serrno), 1);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected error", sstrerror (serrno), 1);
        }
    }
    else
        reportComponent (testdesc, "Returns 0. Access allowed as expected", "", 0);


    // Test 2: Check whether the secondary proxy is not allowed access (as expected)
    strcpy (testdesc, "Test 2:Directory access mode 700, non-owner access check");

    if ( setenv ("X509_USER_PROXY", proxys, 1) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot setenv the secondary proxy location", "", 1);
        goto test2_end;
    }

    ret = dpns_access (dirname, R_OK | W_OK | X_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            reportComponent (testdesc, "Returns -1, serrno is EACCES (as expected)", sstrerror (serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror (serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "Returns 0. Access allowed which is not expected", "", 1);
    }

    test2_end:

        if ( setenv ("X509_USER_PROXY", proxyp, 1) != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Error restoring primary proxy environment", "", 1);
            goto test11_end;
        }

    // Test 3: Add an ACL entry for the 
    strcpy (testdesc, "Test 3:Access mode 700, Check for non-owner access with additional ACE");

    struct dpns_acl list[8];

    list[0].a_type = CNS_ACL_USER_OBJ;
    list[0].a_id = 0;
    list[0].a_perm = 7;

    list[1].a_type = CNS_ACL_GROUP_OBJ;
    list[1].a_id = 0;
    list[1].a_perm = 0;

    list[2].a_type = CNS_ACL_OTHER;
    list[2].a_id = 0;
    list[2].a_perm = 0;

    list[3].a_type = CNS_ACL_USER;
    list[3].a_id = uid;
    list[3].a_perm = 6;

    list[4].a_type = CNS_ACL_MASK;
    list[4].a_id = 0;
    list[4].a_perm = 7;

    list[5].a_type = CNS_ACL_DEFAULT | CNS_ACL_USER_OBJ;
    list[5].a_id = 0;
    list[5].a_perm = 7;

    list[6].a_type = CNS_ACL_DEFAULT | CNS_ACL_GROUP_OBJ;
    list[6].a_id = 0;
    list[6].a_perm = 7;

    list[7].a_type = CNS_ACL_DEFAULT | CNS_ACL_OTHER;
    list[7].a_id = 0;
    list[7].a_perm = 5;

    ret = dpns_setacl (dirname, 8, list);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setacl call failed", sstrerror(serrno), 1);
        goto test3_end;
    }

    // Switch the executing user

    if ( setenv ("X509_USER_PROXY", proxys, 1) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot setenv the secondary proxy location", "", 1);
        goto test3_end;
    }

    ret = dpns_access (dirname, R_OK | W_OK );
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            error = 1;
            reportComponent (testdesc, "Returns -1, EACCES, for secondary (R_OK | W_OK)", sstrerror (serrno), 1);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror (serrno), 1);
        }
    }
    else
    {
        ret = dpns_access (dirname, X_OK );
        if ( ret != 0 )
        {
            if ( serrno == EACCES )
            {
                reportComponent (testdesc, "R_OK and W_OK allowed; X_OK denied", sstrerror (serrno), 0);
            }
            else
            {
                error = 1;
                reportComponent (testdesc, "dpns_access for X_OK returns -1 with unexpected serrno", sstrerror (serrno), 1);
            }
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "R_OK and W_OK allowed (expected); X_OK allowed (unexpected)", "", 1);
        }
    }

    test3_end:

        if ( setenv ("X509_USER_PROXY", proxyp, 1) != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Error restoring primary proxy environment", "", 1);
            goto test11_end;
        }

    // Test 4: Execute for non-existing directory
    strcpy (testdesc, "Test 4:Execute for non-existing directory(ENOENT)");
    ret = dpns_access ("/dir/which/does/not/exists", R_OK);
    if ( ret != 0 )
    {
        if ( serrno == ENOENT )
            reportComponent (testdesc, "returns -1, ENOENT", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    // Test 5: Call with NULL argument
    strcpy (testdesc, "Test 5:Call with NULL argument(EFAULT)");
    ret = dpns_access (NULL, R_OK);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
            reportComponent (testdesc, "Returns -1. Serrno OK", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    //Test 6: dpns_access for a directory which name is exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 6:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    dirname[CA_MAXPATHLEN + 1] = '\0';
    ret = dpns_access (dirname, R_OK);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            reportComponent (testdesc, "Returns -1 (ENAMETOOLONG)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }
    dpns_unlink (filename);
    sprintf (dirname, "%s/dpns_access", base_dir);

    // Test 7: Call with NULL argument
    strcpy (testdesc, "Test 7:Call with invalid amode(EINVAL)");
    ret = dpns_access (dirname, -4100);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
            reportComponent (testdesc, "Returns -1. Serrno OK", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    // Test 8: Component of the path is not a directory
    strcpy (testdesc, "Test 8:Component of the path is not a directory(ENOTDIR)");
    sprintf (filename, "%s/file", dirname);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file", sstrerror(serrno), 1);
        goto test8_end;
    }
    char location[1024];
    sprintf (location, "%s/file/x/y/z", dirname);
    ret = dpns_access (location, R_OK);
    if ( ret != 0 )
    {
        if ( serrno == ENOTDIR )
            reportComponent (testdesc, "Returns -1, serrno OK (ENOTDIR)", sstrerror(serrno), 0);
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
    test8_end:
        dpns_unlink (filename);

    // Test 9: dpns_access for a file in inaccessible directory (permission is not granted)
    strcpy (testdesc, "Test 9:dpns_access access denied scenario (EACCES)");
    char dirname2[CA_MAXPATHLEN];
    sprintf (dirname2, "%s/dpns_access_denied", base_dir);
    sprintf (filename, "%s/dpns_access_denied/file", base_dir);

    ret = dpns_mkdir (dirname2, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating inaccessible directory", sstrerror(serrno), 1);
        goto test9_end;
    }

    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file in the inaccessible directory", sstrerror(serrno), 1);
        goto test9_end;
    }

    ret = dpns_chmod (dirname2, 0);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error setting directory permissions", "", 1);
        goto test9_end;
    }

    ret = dpns_access (filename, R_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            reportComponent (testdesc, "Returns -1", sstrerror(serrno), 0);
        }
        else
        {
            reportComponent (testdesc, "Returns -1", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    test9_end:
        dpns_chmod (dirname2, 0770);
        dpns_unlink (filename);
        dpns_rmdir (dirname2);

    // Test 10: DPNS_HOST unknown
    strcpy (testdesc, "Test 10:Call dpns_access with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_access (dirname, R_OK);
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

    test11_end:

        setenv ("DPNS_HOST", dpns_host, 1);
        sprintf (dirname, "%s/dpns_access", base_dir);
        dpns_rmdir (dirname);

    reportFooter ("");
    reportOverall (error);

    return error;
}

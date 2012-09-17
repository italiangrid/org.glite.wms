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
        printf ("Usage: DPNS_accessr <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_accessr tests...");

    strcpy (testdesc, "Environment preparation");

    // Get the certificate subject of the secondary user
    char subject[PATH_MAX];
    ret = getSubject ( getenv ("X509_USER_PROXY_2"), subject, PATH_MAX );
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot read the subject from the proxy", NULL, 1);
        goto testall_end;
    }

    // Locate the UID using the subject
    uid_t uid;
    ret = dpns_getusrbynam (subject, &uid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrbynam cannot locate uid", sstrerror(serrno), 1);
        goto testall_end;
    }

    // Switch the identity in use
    char* proxyl =  getenv ("X509_USER_PROXY");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access primary proxy", "", 1);
        goto testall_end;
    }
    char proxyp[PATH_MAX+1];
    if ( strcpy (proxyp, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store primary proxy location", "", 1);
        goto testall_end;
    }

    proxyl = getenv ("X509_USER_PROXY_2");
    if ( proxyl == NULL || (proxyl != NULL && access (proxyl, R_OK) != 0 ) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot access secondary proxy", "", 1);
        goto testall_end;
    }
    char proxys[PATH_MAX+1];
    if ( strcpy (proxys, proxyl) == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot store secondary proxy location", "", 1);
        goto testall_end;
    }

    // Create a file with access mode 700
    sprintf (filename, "%s/dpns_accessr", base_dir);
    ret = dpns_creatx (filename, 0700, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dpns_accessr", sstrerror(serrno), 1);
        goto testall_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, dpns_host, "head64.cern.ch:/fs1/email/filereplica.1", '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot add replica", sstrerror (serrno), 1);
        goto testall_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, dpns_host, "head64.cern.ch:/fs2/email/filereplica.2", '-', 'V', "pool2", "/fs2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot add replica", sstrerror (serrno), 1);
        goto testall_end;
    }

    // Test 1: Check whether the owner has access as expected
    strcpy (testdesc, "Test 1:Directory access mode 700, permanent replica, owner access check");
    ret = dpns_accessr ("head64.cern.ch:/fs1/email/filereplica.1", R_OK | X_OK);
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

    // Test 2: Check whether the owner has access as expected
    strcpy (testdesc, "Test 2:Directory access mode 700, volatile replica, owner access check");
    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", R_OK | X_OK);
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

    // Test 3: Check whether the owner is not allowed write access to replicas (file immutability)
    strcpy (testdesc, "Test 3:Owner write access (file immutability)");
    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", W_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            reportComponent (testdesc, "Returns -1, EACCES expected, replica status is '-'", sstrerror (serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected error", sstrerror (serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "Returns 0. Access allowed (not expected, replica status is '-')", "", 1);
    }

    if ( setenv ("X509_USER_PROXY", proxys, 1) != 0 )
    {
        error = 1;
        reportComponent ("Test 3:Directory access mode 700, permanent replica, non-owner access check", \
                         "Cannot setenv the secondary proxy location", "", 1);
        reportComponent ("Test 4:Directory access mode 700, volatile replica, non-owner access check", \
                         "Cannot setenv the secondary proxy location", "", 1);
        goto test45_end;
    }

    // Test 4: Check whether the secondary proxy is not allowed access (as expected)
    strcpy (testdesc, "Test 4:Directory access mode 700, permanent replica, non-owner access check");

    ret = dpns_accessr ("head64.cern.ch:/fs1/email/filereplica.1", R_OK | X_OK);
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

    // Test 5: Check whether the secondary proxy is not allowed access (as expected)
    strcpy (testdesc, "Test 5:Directory access mode 700, volatile replica, non-owner access check");

    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", R_OK | X_OK);
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

    test45_end:

        if ( setenv ("X509_USER_PROXY", proxyp, 1) != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Error restoring primary proxy environment", "", 1);
            goto testall_end;
        }

    // Test 6: Add an ACL entry for the secondary user (non-owner access via custom ACE)
    strcpy (testdesc, "Test 6:Access mode 700, Check for non-owner access with additional ACE");

    struct dpns_acl list[5];

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
    list[3].a_perm = 7;

    list[4].a_type = CNS_ACL_MASK;
    list[4].a_id = 0;
    list[4].a_perm = 7;

    ret = dpns_setacl (filename, 5, list);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setacl call failed", sstrerror(serrno), 1);
        goto test678_end;
    }

    // Switch the executing user

    if ( setenv ("X509_USER_PROXY", proxys, 1) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot setenv the secondary proxy location", "", 1);
        goto test678_end;
    }

    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", R_OK | X_OK );
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            error = 1;
            reportComponent (testdesc, "Returns -1, EACCES, for secondary (R_OK | X_OK)", sstrerror (serrno), 1);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror (serrno), 1);
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0. RX access allowed", "", 0);
    }

    // Test 7: Check write access for secondary user (non-owner rwx access via custom ACE)
    strcpy (testdesc, "Test 7:Access mode 700, Check for non-owner W_OK access via custom ACE");

    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", W_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            reportComponent (testdesc, "Returns -1, EACCES, W_OK denied (not populating)", sstrerror (serrno), 0);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_accessr for W_OK returns -1 with unexpected serrno", sstrerror (serrno), 1);
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "W_OK allowed although not populating (unexpected)", "", 1);
    }

    // Test 8: Check write access for secondary user (non-owner rwx access via custom ACE)
    strcpy (testdesc, "Test 8:Access mode 700, Check for non-owner F_OK access via custom ACE");

    ret = dpns_accessr ("head64.cern.ch:/fs2/email/filereplica.2", F_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            error = 1;
            reportComponent (testdesc, "Returns -1, EACCES, F_OK denied", sstrerror (serrno), 1);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_accessr for F_OK returns -1 with unexpected serrno", sstrerror (serrno), 1);
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0, F_OK", "", 0);
    }

    test678_end:

        if ( setenv ("X509_USER_PROXY", proxyp, 1) != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Error restoring primary proxy environment", "", 1);
            goto testall_end;
        }


    // Test 9: Execute for non-existing directory
    strcpy (testdesc, "Test 9:Execute for non-existing directory(ENOENT)");
    ret = dpns_accessr ("/dir/which/does/not/exists", R_OK);
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

    // Test 10: Call with NULL argument
    strcpy (testdesc, "Test 10:Call with NULL argument(EFAULT)");
    ret = dpns_accessr (NULL, R_OK);
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

    //Test 11: dpns_access for a directory which name is exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 11:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    char sfnname[CA_MAXSFNLEN + 2];
    memset (sfnname, 61, CA_MAXSFNLEN + 1);
    sfnname[CA_MAXSFNLEN + 1] = '\0';
    ret = dpns_accessr (sfnname, R_OK);
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

    sprintf (dirname, "%s/dpns_access", base_dir);

    // Test 12: Call with invalid mode
    strcpy (testdesc, "Test 12:Call with invalid amode(EINVAL)");
    ret = dpns_accessr (dirname, -4100);
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

    // Test 13: dpns_accessr for a file in inaccessible directory (permission is not granted)
    strcpy (testdesc, "Test 13:dpns_access access denied scenario (EACCES)");
    char dirname2[CA_MAXPATHLEN];
    sprintf (dirname2, "%s/dpns_access_denied", base_dir);
    sprintf (filename, "%s/dpns_access_denied/file", base_dir);

    ret = dpns_mkdir (dirname2, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating inaccessible directory", sstrerror(serrno), 1);
        goto test13_end;
    }

    struct dpns_fileid dpns_fid2;
    ret = dpns_creatx (filename, 0664, &dpns_fid2);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file in the inaccessible directory", sstrerror(serrno), 1);
        goto test13_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid2, dpns_host, "head64.cern.ch:/fs3/email/filereplica.3", '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating replica", sstrerror(serrno), 1);
        goto test13_end;
    }

    ret = dpns_chmod (dirname2, 0);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error setting directory permissions", "", 1);
        goto test13_endr;
    }

    ret = dpns_accessr ("head64.cern.ch:/fs3/email/filereplica.3", R_OK);
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

    test13_endr:
        dpns_chmod (dirname2, 0770);
        
        dpns_delreplica (NULL, &dpns_fid2, "head64.cern.ch:/fs3/email/filereplica.3");
    test13_end:
        dpns_chmod (dirname2, 0770);
        dpns_unlink (filename);
        dpns_rmdir (dirname2);

    // Test 14: DPNS_HOST unknown
    strcpy (testdesc, "Test 14:Call dpns_access with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_accessr (dirname, R_OK);
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

    setenv ("DPNS_HOST", dpns_host, 1);
    // Test 15: Check W_OK for a replica which is being populated
    strcpy (testdesc, "Test 15:Check W_OK for a replica which is being populated");
    ret = dpns_setrstatus ("head64.cern.ch:/fs1/email/filereplica.1", 'P');
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot set replica status to 'P'", sstrerror (serrno), 1);
        goto test15_end;
    }

    ret = dpns_accessr ("head64.cern.ch:/fs1/email/filereplica.1", W_OK);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            error = 1;
            reportComponent (testdesc, "Returns -1, EACCES not expected (status is 'P')", sstrerror (serrno), 1);
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Returns -1. Unexpected error", sstrerror (serrno), 1);
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0. Access allowed as expected (status is 'P')", "", 0);
    }
    

    test15_end:

    testall_end:

        setenv ("DPNS_HOST", dpns_host, 1);
        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs1/email/filereplica.1");
        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs2/email/filereplica.2");
        sprintf (filename, "%s/dpns_accessr", base_dir);
        dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}

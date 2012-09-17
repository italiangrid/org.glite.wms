#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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

    if ( argc != 5 )
    {
        printf ("Usage: DPNS_chown <DPNS_HOST> <BASE_DIR> <MY_GROUP> <FOREIGN_GROUP>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group = argv[3];
    char* foreign_group = argv[4];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_chown tests...");

    // Test 1: Create a subdirectory in <BASE_DIR> and try change the owner user.
    //         dpns_chown should fail with EPERM (We are not an admin)
    strcpy (testdesc, "Test 1:Create a directory and try change the owner user");
    sprintf (dirname, "%s/dir_chown", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create test directory dir_chown", sstrerror(serrno), 1);
        goto test1_end;
    }

    // Read the user's subject from the pem file
    char subject[1024];
    ret = getSubject ( getenv ("X509_USER_PROXY_2"), subject, 1024);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot read the subject from user proxy", NULL, 1);
        goto test1_end;
    }

    // Locate the UID using the subject
    uid_t uid;
    ret = dpns_getusrbynam (subject, &uid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrbynam cannot locate uid", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_chown (dirname, uid, -1);
    if ( ret == 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_chown succeeded (unexpected)", NULL, 1);
        goto test1_end;
    }
    if ( ret != 0 && serrno != EPERM )
    {
        error = 1;
        reportComponent (testdesc, "dpns_chown failed but the error code is not EPERM", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_chown failed with EPERM (as expected)", NULL, 0);

    test1_end:

        sprintf (dirname, "%s/dir_chown", base_dir);
        dpns_rmdir (dirname);


    // Test 2: Try changing the owner group with one we are a member of
    //         dpns_chown should succeed
    strcpy (testdesc, "Test 2:Change the owning group with one we are a member of");
    sprintf (dirname, "%s/dir_chown", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create test directory dir_chown", sstrerror(serrno), 1);
        goto test2_end;
    }

    gid_t gid;
    ret = dpns_getgrpbynam (my_group, &gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam cannot locate the gid", sstrerror(serrno), 1);
        goto test2_end;
    }
                                            
    ret = dpns_chown (dirname, -1, gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_chown failed", sstrerror(serrno), 1);
        goto test2_end;
    }

    reportComponent (testdesc, "dpns_chown succeeded", NULL, 0);

    test2_end:

        sprintf (dirname, "%s/dir_chown", base_dir);
        dpns_rmdir (dirname);


    // Test 3: Try changing the owner group with one we are not a member of
    //         dpns_chown should fail
    strcpy (testdesc, "Test 3:Change the owning group with one we are not a member of");
    sprintf (dirname, "%s/dir_chown", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create test directory dir_chown", sstrerror(serrno), 1);
        goto test3_end;
    }

    ret = dpns_getgrpbynam (foreign_group, &gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpbynam cannot locate the gid", sstrerror(serrno), 1);
        goto test3_end;
    }

    ret = dpns_chown (dirname, -1, gid);
    if ( ret == 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_chown succeeded (unexpected)", NULL, 1);
        goto test1_end;
    }
    if ( ret != 0 && serrno != EPERM )
    {
        error = 1;
        reportComponent (testdesc, "dpns_chown failed but the error code is not EPERM", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_chown failed with EPERM (as expected)", NULL, 0);

    test3_end:

        sprintf (dirname, "%s/dir_chown", base_dir);
        dpns_rmdir (dirname);


    reportFooter ("");
    reportOverall (error);

    return error;
}



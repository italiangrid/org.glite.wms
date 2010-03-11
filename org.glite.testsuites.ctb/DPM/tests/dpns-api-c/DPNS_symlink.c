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
    char symname[CA_MAXPATHLEN + 2];
    char * envvar;
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_setratime <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_symlink tests...");

    // Test 1: Create directory and symlink to it
    strcpy (testdesc, "Test 1:Create directory and symlink to it");
    sprintf (dirname, "%s/test_dir", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create target directory", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (symname, "%s/dpns_symlink", base_dir);
    ret = dpns_symlink (dirname, symname);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create symlink to a directory", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_readlink (symname, filename, CA_MAXPATHLEN);
    if ( strcmp (dirname, filename) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Symlink verification failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "Symlink to directory verified successfully", "", 0);

    test1_end:

        dpns_unlink (symname);
        dpns_rmdir (dirname);

    // Test 2: Create file and symlink to it
    strcpy (testdesc, "Test 2:Create a file and symlink to it");
    sprintf (filename, "%s/test_file", base_dir);
    ret = dpns_creat (filename, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create target file", sstrerror(serrno), 1);
        goto test2_end;
    }

    sprintf (symname, "%s/dpns_symlink", base_dir);
    ret = dpns_symlink (filename, symname);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create symlink to a file", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_readlink (symname, dirname, CA_MAXPATHLEN);
    if ( strcmp (dirname, filename) != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Symlink verification failed", sstrerror(serrno), 1);
        goto test2_end;
    }

    reportComponent (testdesc, "Symlink to file verified successfully", "", 0);

    test2_end:

        dpns_unlink (symname);
        dpns_unlink (filename);

    // Test 3: Execute for non-existing directory
    strcpy (testdesc, "Test 3:Execute for non-existing directory(ENOENT)");
    ret = dpns_symlink ("/dpm", "/location/which/does/not/exist");
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

    // Test 4: Call dpns_symlink with NULL argument
    strcpy (testdesc, "Test 4:Call dpns_symlink with NULL argument(EFAULT)");
    ret = dpns_symlink (NULL, NULL);
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

    // Test 5: dpns_symlink for a file in inaccessible directory (permission is not granted)
    strcpy (testdesc, "Test 5:dpns_utime access denied scenario (EACCES)");
    sprintf (dirname, "%s/dpns_symlink_access", base_dir);
    sprintf (filename, "%s/dpns_symlink_access/symlink", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret == 0 )
    {
        ret = dpns_chmod (dirname, 0);
        if ( ret == 0 )
        {
            ret = dpns_symlink ("/dpm", filename);
            if ( ret != 0 )
            {
                if ( serrno == EACCES )
                    reportComponent (testdesc, "Returns -1", sstrerror(serrno), 0);
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
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Error setting directory permissions", "", 1);
        }
    }
    else
    {
        reportComponent (testdesc, "Error creating inaccessible directory", sstrerror(serrno), 1);
        error = 1;
    }
    dpns_chmod (dirname, 0770);
    dpns_unlink (filename);
    dpns_rmdir (dirname);

    //Test 6: dpns_symlink for a directory which name is exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 6:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    sprintf (filename, "%s/symlink_maxpathlen", base_dir);
    dirname[CA_MAXPATHLEN + 1] = '\0';
    ret = dpns_symlink (dirname, filename);
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

    // Test 7: Component of the path is not a directory
    strcpy (testdesc, "Test 7:Component of the path is not a directory(ENOTDIR)");
    sprintf (dirname, "%s/file", base_dir);
    sprintf (filename, "%s/file/file", base_dir);
    ret = dpns_creat (dirname, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file", sstrerror(serrno), 1);
        goto test7_end;
    }
    ret = dpns_symlink ("/dpm", filename);
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
    test7_end:
        dpns_unlink (filename);
        dpns_unlink (dirname);

    // Test 8: Entry already exists (EEXIST)
    strcpy (testdesc, "Test 8:Entry already exists (EEXIST)");
    sprintf (filename, "%s/exfile", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Error creating file base_dir/exfile", sstrerror(serrno), 1);
        error = 1;
    }
    else
    {
        ret = dpns_symlink ("/dpm", filename);
        if ( ret != 0 )
        {
            if ( serrno == EEXIST )
                reportComponent (testdesc, "Returns -1, serrno OK (EEXIST)", sstrerror(serrno), 0);
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
        dpns_unlink (filename);
    }

    // Test 9: DPNS_HOST unknown
    strcpy (testdesc, "Test 9:Call dpns_symlink with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    sprintf (filename, "%s/file", base_dir);
    ret = dpns_symlink ("/dpm", filename);
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
    dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



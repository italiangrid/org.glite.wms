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

    if ( argc != 4 )
    {
        printf ("Usage: DPNS_mkdir <DPNS_HOST> <BASE_DIR> <MY_GROUP>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* my_group = argv[3];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_mkdir tests...");

    // Create a directory with mode 775
    strcpy (testdesc, "Test 1:Create a test directory");
    sprintf (dirname, "%s/dpns_mkdir", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_mkdir returned an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filestat dirstat;
    ret = dpns_stat (dirname, &dirstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_mkdir returned 0. Cannot stat created directory for inspection", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( dirstat.filemode && S_IFDIR == 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_mkdir OK. dpns_stat OK. filemode does not denote a directory", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_mkdir OK. dpns_stat OK", sstrerror(serrno), 0);

    test1_end:

        dpns_rmdir (dirname);

    strcpy (testdesc, "Test 2:Create a directory inside another with S_ISGID set");
    sprintf (dirname, "%s/mkdir_base", base_dir);
    sprintf (filename, "%s/mkdir_base/dpns_mkdir", base_dir);

    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create base directory", sstrerror(serrno), 1);
        goto test2_end;
    }

    gid_t ggid;
    ret = dpns_getgrpbynam (my_group, &ggid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot get gid for auxiliary group", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_chown (dirname, -1, ggid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot change owner group for base", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_chmod (dirname, S_ISGID | 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot set S_ISGID for base", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_mkdir (filename, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create test directory", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_stat (filename, &dirstat);
    if ( dirstat.gid != ggid )
    {
        error = 1;
        reportComponent (testdesc, "Test directory created OK, owner group gid does not match", sstrerror(serrno), 1);
        goto test2_end;
    }

    reportComponent (testdesc, "Test directory created OK and owner group matches", "", 0);

    test2_end:
        dpns_rmdir (filename);
        dpns_rmdir (dirname);

    // Test 3: Execute for non-existing directory
    strcpy (testdesc, "Test 3:Execute for non-existing directory (ENOENT)");
    ret = dpns_mkdir ("/directory/which/does/not/exist", 0775);
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

    strcpy (testdesc, "Test 4:Directory already exists (EEXIST)");
    ret = dpns_mkdir (dirname, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_mkdir cannot create initial directory", sstrerror(serrno), 1);
    }
    else
    {
        ret = dpns_mkdir (dirname, 0664);
        if ( ret != 0 )
        {
            if ( serrno == EEXIST )
                 reportComponent (testdesc, "dpns_mkdir error. serrno is EEXIST", sstrerror(serrno), 0);
            else
            {
                reportComponent (testdesc, "dpns_mkdir sets wrong serrno", sstrerror(serrno), 1);
                error = 1;
            }
        }
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns 0", sstrerror(serrno), 1);
            error = 1;
        }
    }
    dpns_rmdir(dirname);

    strcpy (testdesc, "Test 3:Path is NULL (EFAULT)");
    ret = dpns_mkdir (NULL, 0775);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
            reportComponent (testdesc, "returns -1, EFAULT", sstrerror(serrno), 0);
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

    strcpy (testdesc, "Test 4:Path is exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            reportComponent (testdesc, "dpns_mkdir returns -1 (ENAMETOOLONG)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
        error = 1;
    }

    strcpy (testdesc, "Test 5:Path component is exceeding CA_MAXNAMELEN(ENAMETOOLONG)");
    memset (filename, 70, 260);
    filename[260] = '\0';
    sprintf (dirname, "%s/%s", base_dir, filename);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            reportComponent (testdesc, "dpns_mkdir returns -1 (ENAMETOOLONG)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
        error = 1;
    }

    strcpy (testdesc, "Test 6:Path component is not a directory (ENOTDIR)");
    sprintf (filename, "%s/dpns_mkdir_file", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create temp file", sstrerror (serrno), 1);
    }
    else
    {
        sprintf (dirname, "%s/dpns_mkdir_file/dpns_mkdir_directory", base_dir);
        ret = dpns_mkdir (dirname, 0775);
        if ( ret != 0 )
        {
            if ( serrno == ENOTDIR )
                reportComponent (testdesc, "dpns_mkdir returns -1 (ENOTDIR)", sstrerror(serrno), 0);
            else
            {
                reportComponent (testdesc, "dpns_mkdir returns -1. Unexpected serrno", sstrerror(serrno), 1);
                error = 1;
            }
        }
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
            error = 1;
        }
    }
    dpns_unlink (filename);

    strcpy (testdesc, "Test 7:Search permission is denied on a component (EACCES)");
    sprintf (dirname, "%s/dir_1", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create temp directory 1", sstrerror (serrno), 1);
        goto test7_end;
    }
    char dirname2[255];
    sprintf (dirname2, "%s/dir_1/dir_2", base_dir);
    ret = dpns_mkdir (dirname2, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create temp directory 2", sstrerror (serrno), 1);
        goto test7_end;
    }
    ret = dpns_chmod (dirname, 0);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot chmod temp directory 1", sstrerror (serrno), 1);
        goto test7_end;
    }
    char dirname3[255];
    sprintf (dirname3, "%s/dir_1/dir_2/dir_3", base_dir);
    ret = dpns_mkdir (dirname3, 0775);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
            reportComponent (testdesc, "dpns_mkdir returns -1 (EACCES)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
        error = 1;
    }
    test7_end:
        dpns_chmod (dirname, 0775);
        dpns_rmdir (dirname3);
        dpns_rmdir (dirname2);
        dpns_rmdir (dirname);

    strcpy (testdesc, "Test 8:Write access for parent denied (EACCES)");
    sprintf (dirname, "%s/dir_1", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create temp directory", sstrerror (serrno), 1);
        goto test8_end;
    }
    ret = dpns_chmod (dirname, 0555);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot chmod temp directory", sstrerror (serrno), 1);
        goto test8_end;
    }
    sprintf (dirname2, "%s/dir_1/dir_2", base_dir);
    ret = dpns_mkdir (dirname2, 0775);
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
            reportComponent (testdesc, "dpns_mkdir returns -1 (EACCES)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
        error = 1;
    }
    test8_end:
        dpns_chmod (dirname, 0775);
        dpns_rmdir (dirname2);
        dpns_rmdir (dirname);

    strcpy (testdesc, "Test 9:Call dpns_mkdir with unknown DPNS_HOST(SENOSHOST)");
    char errorbuf[4096];
    dpns_seterrbuf(errorbuf, 4096);
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    sprintf (dirname, "%s/mkdir_nohost", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        if ( serrno == SENOSHOST )
            reportComponent (testdesc, "dpns_mkdir returns -1, serrno OK (SENOHOST)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_mkdir returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_mkdir returns 0", "", 1);
        error = 1;
    }
    dpns_rmdir (dirname);


    reportFooter ("");
    reportOverall (error);

    return error;
}



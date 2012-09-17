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
        printf ("Usage: DPNS_setratime <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_utime tests...");

    // Test 1:Create a test file and set its last access and modification times
    strcpy (testdesc, "Test 1:Create a file and execute dpns_utime");
    sprintf (filename, "%s/file_utime", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file_utime", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filestat fstat;
    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat file_utime", sstrerror(serrno), 1);
        goto test1_end;
    }

    time_t atime = fstat.atime;
    time_t mtime = fstat.mtime;

    time_t now = time (NULL);
    struct utimbuf amtimes;
    amtimes.actime = now + 100;
    amtimes.modtime = now + 200;

    ret = dpns_utime (filename, &amtimes);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_utime exited with an error", sstrerror(serrno), 1);
        goto test1_end;
    }
    
    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat file_utime (2)", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( fstat.atime - atime < 100 || fstat.atime - atime > 107 )
    {
        error = 1;
        reportComponent (testdesc, "Unexpected atime value after dpns_utime", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( fstat.mtime - mtime < 200 || fstat.mtime - mtime > 207 )
    {
        error = 1;
        reportComponent (testdesc, "Unexpected mtime value after dpns_utime", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_utime works as expected for files", "", 0);
   
    test1_end :

        sprintf (filename, "%s/file_utime", base_dir);
        dpns_unlink (filename);

    // Test 2: Execute for non-existing directory
    strcpy (testdesc, "Test 2:Execute for non-existing directory(ENOENT)");
    ret = dpns_utime ("/directory/which/does/not/exist", NULL);
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

    // Test 3: Call dpns_utime with NULL argument
    strcpy (testdesc, "Test 3:Call dpns_utime with NULL argument(EFAULT)");
    ret = dpns_utime (NULL, NULL);
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

    // Test 4: dpns_utime for a file in inaccessible directory (permission is not granted)
    strcpy (testdesc, "Test 4:dpns_utime access denied scenario (EACCES)");
    sprintf (dirname, "%s/dpns_utime_access", base_dir);
    sprintf (filename, "%s/dpns_utime_access/file_utime", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret == 0 )
    {
        int ret1 = dpns_creat (filename, 0664);
        ret = dpns_chmod (dirname, 0);
        if ( ret == 0 && ret1 == 0 )
        {
            ret = dpns_utime (filename, NULL);
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
            reportComponent (testdesc, "Error creating file or setting directory permissions", "", 1);
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

    //Test 5: dpns_utime for a directory which name is exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 5:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    dirname[CA_MAXPATHLEN + 1] = '\0';
    ret = dpns_utime (dirname, NULL);
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

    // Test 6: Component of the path is not a directory 
    strcpy (testdesc, "Test 6:Component of the path is not a directory(ENOTDIR)");
    sprintf (dirname, "%s/file", base_dir);
    sprintf (filename, "%s/file/file", base_dir);
    ret = dpns_creat (dirname, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file", sstrerror(serrno), 1);
        goto test6_end;
    }
    ret = dpns_utime (filename, NULL);
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
    test6_end:
        dpns_unlink (dirname);

    // Test 7: DPNS_HOST unknown
    strcpy (testdesc, "Test 7:Call dpns_utime with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_utime (base_dir, NULL);
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



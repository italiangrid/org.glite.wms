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
        printf ("Usage: DPNS_creat <DPNS_HOST> <BASE_DIR> <ADDITIONAL_GROUP>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    char* add_group = argv[3];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_creat(x) tests...");

    // Test 1: Create a file with access mdoe 775
    strcpy (testdesc, "Test 1:Create a file with access mode 775");
    sprintf (filename, "%s/dpns_creat", base_dir);
    struct dpns_filestat fstat;
    ret = dpns_creat (filename, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dpns_creat", sstrerror(serrno), 1);
    }
    else
    {
        ret = dpns_stat (filename, &fstat);
        if ( ret != 0 )
        {
            error = 1;
            reportComponent (testdesc, "Cannot stat file dpns_creat", sstrerror (serrno), 1);
        }
        else
        {
            if ( fstat.filemode != (mode_t)0100775 )
            {
                error = 1;
                reportComponent (testdesc, "Filemode not correct", "", 1);
            }
            else
            {
                reportComponent (testdesc, "File created successfully and filemode set OK", "", 0);
            }
        }
    }
    dpns_unlink (filename);

    // Test 2:Create a file inside a directory which has the S_ISGID bit set
    strcpy (testdesc, "Test 2:Create a file inside a directory which has the S_ISGID bit set");
    sprintf (dirname, "%s/dir_isgid", base_dir);
    sprintf (filename, "%s/dir_isgid/dpns_creat", base_dir);
    ret = dpns_mkdir (dirname, 02775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create a directory with the S_ISGID bit set", sstrerror (serrno), 1);
        goto test2_end;
    }

    gid_t add_gid;
    ret = dpns_getgrpbynam (add_group, &add_gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot lookup GID", sstrerror (serrno), 1);
        goto test2_end;
    }

    ret = dpns_chown (dirname, -1, add_gid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot change directory group owner", sstrerror (serrno), 1);
        goto test2_end;
    }

    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dir_isgid/dpns_creat", sstrerror (serrno), 1);
        goto test2_end;
    }

    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat file dir_isgid/dpns_creat", sstrerror (serrno), 1);
        goto test2_end;
    }

    if ( fstat.gid != add_gid )
    {
        error = 1;
        reportComponent (testdesc, "dir_isgid/dpns_creat has unexpected group owner", sstrerror (serrno), 1);
        goto test2_end;
    }

    reportComponent (testdesc, "dir_isgid/dpns_creat has correct group owner", "", 0);

    test2_end:
        dpns_unlink (filename);
        dpns_rmdir (dirname);


    // Test 3:Create a file with dpns_creatx and check the returned file id
    strcpy (testdesc, "Test 3:Check the fileid returned by dpns_creatx with mode 7777");
    sprintf (filename, "%s/dpns_creatx", base_dir);

    ret = dpns_creatx (filename, 07777, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_creatx failed", sstrerror (serrno), 1);
        goto test3_end;
    }

    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_stat failed", sstrerror (serrno), 1);
        goto test3_end; 
    }

    if ( fstat.fileid != dpns_fid.fileid )
    {
        error = 1;
        reportComponent (testdesc, "fileid does not match", sstrerror (serrno), 1);
        goto test3_end;
    }

    reportComponent (testdesc, "dpns_creatx returns correct fileid", "", 0);

    test3_end:
        dpns_unlink (filename);

    int subtest = 0;

    // Test 4 and 5: File already exists and has replicas (EEXIST)
    strcpy (testdesc, "Test 4:File already exists and has replicas (EEXIST)");
    sprintf (filename, "%s/creat_exists", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Error creating file base_dir/creat_exists", sstrerror(serrno), 1);
        error = 1;
        goto test4_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, dpns_host, "head64.cern.ch:/fs1/email/filereplica.1", '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Error adding replica for base_dir/creat_exists", sstrerror(serrno), 1);
        error = 1;
        goto test4_end;
    }

    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        if ( serrno == EEXIST )
            reportComponent (testdesc, "dpns_creat returns EEXIST", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creat returns wrong serrno", sstrerror(serrno), 1);
            error = 1;
            goto test4_end;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creat returns 0", sstrerror(serrno), 1);
        error = 1;
        goto test4_end;
    }

    strcpy (testdesc, "Test 5:File already exists and has replicas (EEXIST)");
    struct dpns_fileid dpns_fid2;
    ret = dpns_creatx (filename, 0664, &dpns_fid2);
    if ( ret != 0 )
    {
        if ( serrno == EEXIST )
            reportComponent (testdesc, "dpns_creatx returns EEXIST", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creatx returns wrong serrno", sstrerror(serrno), 1);
            error = 1;
            goto test4_end;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creatx returns 0", sstrerror(serrno), 1);
        error = 1;
        goto test4_end;
    }

    test4_end:
        dpns_delreplica (NULL, &dpns_fid, "head64.cern.ch:/fs1/email/filereplica.1");
        dpns_unlink (filename);

    // Test 6: Execute for non-existing directory
    strcpy (testdesc, "Test 6:Execute for non-existing directory(ENOENT)");
    ret = dpns_creat ("/location/which/does/not/exist", 0775);
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

    // Test 7: Call dpns_creat with NULL arguments
    strcpy (testdesc, "Test 7:Call dpns_creat with NULL arguments(EFAULT)");
    ret = dpns_creat (NULL, 0765);
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

    // Test 8: Call dpns_creatx with NULL arguments
    strcpy (testdesc, "Test 8:Call dpns_creatx with NULL arguments(EFAULT)");
    ret = dpns_creatx (NULL, 0765, NULL);
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

    // Test 9: Component of the path is not a directory
    strcpy (testdesc, "Test 9:Component of the path is not a directory (ENOTDIR)");
    sprintf (dirname, "%s/file", base_dir);
    sprintf (filename, "%s/file/file", base_dir);
    ret = dpns_creat (dirname, 0664);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating file", sstrerror(serrno), 1);
        goto test7_end;
    }
    ret = dpns_creat (filename, 0664);
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

    // Test 10: Path is an existing directory
    strcpy (testdesc, "Test 10:File is an existing directory (EISDIR)");
    ret = dpns_creat (base_dir, 0664);
    if ( ret != 0 )
    {
        if ( serrno == EISDIR )
            reportComponent (testdesc, "dpns_creat returns -1, serrno OK (ENOTDIR)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creat returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creat returns 0", "", 1);
        error = 1;
    }

    // Test 11: Path is an existing directory
    strcpy (testdesc, "Test 11:File is an existing directory (EISDIR)");
    ret = dpns_creatx (base_dir, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        if ( serrno == EISDIR )
            reportComponent (testdesc, "dpns_creatx returns -1, serrno OK (ENOTDIR)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creatx returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creatx returns 0", "", 1);
        error = 1;
    }

    //Test 12: Path exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 12:Path is exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (filename, 61, CA_MAXPATHLEN + 1);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            reportComponent (testdesc, "dpns_creat returns -1 (ENAMETOOLONG)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creat returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creat returns 0", "", 1);
        error = 1;
    }

    //Test 13: Path exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 13:Path is exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (filename, 61, CA_MAXPATHLEN + 1);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            reportComponent (testdesc, "dpns_creatx returns -1 (ENAMETOOLONG)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creatx returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creatx returns 0", "", 1);
        error = 1;
    }
    dpns_unlink (filename);

    // Test 14: DPNS_HOST unknown
    strcpy (testdesc, "Test 14:Call dpns_creat with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    sprintf (filename, "%s/file", base_dir);
    ret = dpns_creat (filename, 0755);
    if ( ret != 0 )
    {
        if ( serrno == SENOSHOST )
            reportComponent (testdesc, "dpns_creat returns -1, serrno OK (SENOHOST)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creat returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creat returns 0", "", 1);
        error = 1;
    }

    // Test 15: DPNS_HOST unknown
    strcpy (testdesc, "Test 15:Call dpns_creatx with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    sprintf (filename, "%s/file", base_dir);
    ret = dpns_creatx (filename, 0755, &dpns_fid);
    if ( ret != 0 )
    {
        if ( serrno == SENOSHOST )
            reportComponent (testdesc, "dpns_creatx returns -1, serrno OK (SENOHOST)", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_creatx returns -1, serrno BAD", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_creatx returns 0", "", 1);
        error = 1;
    }

    dpns_unlink (filename);


    reportFooter ("");
    reportOverall (error);

    return error;
}



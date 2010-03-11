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

    reportHeader ("* Executing dpns_umask tests...");

    // Prepare a directory with simple ACL (no default ACEs)
    sprintf (dirname, "%s/dir_umask", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create working directory dir_umask", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_acl list[3];

    list[0].a_type = CNS_ACL_USER_OBJ;
    list[0].a_id = 0;
    list[0].a_perm = 7;

    list[1].a_type = CNS_ACL_GROUP_OBJ;
    list[1].a_id = 0;
    list[1].a_perm = 7;

    list[2].a_type = CNS_ACL_OTHER;
    list[2].a_id = 0;
    list[2].a_perm = 5;

    ret = dpns_setacl (dirname, 3, list);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setacl call failed for dir_umask", sstrerror(serrno), 1);
        goto test1_end;
    }

    // Test 1: Set the umask to 007 and create a file with mode 666
    strcpy (testdesc, "Test 1:Umask 007 and file creation with mode 666");
    sprintf (filename, "%s/dir_umask/testfile_umask", base_dir);
    mode_t pmask = dpns_umask (0007);
    ret = dpns_creat (filename, 0666);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create dir_umask/testfile_umask", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filestat fstat;
    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat dir_umask/testfile_umask", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( fstat.filemode != ( S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) )
    {
        error = 1;
        reportComponent (testdesc, "Filemode not correct", sstrerror(serrno), 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_umask works as expected for files", "", 0);

   
    test1_end :

    // Test 2: Set the umask to 042 and create a directory with mode 775
    strcpy (testdesc, "Test 2:Umask 042 and file creation with mode 775");
    sprintf (filename, "%s/dir_umask/testdir_umask", base_dir);
    pmask = dpns_umask (0042);
    ret = dpns_mkdir (filename, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create dir_umask/testdir_umask", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_stat (filename, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat dir_umask/testdir_umask", sstrerror(serrno), 1);
        goto test2_end;
    }

    if ( fstat.filemode != ( S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat dir_umask/testdir_umask", sstrerror(serrno), 1);
        goto test2_end;
    }

    reportComponent (testdesc, "dpns_umask works as expected for directories", "", 0);

    test2_end:

        sprintf (filename, "%s/dir_umask/testfile_umask", base_dir);
        dpns_unlink (filename);
        sprintf (filename, "%s/dir_umask/testdir_umask", base_dir);
        dpns_rmdir (filename);
        sprintf (filename, "%s/dir_umask", base_dir);
        dpns_rmdir (filename);


    reportFooter ("");
    reportOverall (error);

    return error;
}



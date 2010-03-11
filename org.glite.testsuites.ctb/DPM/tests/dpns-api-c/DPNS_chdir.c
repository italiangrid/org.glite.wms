#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>

#include "utils.h"

int main (int argc, char** argv)
{
    int ret;
    int error = 0;
    dpns_DIR* dir;
    char testdesc[256];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_chdir <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    printf ("\n* Executing dpns_chdir tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: Create a subdirectory in <BASE_DIR> and perform chdir to it
    strcpy (testdesc, "Test 1:Perform chdir to a newly created directory");
    sprintf (dirname, "%s/dir_chdir", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        printf ("%s:CANNOT PREPARE ENVIRONMENT:%s:FAILURE\n", testdesc, sstrerror(serrno));
    }
    else
    {
        ret = dpns_chdir (dirname);

        if ( ret != 0 )
        {
            error = 1;
            printf ("%s:dpns_chdir call failed:%s:FAILURE\n", testdesc, sstrerror(serrno));
            goto test1_end;
        }

        // Test operations using paths relative to the new directory
        ret = populateDir (".", 10);
        if ( ret != 0 )
        {
            error = 1;
            printf ("%s:Error preparing scenario:%s:FAILURE\n", testdesc, sstrerror(serrno));
            goto test1_end;
        }
        ret = populateDir ("dir_1", 10);
        if ( ret != 0 )
        {
            error = 1;
            printf ("%s:Error preparing scenario:%s:FAILURE\n", testdesc, sstrerror(serrno));
            goto test1_end;
        }

        // Check the created directory structure exists
        dir_contents* ds = getDirList ("dir_1");
        if ( ds == NULL || verifyContents (ds, 10) )
        {
            error = 1;
            printf ("%s:dir_chdir/dir_1 contents does not match:%s:FAILURE\n", testdesc, sstrerror(serrno));
            goto test1_end;
        }
        free (ds);

        sprintf (filename, "%s/dir_chdir", base_dir);
        ds = getDirList (filename);
        if ( ds == NULL )
        {
            error = 1;
            printf ("%s:dir_chdir contents does not match:%s:FAILURE\n", testdesc, sstrerror(serrno));
            goto test1_end;
        }
        free (ds);

        printf ("%s:dir_chdir OK. Contents matches::SUCCESS\n", testdesc);
    }

    test1_end:

        dpns_chdir ("/");
        sprintf (dirname, "%s/dir_chdir/dir_1", base_dir);
        cleanupDir (dirname, 10);
        sprintf (dirname, "%s/dir_chdir", base_dir);
        cleanupDir (dirname, 10);
        dpns_rmdir (dirname);

    // Test 2: Open non-existing directory
    strcpy (testdesc, "Test 2:dpns_chdir to non-existing directory(ENOENT)");
    ret = dpns_chdir ("/directory/which/does/not/exist");
    if ( dir != 0 )
    {
        if ( serrno == ENOENT )
            printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
        else
        {
            printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
            error = 1;
        }
    }
    else
    {
        printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
        error = 1;
    }

    // Test 3: Call dpns_chdir to a file
    strcpy (testdesc, "Test 3:dpns_chdir to a file(ENOTDIR)");
    sprintf (filename, "%s/file", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret == 0 )
    {
        ret = dpns_chdir (filename);
        if ( ret != 0 )
        {
            if ( serrno == ENOTDIR )
                printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
            else
            {
                printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
                error = 1;
            }
        }
        else
        {
            printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
            error = 1;
        }
        dpns_unlink (filename);
    }
    else
    {
        printf ("%s:CANNOT PREPARE ENVIRONMENT::FAILURE\n", testdesc);
        error = 1;
    }

    // Test 4: dpns_chdir with NULL argument
    strcpy (testdesc, "Test 4:dpns_chdir with NULL argument(EFAULT)");
    ret = dpns_chdir (NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
            printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
        else
        {
            printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
            error = 1;
        }
    }
    else
    {
        printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
        error = 1;
    }

    // Test 5: dpns_chdir to a directory where access is not granted
    strcpy (testdesc, "Test 5:dpns_chdir to inaccesible directory(EACCES)");
    sprintf (dirname, "%s/EACCES", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret == 0 )
    {
        ret = dpns_chmod (dirname, 0);
        if ( ret == 0 )
        {
            ret = dpns_chdir (dirname);
            if ( ret != 0 )
            {
                if ( serrno == EACCES )
                    printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
                else
                {
                    printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
                    error = 1;
                }
            }
            else
            {
                printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
                error = 1;
                dpns_closedir (dir);
            }
            dpns_chmod (dirname, 0775);
            dpns_rmdir (dirname);
        }
        else
        {
            dpns_rmdir (dirname);
        }
    }
    else
    {
        printf ("%s:CANNOT PREPARE ENVIRONMENT::FAILURE\n", testdesc);
        error = 1;
    }
   
    //Test 6: Open directory with name exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 6:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    dirname[CA_MAXPATHLEN + 1] = '\0';
    ret = dpns_chdir (dirname);
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
            printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
        else
        {
            printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
            error = 1;
        }
    }
    else
    {
        printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
        error = 1;
    }

    // Test 7: DPNS_HOST unknown
    strcpy (testdesc, "Test 7:Call dpns_chdir with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_chdir (filename);
    if ( ret != 0 )
    {
        if ( serrno == SENOSHOST )
            printf ("%s:RETURNS NULL:%s:SUCCESS\n", testdesc, sstrerror(serrno));
        else
        {
            printf ("%s:RETURNS NULL:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
            error = 1;
        }
    }
    else
    {
        printf ("%s:RETURNS VALID HANDLE::FAILURE\n", testdesc);
        error = 1;
    }

    printf ("--------------------------------------------------------------------------------\n");

    if ( error == 0 )
        printf ("Overall result:SUCCESS\n\n");
    else
        printf ("Overall result:FAILURE\n\n");

    return error;
}


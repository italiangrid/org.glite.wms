#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>

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
        printf ("Usage: DPNS_opendir <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    printf ("\n* Executing dpns_opendir tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: open the existing directory <BASE_DIR>
    strcpy (testdesc, "Test 1:Open existing and accessible directory");
    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        printf ("%s:RETURNS NULL:%s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
    }
    else
    {
        dpns_closedir (dir);
        printf ("%s:RETURNS VALID HANDLE::SUCCESS\n", testdesc);
    }

    // Test 2: Open non-existing directory
    strcpy (testdesc, "Test 2:Open non-existing directory(ENOENT)");
    dir = dpns_opendir ("/directory/which/does/not/exist");
    if ( dir == NULL )
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

    // Test 3: Call dpns_opendir with NULL argument
    strcpy (testdesc, "Test 3:Call dpns_opendir with NULL argument(EFAULT)");
    dir = dpns_opendir (NULL);
    if ( dir == NULL )
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

    // Test 4: Open directory for which permission is not granted
    strcpy (testdesc, "Test 4:Open inaccesible directory(EACCES)");
    sprintf (dirname, "%s/EACCES", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret == 0 )
    {
        ret = dpns_chmod (dirname, 0);
        if ( ret == 0 )
        {
            dir = dpns_opendir (dirname);
            if ( dir == NULL )
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

    //Test 5: Open directory with name exceeding CA_MAXPATHLEN
    strcpy (testdesc, "Test 5:Directory name exceeding CA_MAXPATHLEN(ENAMETOOLONG)");
    memset (dirname, 61, CA_MAXPATHLEN + 1);
    dirname[CA_MAXPATHLEN + 1] = '\0';
    dir = dpns_opendir (dirname);
    if ( dir == NULL )
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

    // Test 6: Call dpns_opendir with filename
    strcpy (testdesc, "Test 6:Call dpns_opendir with filename(ENOTDIR)");
    sprintf (filename, "%s/file", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret == 0 )
    {
        dir = dpns_opendir (filename);
        if ( dir == NULL )
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

    // Test 7: DPNS_HOST unknown
    strcpy (testdesc, "Test 7:Call dpns_opendir with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    dir = dpns_opendir (filename);
    if ( dir == NULL )
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


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <dirent.h>
#include <errno.h>
#include <serrno.h>

int main (int argc, char** argv)
{
    int cnt;
    int ret;
    int error = 0;
    dpns_DIR* dir;
    char testdesc[256];
    struct dirent *dentry;
    struct dirent entries[16];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char filemap[10];
    char dirmap[10];

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_readdir <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    printf ("\n* Executing dpns_readdir tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: Opening and reading the empty directory <BASE_DIR>
    strcpy (testdesc, "Test 1:Read the initially empty <BASE_DIR>");
    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        printf ("%s:dpns_opendir returns NULL:%s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
    }
    else
    {
        cnt = 0;
        serrno = 0;

        while ( dentry = dpns_readdir (dir) ) ++cnt;

        if ( dentry == NULL && serrno != 0 )
        {
            printf ("%s:RETURNS NULL:serrno is set to %d:FAILURE\n", testdesc, serrno);
            error = 1;
        }
        else
        {
            if ( cnt == 0 )
                printf ("%s:RETURNS OK:Count is 0:SUCCESS\n", testdesc);
            else
            {     
                printf ("%s:RETURNS OK:Unexpected count is %d:FAILURE\n", testdesc, cnt);
                error = 1;
            }
        }
        dpns_closedir (dir);
    }

    // Test 2: Populate directory with files and directories and read it
    strcpy (testdesc, "Test 2:Read populated directory");
    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret = dpns_creat (filename, 0664);
        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret = dpns_mkdir (filename, 0775);
    }

    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        printf ("%s:dpns_opendir returns NULL:%s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
    }
    else
    {
        for ( cnt = 0; cnt < 10; ++cnt )
        {
           filemap[cnt] = 0;
           dirmap[cnt] = 0;
        }

        cnt = 0;
        serrno = 0;
        int unexpected = 0;
        while ( dentry = dpns_readdir (dir) )
        {
            unexpected = 0;
            if ( strstr (dentry->d_name, "file_") == dentry->d_name && strlen (dentry->d_name) == 6)
            {
                if ( (dentry->d_name[5] - '0' >= 0) && (dentry->d_name[5] - '0' <= 9) )
                {
                    filemap[dentry->d_name[5] - '0'] = 1;
                    continue;
                }
                else
                    unexpected = 1;
            }                
            if ( strstr (dentry->d_name, "dir_") == dentry->d_name )
            {
                if ( (dentry->d_name[4] - '0' >= 0) && (dentry->d_name[4] - '0' <= 9) )
                {
                    dirmap[dentry->d_name[4] - '0'] = 1;
                    continue;
                }
                else
                    unexpected = 1;
            }
            if ( unexpected == 1 ) break;
        }
        int notfound = 0;
        for ( cnt = 0; cnt < 10; ++cnt )
        {
            if ( filemap[cnt] != 1 || dirmap[cnt] != 1 )
            {
                notfound = 1;
                break;
            }
        }
        if ( serrno != 0 )
        {
            printf ("%s:RETURNS BAD:serrno is %d - %s:FAILURE\n", testdesc, serrno, sstrerror(serrno));
            error = 1;
        }
        else
        {
            if ( unexpected == 1 )
            {
                printf ("%s:RETURNS OK:Unexpected entries found in list:FAILURE\n", testdesc, sstrerror(serrno));
                error = 1;
            }
            else if ( notfound == 1 )
            {
                printf ("%s:RETURNS OK:Missing entries from list:FAILURE\n", testdesc, sstrerror(serrno));
                error = 1;
            }
            else
                printf ("%s:RETURNS OK:All entries found. None unexpected:SUCCESS\n", testdesc);
        }
        dpns_closedir (dir);
    }

    // Test 3: Call dpns_opendir with NULL argument
    strcpy (testdesc, "Test 3:Call dpns_readdir with NULL argument(EFAULT)");
    dentry = dpns_readdir (NULL);
    if ( dentry == NULL )
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
        printf ("%s:RETURNS NON NULL::FAILURE\n", testdesc);
        error = 1;
    }

    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret = dpns_unlink (filename);
        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret = dpns_rmdir (filename);
    }

    printf ("--------------------------------------------------------------------------------\n");

    if ( error == 0 )
        printf ("Overall result:SUCCESS\n\n");
    else
        printf ("Overall result:FAILURE\n\n");

    return error;
}


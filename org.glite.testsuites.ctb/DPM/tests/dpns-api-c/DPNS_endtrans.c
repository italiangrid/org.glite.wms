#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <dirent.h>
#include <errno.h>
#include <serrno.h>

int main (int argc, char** argv)
{
    int cnt = 0;
    int ret = 0;
    int error = 0;

    dpns_DIR* dir;
    char testdesc[256];
    struct dirent *dentry;
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

    printf ("\n* Executing dpns_endtrans tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: Start a transaction which should complete successfully.
    strcpy (testdesc, "Test 1:Successfull transation scenario");

    // First do some modifications outside the transaction scope
    sprintf (filename, "%s/file_before", base_dir);
    ret = dpns_creat (filename, 0664);
    if ( ret != 0 ) 
    {
        printf ("%s:Error preparing scenario:dpns_creat failed with %s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
        goto test1_end;
    }
    sprintf (filename, "%s/dir_before", base_dir);
    ret = dpns_mkdir (filename, 0775);
    if ( ret != 0 )
    {
        printf ("%s:Error preparing scenario:dpns_mkdir failed with %s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
        goto test1_end;
    }

    // Start a transaction
    ret = dpns_starttrans (dpns_host, "Test transaction");
    if ( ret != 0 )
    {
        printf ("%s:dpns_starttrans returned with error:%s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
        goto test1_end;
    }

    //Add some entries in the transaction scope
    ret = 0;
    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret += dpns_creat (filename, 0664);
        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret += dpns_mkdir (filename, 0775);
    }
    if ( ret != 0 )
    {
        printf ("%s:Problem while adding files/directories inside the transaction scope::FAILURE\n", testdesc);
        error = 1;
        goto test1_end;
    }

    //Remove the entries which were created before the start of the transaction
    ret = 0;
    sprintf (filename, "%s/file_before", base_dir);
    ret += dpns_unlink (filename);
    sprintf (filename, "%s/dir_before", base_dir);
    ret += dpns_rmdir (filename);
    if ( ret != 0 )
    {
        printf ("%s:Problem while removing files/directories inside the transaction scope::FAILURE\n", testdesc);
        error = 1;
        goto test1_end;
    }

    ret = dpns_endtrans ();
    if ( ret != 0 )
    {
        printf ("%s:dpns_endtrans returns with error:%s:FAILURE\n", testdesc, sstrerror(serrno));
        error = 1;
        goto test1_end;
    }

    // Check the entries created inside the transaction scope exists (and only these entries!).
    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        printf ("%s:dpns_opendir returns NULL, cannot verify transaction result:%s:FAILURE\n", testdesc, sstrerror(serrno));
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

    test1_end:

        // Try do some cleanup...
        sprintf (filename, "%s/file_before", base_dir); 
        dpns_unlink (filename);
        sprintf (filename, "%s/dir_before", base_dir);
        dpns_rmdir (filename);
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


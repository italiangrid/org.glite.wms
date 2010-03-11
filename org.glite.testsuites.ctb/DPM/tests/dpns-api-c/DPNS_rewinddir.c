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

    printf ("\n* Executing dpns_rewinddir tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: Populate directory with files and directories and read it with two rewinds
    strcpy (testdesc, "Test 1:Read directory with rewinds");
    sprintf (dirname, "%s/dpns_rewind", base_dir, cnt);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create directory dpns_rewind", sstrerror(serrno), 1);
        goto test1_end;
    }
    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", dirname, cnt);
        dpns_creat (filename, 0664);
        sprintf (filename, "%s/dir_%d", dirname, cnt);
        dpns_mkdir (filename, 0775);
    }

    dir = dpns_opendir (dirname);
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
        while ( (dentry = dpns_readdir (dir)) && cnt < 10 ) { ++cnt; }
        if ( cnt != 10 )
        {
            printf ("%s:Initial dpns_readdir returns less entries than expected(%d)::FAILURE\n", testdesc, cnt);
            dpns_closedir (dir);
            error = 1;
        }
        else
        {
            dpns_rewinddir (dir);
            cnt = 0;
            while ( dentry = dpns_readdir (dir) ) { ++cnt; } 

            if ( cnt != 20 )
            {
                 printf ("%s:Second full dpns_readdir returns less entries than expected(%d)::FAILURE\n", testdesc, cnt);
                 dpns_closedir (dir);
                 error = 1;
            }
            else 
            {
                dpns_rewinddir (dir);

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
                    }
                    if ( strstr (dentry->d_name, "dir_") == dentry->d_name )
                    {
                        if ( (dentry->d_name[4] - '0' >= 0) && (dentry->d_name[4] - '0' <= 9) )
                        {
                            dirmap[dentry->d_name[4] - '0'] = 1;
                            continue;
                        }
                    }
                    unexpected = 1;
                    break;
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
                        printf ("\n\n");
                        for ( cnt = 0; cnt < 10; ++cnt ) printf ("%d:%d\n", cnt, filemap[cnt]);
                        printf ("\n\n");
                        for ( cnt = 0; cnt < 10; ++cnt ) printf ("%d:%d\n", cnt, dirmap[cnt]);
                        error = 1;
                    }
                    else
                        printf ("%s:RETURNS OK:All entries found. None unexpected:SUCCESS\n", testdesc);
                }
                dpns_closedir (dir);
            }
        }
    }

    test1_end:
        for ( cnt = 0; cnt < 10; ++cnt )
        {
            sprintf (filename, "%s/file_%d", dirname, cnt);
            ret = dpns_unlink (filename);
            sprintf (filename, "%s/dir_%d", dirname, cnt);
            ret = dpns_rmdir (filename);
        }
        dpns_rmdir (dirname);

    printf ("--------------------------------------------------------------------------------\n");

    if ( error == 0 )
        printf ("Overall result:SUCCESS\n\n");
    else
        printf ("Overall result:FAILURE\n\n");

    return error;
}


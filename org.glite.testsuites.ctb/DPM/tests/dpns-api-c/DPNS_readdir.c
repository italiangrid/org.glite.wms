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

    reportHeader ("* Executing dpns_readdir tests...");

    // Test 1: Opening and reading the empty directory <BASE_DIR>
    strcpy (testdesc, "Test 1:Read the initially empty <BASE_DIR>");
    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        reportComponent (testdesc, "dpns_opendir returns NULL", sstrerror(serrno), 1);
        error = 1;
    }
    else
    {
        cnt = 0;
        serrno = 0;

        while ( dentry = dpns_readdir (dir) ) ++cnt;

        if ( dentry == NULL && serrno != 0 )
        {
            reportComponent (testdesc, "dpns_opendir returns NULL", sstrerror (serrno), 1);
            error = 1;
        }
        else
        {
            if ( cnt == 0 )
                reportComponent (testdesc, "Returns OK. Count is 0", "", 0);
            else
            {     
                reportComponent (testdesc, "Returns OK. Unexpected file count", "", 1);
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
        reportComponent (testdesc, "dpns_opendir returns NULL", sstrerror(serrno), 1);
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
            reportComponent (testdesc, "dpns_readdir returns with error", sstrerror(serrno), 1);
            error = 1;
        }
        else
        {
            if ( unexpected == 1 )
            {
                reportComponent (testdesc, "Unexpected entries found in list", sstrerror(serrno), 1);
                error = 1;
            }
            else if ( notfound == 1 )
            {
                reportComponent (testdesc, "Missing entries from list", sstrerror(serrno), 1);
                error = 1;
            }
            else
                reportComponent (testdesc, "All entries found. None unexpected", sstrerror(serrno), 0);
        }
        dpns_closedir (dir);
    }

    // Test 3: Call dpns_opendir with NULL argument
    strcpy (testdesc, "Test 3:Call dpns_readdir with NULL argument(EFAULT)");
    dentry = dpns_readdir (NULL);
    if ( dentry == NULL )
    {
        if ( serrno == EFAULT )
            reportComponent (testdesc, "dpns_readdir returns NULL, serrno is EFAULT", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "dpns_readdir returns NULL, unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_readdir returns non-NULL", "", 1);
        error = 1;
    }

    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret = dpns_unlink (filename);
        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret = dpns_rmdir (filename);
    }


    reportFooter ("");
    reportOverall (error);

    return error;
}


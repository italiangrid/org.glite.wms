#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <dirent.h>
#include <errno.h>
#include <serrno.h>
#include <sys/stat.h>
#include <sys/types.h>

int main (int argc, char** argv)
{
    int cnt;
    int ret;
    int error = 0;
    dpns_DIR* dir;
    char testdesc[256];
    struct dpns_direnstatg *dentry;
    struct dirent entries[16];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char linkname[CA_MAXPATHLEN + 2];
    char errorstr[2048];
    char filemap[10];
    char dirmap[10];
    char linkmap[10];

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_readdirg <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_readdirg tests...");

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

        while ( dentry = dpns_readdirg (dir) ) ++cnt;

        if ( dentry == NULL && serrno != 0 )
        {
            reportComponent (testdesc, "dpns_readdirg returns NULL", sstrerror (serrno), 1);
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

    // Test 2: Populate directory with files directories and symlinks and read it
    strcpy (testdesc, "Test 2:Read populated directory");
    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret = dpns_creat (filename, 0664);
        if ( ret != 0 )
        {
            reportComponent (testdesc, "dpns_creat returned an error", sstrerror (serrno), 1);
            error = 1;
            goto test2_end;
        }

        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret = dpns_mkdir (filename, 0775);
        if ( ret != 0 )
        {
            reportComponent (testdesc, "dpns_mkdir returned an error", sstrerror (serrno), 1);
            error = 1;
            goto test2_end;
        }

        sprintf (linkname, "%s/link_%d", base_dir, cnt);
        if ( cnt % 2 )
            ret = dpns_symlink (dirname, linkname);
        else
            ret = dpns_symlink (filename, linkname);
        if ( ret != 0 )
        {
            reportComponent (testdesc, "dpns_symlink returned an error", sstrerror (serrno), 1);
            error = 1;
            goto test2_end;
        }
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
        while ( dentry = dpns_readdirg (dir) )
        {
            unexpected = 0;

            if ( strstr (dentry->d_name, "file_") == dentry->d_name && strlen (dentry->d_name) == 6)
            {
                if ( (dentry->d_name[5] - '0' >= 0) && (dentry->d_name[5] - '0' <= 9) )
                {
                    filemap[dentry->d_name[5] - '0'] = 1;
                    if ( dentry->filemode & S_IFREG == 0 || ((dentry->filemode & 0777) != 0664) )
                    {
                        sprintf (errorstr, "Wrong filemode detected for file %s", dentry->d_name);
                        reportComponent (testdesc, errorstr, "", 1);
                        error = 1;
                        goto test2_end;
                    }
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
                    if ( dentry->filemode & S_IFDIR == 0 || ((dentry->filemode & 0777) != 0775) )
                    {
                        sprintf (errorstr, "Wrong filemode detected for directory %s", dentry->d_name);
                        reportComponent (testdesc, errorstr, "", 1);
                        error = 1;
                        goto test2_end;
                    }
                    continue;
                }
                else
                    unexpected = 1;
            }

            if ( strstr (dentry->d_name, "link_") == dentry->d_name )
            {
                if ( (dentry->d_name[5] - '0' >= 0) && (dentry->d_name[5] - '0' <= 9) )
                {
                    linkmap[dentry->d_name[5] - '0'] = 1;
                    if ( dentry->filemode & S_IFLNK == 0 || ((dentry->filemode & 0777) != 0777))
                    {
                        sprintf (errorstr, "Wrong filemode detected for link %s", dentry->d_name);
                        reportComponent (testdesc, errorstr, "", 1);
                        error = 1;
                        goto test2_end;
                    }
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
            if ( filemap[cnt] != 1 || dirmap[cnt] != 1 || linkmap[cnt] != 1 )
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
                reportComponent (testdesc, "All entries found. None unexpected", "", 0);
        }
        dpns_closedir (dir);
    }

    test2_end:

    for ( cnt = 0; cnt < 10; ++cnt )
    {
        sprintf (linkname, "%s/link_%d", base_dir, cnt);
        ret = dpns_unlink (linkname);
        sprintf (filename, "%s/file_%d", base_dir, cnt);
        ret = dpns_unlink (filename);
        sprintf (filename, "%s/dir_%d", base_dir, cnt);
        ret = dpns_rmdir (filename);
    }

    // Test 3: Call dpns_opendirg with NULL argument
    strcpy (testdesc, "Test 3:Call dpns_readdirg with NULL argument(EFAULT)");
    dentry = dpns_readdirg (NULL);
    if ( dentry == NULL )
    {
        if ( serrno == EFAULT )
            reportComponent (testdesc, "serrno is EFAULT", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "dpns_readdirg returns non-NULL", "", 1);
        error = 1;
    }


    reportFooter ("");
    reportOverall (error);

    return error;
}


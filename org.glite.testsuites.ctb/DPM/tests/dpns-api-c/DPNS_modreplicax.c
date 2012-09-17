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
    char sfn1[1024], sfn2[1024];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char symname[CA_MAXPATHLEN + 2];
    char * envvar;
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_modreplicax <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_modreplicax tests...");

    // Test 1: Create a file, add replica and modify it
    strcpy (testdesc, "Test 1:Modify replicax properties");
    sprintf (filename, "%s/dpns_modreplicax", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dpns_modreplicax", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn1, "%s:/fs1/dir1/filerep.1", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn1, '-', 'P', "pool1", "/fs1", 'P', "setname_1"); 
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn2, "%s:/fs1/dir1/filerep.2", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn2, '-', 'P', "pool2", "/fs2", 'S', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_modreplicax (sfn1, "setname_new_1", "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'S');
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot modify replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_modreplicax (sfn2, "setname_new_2", "poolname_new_2", "dpns_host_new_2", "fs_new_2", 'P');
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot modify replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    int found1 = -1, found2 = -1;
    int rcount = 0;
    struct dpns_filereplicax *r_entries;
    ret = dpns_getreplicax (filename, NULL, NULL, &rcount, &r_entries);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot get replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }
    else
    {
        if ( rcount != 2 )
        {
            error = 1;
            reportComponent (testdesc, "Wrong replica count returned", sstrerror(serrno), 1);
            goto test1_end;
        }
        else
        {
            if ( strcmp (sfn1, r_entries[0].sfn) == 0 )
                found1 = 0;
            else
                if ( strcmp (sfn2, r_entries[0].sfn) == 0 )
                    found2 = 0;

            if ( strcmp (sfn1, r_entries[1].sfn) == 0 )
                found1 = 1;
            else
                if ( strcmp (sfn2, r_entries[1].sfn) == 0 )
                    found2 = 1;

            if ( found1 == -1 )
            {
                error = 1;
                reportComponent (testdesc, "Cannot find replica 1 in the getreplica result", sstrerror(serrno), 1);
                goto test1_end;
            }

            if ( found2 == -1 )
            {
                error = 1;
                reportComponent (testdesc, "Cannot find replica 2 in the getreplica result", sstrerror(serrno), 1);
                goto test1_end;
            }

            if ( strcmp (r_entries[found1].setname, "setname_new_1") != 0 ||
                 strcmp (r_entries[found1].poolname, "poolname_new_1") != 0 ||
                 strcmp (r_entries[found1].host, "dpns_host_new_1") != 0 ||
                 strcmp (r_entries[found1].fs, "fs_new_1") != 0 ||
                 r_entries[found1].r_type != 'S' )
            {
                error = 1;
                reportComponent (testdesc, "Property mismatch for replica 1", sstrerror(serrno), 1);
                goto test1_end;
            }

            if ( strcmp (r_entries[found2].setname, "setname_new_2") != 0 ||
                 strcmp (r_entries[found2].poolname, "poolname_new_2") != 0 ||
                 strcmp (r_entries[found2].host, "dpns_host_new_2") != 0 ||
                 strcmp (r_entries[found2].fs, "fs_new_2") != 0 ||
                 r_entries[found2].r_type != 'P' )
            {
                error = 1;
                reportComponent (testdesc, "Property mismatch for replica 2", sstrerror(serrno), 1);
                goto test1_end;
            }
        }
    }

    reportComponent (testdesc, "Modified properties matched OK", "", 0);

    test1_end:

        dpns_delreplica (NULL, NULL, sfn1);
        dpns_delreplica (NULL, NULL, sfn2);
        dpns_unlink (filename);


    // Test 2: Execute for non-existing replica
    strcpy (testdesc, "Test 2:Execute for non-existing replica(ENOENT)");
    sprintf (filename, "%s/dpns_modreplicax", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dpns_modreplica", sstrerror(serrno), 1);
        goto test2_end;
    }

    ret = dpns_modreplicax ("some_nonexisting_sfn", "setname_new_1", "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'S');
    if ( ret != 0 )
    {
        if ( serrno == ENOENT )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is ENOENT", sstrerror(serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test2_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test2_end;
    }

    test2_end:

        dpns_unlink (filename);

    // Test 3: Call dpns_modreplicax with sfn == NULL 
    strcpy (testdesc, "Test 3:Call dpns_modreplicax with sfn == NULL (EFAULT)");
    sprintf (filename, "%s/dpns_modreplicax", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dpns_modreplica", sstrerror(serrno), 1);
        goto test3_end;
    }

    ret = dpns_modreplicax (NULL, "setname_new_1", "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'S');
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is EFAULT", sstrerror(serrno), 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test3_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test3_end;
    }

    test3_end:

        dpns_unlink (filename);


    // Test 4: dpns_modreplicax for a file when permission is not granted
    strcpy (testdesc, "Test 4:dpns_modreplicax for a file when permission is not granted (EACCES)");
    sprintf (dirname, "%s/dir_modreplicax", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create directory dir_modreplica", sstrerror(serrno), 1);
        goto test4_end;
    }

    sprintf (filename, "%s/dir_modreplicax/dpns_modreplicax", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file dir_modreplicax/dpns_modreplicax", sstrerror(serrno), 1);
        goto test4_end;
    }

    sprintf (sfn1, "%s:/fs1/dir1/filerep.1", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn1, '-', 'P', "pool1", "/fs1", 'P', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 1", sstrerror(serrno), 1);
        goto test4_end;
    }
 
    ret = dpns_chmod (dirname, 0000);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot change access bits for file", sstrerror(serrno), 1);
        goto test4_end;
    }

    ret = dpns_modreplicax (sfn1, "setname_new_1", "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'P');
    if ( ret != 0 )
    {
        if ( serrno == EACCES )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is EACCES", sstrerror(serrno), 0);
            goto test4_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test4_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test4_end;
    }

    test4_end:

        dpns_chmod (dirname, 0775);
        dpns_delreplica (NULL, NULL, sfn1);
        dpns_unlink (filename);
        dpns_rmdir (dirname);

    //Test 5: dpns_modreplicax for sfn exceeding CA_MAXSFNLEN
    strcpy (testdesc, "Test 5:dpns_modreplicax for sfn exceeding CA_MAXSFNLEN(ENAMETOOLONG)");
    sprintf (filename, "%s/modreplicax_maxdfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Cannot create file modreplicax_maxsfn", sstrerror (serrno), 1);
        error = 1;
        goto test5_end;
    }

    char sfnbig[1500];
    memset (sfnbig, 70, 1498); sfnbig[1499] = '\0';
    ret = dpns_modreplicax (sfnbig, "setname_new_1", "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'S');
    if ( ret != 0 )
    {
        if ( serrno == ENAMETOOLONG )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is ENAMETOOLONG", sstrerror(serrno), 0);
            goto test5_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test5_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test5_end;
    }

    test5_end:

        dpns_chmod (dirname, 0775);
        dpns_delreplica (NULL, NULL, sfn1);
        dpns_unlink (filename);


    //Test 6: dpns_modreplica with sername exceeding 36
    strcpy (testdesc, "Test 6:dpns_modreplicax with setname exceeding 36(EINVAL)");
    sprintf (filename, "%s/modreplicax_maxdfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Cannot create file modreplicax_maxsfn", sstrerror (serrno), 1);
        error = 1;
        goto test6_end;
    }

    char setname[64];
    memset (setname, 70, 62); setname[63] = '\0';
    ret = dpns_modreplicax ("some_file", setname, "poolname_new_1", "dpns_host_new_1", "fs_new_1", 'P');
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is EINVAL", sstrerror(serrno), 0);
            goto test6_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test6_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test6_end;
    }

    test6_end:

        dpns_chmod (dirname, 0775);
        dpns_delreplica (NULL, NULL, sfn1);
        dpns_unlink (filename);

    //Test 7: dpns_modreplicax with sername exceeding 36
    strcpy (testdesc, "Test 7:dpns_modreplicax with poolname exceeding CA_MAXPOOLNAMELEN(EINVAL)");
    sprintf (filename, "%s/modreplicax_maxdfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Cannot create file modreplicax_maxsfn", sstrerror (serrno), 1);
        error = 1;
        goto test7_end;
    }

    char poolname[CA_MAXPOOLNAMELEN+10];
    memset (poolname, 70, CA_MAXPOOLNAMELEN+8); setname[CA_MAXPOOLNAMELEN+9] = '\0';
    ret = dpns_modreplicax ("some_file", "setname_new_1", poolname, "dpns_host_new_1", "fs_new_1", 'S');
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_modreplica returns -1, serrno is EINVAL", sstrerror(serrno), 0);
            goto test7_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplica returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test7_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplica returns 0", "", 1);
        goto test7_end;
    }

    test7_end:

        dpns_chmod (dirname, 0775);
        dpns_delreplica (NULL, NULL, sfn1);
        dpns_unlink (filename);

    //Test 8: dpns_modreplicax with sername exceeding 36
    strcpy (testdesc, "Test 8:dpns_modreplicax with poolname exceeding CA_MAXPOOLNAMELEN(EINVAL)");
    sprintf (filename, "%s/modreplicax_maxdfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        reportComponent (testdesc, "Cannot create file modreplicax_maxsfn", sstrerror (serrno), 1);
        error = 1;
        goto test8_end;
    }

    char hostname[CA_MAXHOSTNAMELEN+10];
    memset (hostname, 70, CA_MAXHOSTNAMELEN+8); setname[CA_MAXHOSTNAMELEN+9] = '\0';
    ret = dpns_modreplicax ("some_file", "setname_new_1", "poolname_new_1", hostname, "fs_new_1", 'P');
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is EINVAL", sstrerror(serrno), 0);
            goto test8_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modreplicax returns -1, serrno is unexpected", sstrerror(serrno), 1);
            goto test8_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modreplicax returns 0", "", 1);
        goto test8_end;
    }

    test8_end:

        dpns_chmod (dirname, 0775);
        dpns_delreplica (NULL, NULL, sfn1);
        dpns_unlink (filename);

    // Test 9: DPNS_HOST unknown
    strcpy (testdesc, "Test 9:Call dpns_modreplicax with unknown DPNS_HOST(SENOSHOST)");
    setenv ("DPNS_HOST", "host.which.does.not.exist", 1);
    ret = dpns_modreplicax ("a", "b", "c", "d", "e", 'S');
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
    dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



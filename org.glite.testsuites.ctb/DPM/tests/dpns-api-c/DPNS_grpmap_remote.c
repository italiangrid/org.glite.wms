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
        printf ("Usage: DPNS_getgrpmap_remote <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing DPNS groupmap remote tests...");
    fflush (stdout);

    char errorbuffer [4096];
    dpns_seterrbuf ( errorbuffer, 4096 );

    // Test 1:Test groupmap operations
    strcpy (testdesc, "Test 1:Test groupmap operations");

    int entcnt;
    struct dpns_groupinfo * entries = NULL;
    ret = dpns_getgrpmap (&entcnt, &entries);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading groupmap", sstrerror (serrno), 1);
        goto test1_end;
    }

    if ( entcnt > 1 )
        qsort(entries, entcnt, sizeof(struct dpns_groupinfo), CnsGroupInfoCompare);

    dpns_startsess(dpns_host, "dpns_entergrpmap test");

    int i;
    int created = 0;
    int rcnt = entcnt - 1;
    char fqan[1024];
    int newids[200];
    memset (newids, '\0', sizeof(int) * 200);
    for ( i = 32000; i > 0; --i )
    {
        while ( entries[rcnt].gid > i && rcnt > 0 ) --rcnt;

        if ( entries[rcnt].gid == i )
        {
            continue;
        }
        else
        {
            if ( created % 2 )
            {
                sprintf (fqan, "vo_name/group_name_%d", i);
                ret = dpns_entergrpmap (i, fqan);
            }
            else
            {
                sprintf (fqan, "vo_name/group_name/Role=Role_%d", i);
                ret = dpns_entergrpmap (i, fqan);
            }
            if ( ret != 0 )
            {
                error = 1;
                sprintf (errorstr, "Error registering group %d, %s", i, sstrerror (serrno));
                reportComponent (testdesc, errorstr, "", 1);
                goto test1_end;
            }
            else
            {
                newids[created] = i;
                ++created;
            }
        }
        if ( created == 200 ) break;
    }

    dpns_endsess();

    int entcnt2;
    struct dpns_groupinfo * entries2 = NULL;
    ret = dpns_getgrpmap (&entcnt2, &entries2);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading groupmap (2)", sstrerror (serrno), 1);
        goto test1_end;
    }

    if ( entcnt2 != entcnt + 200 )
    {
        error = 1;
        reportComponent (testdesc, "Unexpected groupmap entries count", "", 1);
        goto test1_end;
    }

    if ( entcnt2 > 1 )
        qsort(entries2, entcnt2, sizeof(struct dpns_groupinfo), CnsGroupInfoCompare);

    dpns_startsess (dpns_host, "Erase session");
    for ( i = 199; i > 140; --i )
    {
        if ( newids[i] != 0 )
        {
            ret = dpns_rmgrpmap (newids[i], NULL);
            if ( ret != 0 )
            {
                error = 1;
                reportComponent (testdesc, "Error during dpns_rmgrpmap operations by id only", sstrerror (serrno), 1);
                dpns_endsess();
                goto test1_end;
            }
        }
    }

    int fulcount = 0;
    for ( i = 140; i >= 0; --i )
    {
        if ( newids[i] != 0 )
        {
            while ( entries2[fulcount].gid != newids[i] && fulcount < entcnt2 ) ++fulcount;

            if ( fulcount == entcnt2 && i != 199 )
            {
                error = 1;
                reportComponent (testdesc, "Cannot find an added groupmap entry in the list returned by dpns_getgrpmap (2)", "", 1);
                dpns_endsess();
                goto test1_end;
            }

            if ( i % 2 )
            {
                ret = dpns_rmgrpmap (entries2[fulcount].gid, entries2[fulcount].groupname);
                if ( ret != 0 )
                {
                    error = 1;
                    reportComponent (testdesc, "Error during dpns_rmgrpmap operations by id and groupname", sstrerror (serrno), 1);
                    dpns_endsess();
                    goto test1_end;
                }
            }
            else
            {
                ret = dpns_rmgrpmap (0, entries2[fulcount].groupname);
                if ( ret != 0 )
                {
                    error = 1;
                    reportComponent (testdesc, "Error during dpns_rmgrpmap operations by groupname only", sstrerror (serrno), 1);
                    dpns_endsess();
                    goto test1_end;
                }
            }
        }
    }


    dpns_endsess();

    reportComponent (testdesc, "dpns_getgrpmap returned 0 and the groupmap mathches", "", 0);

    test1_end:

        ret = dpns_startsess (dpns_host, "Cleanup session");
        for ( i = 0; i < 200; ++i )
            if ( newids[i] != 0 )
                dpns_rmgrpmap (newids[i], NULL);
        dpns_endsess();

        if ( entries != NULL ) free (entries);
        if ( entries2 != NULL ) free (entries2);

    strcpy (testdesc, "Test 2:Call dpns_getgrpmap with NULL pointer for the entries count");
    ret = dpns_getgrpmap (NULL, &entries2);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test2_end;
        }
    }
    else 
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpmap returned 0. Unexpected", "", 1);
        goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:Call dpns_getgrpmap with NULL pointer for the entries");
    int ecount2;
    ret = dpns_getgrpmap (&ecount2, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getgrpmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getgrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test3_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getgrpmap returned 0. Unexpected", "", 1);
        goto test3_end;
    }

    test3_end:

    strcpy (testdesc, "Test 4:Call dpns_entergrpmap with NULL pointer for the entries");
    ret = dpns_entergrpmap (32767, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test4_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test4_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_entergrpmap returned 0. Unexpected", "", 1);
        goto test4_end;
    }

    test4_end:

        dpns_rmgrpmap (32767, NULL);

    strcpy (testdesc, "Test 5:Call dpns_entergrpmap with group which already exists (EEXIST)");
    ret = dpns_entergrpmap (-1, "New Test group for testing EEXIST");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Initial entry creation failed. dpns_entergrpmap returned 0. Unexpected", "", 1);
        goto test5_end;
    }

    ret = dpns_entergrpmap (-1, "New Test group for testing EEXIST");
    if ( ret != 0 )
    {
        if ( serrno == EEXIST )
        {
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is EEXIST", sstrerror (serrno), 0);
            goto test5_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test5_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_entergrpmap returned 0. Unexpected", "", 1);
        goto test5_end;
    }

    test5_end:

        dpns_rmgrpmap (0, "New Test group for testing EEXIST");
        dpns_rmgrpmap (0, "New Test group for testing EEXIST");

    strcpy (testdesc, "Test 6:Call dpns_entergrpmap with groupname length exceeding 255 (EINVAL)");
    char groupname[1024];
    memset (groupname, 70, 1023); groupname[1023] = '\0';
    ret = dpns_entergrpmap (-1, groupname);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test6_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_entergrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test6_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_entergrpmap returned 0. Unexpected", "", 1);
        goto test6_end;
    }

    test6_end:

    strcpy (testdesc, "Test 7:Call dpns_rmgrpmap with groupname length exceeding 255 (EINVAL)");
    char lgroupname[1024];
    memset (lgroupname, 70, 1023); lgroupname[1023] = '\0';
    ret = dpns_rmgrpmap (-1, lgroupname);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_rmgrpmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test7_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_rmgrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test7_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmgrpmap returned 0. Unexpected", "", 1);
        goto test7_end;
    }

    test7_end:

    strcpy (testdesc, "Test 8:Call dpns_rmgrpmap with groupname which does not exist (EINVAL)");
    ret = dpns_rmgrpmap (-1, "Some _wierd_ groupname which _does_ _not_ exist");
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_rmgrpmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test8_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_rmgrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test8_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmgrpmap returned 0. Unexpected", "", 1);
        goto test8_end;
    }

    test8_end:


    strcpy (testdesc, "Test 9:Call dpns_rmgrpmap - id and name doesn't point to the same entry (EINVAL)");
    ret = dpns_entergrpmap (-1, "Some _wierd_ groupname 1 which _does_ _not_ exist");
    ret += dpns_entergrpmap (-1, "Some _wierd_ groupname 2 which _does_ _not_ exist");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating test groups for scenario", "", 1);
        goto test9_end;
    }

    gid_t gidfirst;
    ret = dpns_getgrpbynam ("Some _wierd_ groupname 1 which _does_ _not_ exist", &gidfirst);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error getting groupid by name", sstrerror(serrno), 1);
        goto test9_end;
    }

    ret = dpns_rmgrpmap (gidfirst, "Some _wierd_ groupname 2 which _does_ _not_ exist");
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "serrno is EINVAL", sstrerror (serrno), 0);
            goto test9_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_rmgrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test9_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmgrpmap returned 0. Unexpected", "", 1);
        goto test9_end;
    }

    test9_end:

       dpns_rmgrpmap (0, "Some _wierd_ groupname 1 which _does_ _not_ exist");
       dpns_rmgrpmap (0, "Some _wierd_ groupname 2 which _does_ _not_ exist");

    strcpy (testdesc, "Test 10:Use dpns_modifygrpmap to modify an existing entry");

    ret = dpns_entergrpmap (-1, "Value 1 for some brand new group");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating test group for scenario", "", 1);
        goto test10_end;
    }

    ret = dpns_getgrpbynam ("Value 1 for some brand new group", &gidfirst);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error looking group id up by group name", sstrerror(serrno), 1);
        goto test10_end;
    }

    ret = dpns_modifygrpmap (gidfirst, "Value 1 for some brand new group/Role=SomeRoleAdded");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error during groupmap modification", sstrerror(serrno), 1);
        goto test10_end;
    }

    char groupfirst[1024];
    ret = dpns_getgrpbygid (gidfirst, groupfirst);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error looking group name up by group gid", sstrerror(serrno), 1);
        goto test10_end;
    }

    if ( strcmp (groupfirst, "Value 1 for some brand new group/Role=SomeRoleAdded") != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Modification successful by new name does not match.", sstrerror(serrno), 1);
        goto test10_end;
    }

    reportComponent (testdesc, "Modification successful. Group name matches", "", 0);

    test10_end:

       dpns_rmgrpmap (0, "Value 1 for some brand new group");
       dpns_rmgrpmap (0, "Value 1 for some brand new group/Role=SomeRoleAdded");

    strcpy (testdesc, "Test 11:dpns_modifygrpmap: groupname is a NULL pointer (EFAULT)");

    ret = dpns_modifygrpmap (-1, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_modifygrpmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test11_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modifygrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test11_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modifygrpmap returned 0. Unexpected", "", 1);
        goto test11_end;
    }

    test11_end:

    strcpy (testdesc, "Test 12:dpns_modifygrpmap: non-existent gid (EINVAL)");
    ret = dpns_getgrpmap (&entcnt, &entries);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading groupmap to find an unused GID", sstrerror (serrno), 1);
        goto test12_end;
    }

    int gidfound;
    if ( entcnt > 0 )
    {
        qsort(entries, entcnt, sizeof(struct dpns_groupinfo), CnsGroupInfoCompare);
        gidfound = entries[entcnt-1].gid + 1;
    }
    else
        gidfound = 101;

    ret = dpns_modifygrpmap (gidfound, "does not matter");
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_modifygrpmap returned an error. serrno is INVAL", sstrerror (serrno), 0);
            goto test12_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modifygrpmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test12_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modifygrpmap returned 0. Unexpected", "", 1);
        goto test12_end;
    }

    test12_end:

    reportFooter ("");
    reportOverall (error);

    return error;
}


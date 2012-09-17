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
        printf ("Usage: DPNS_usrmap_remote <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing DPNS usermap remote tests...");
    fflush (stdout);

    char errorbuffer [4096];
    dpns_seterrbuf ( errorbuffer, 4096 );

    // Test 1:Test usermap operations
    strcpy (testdesc, "Test 1:Test usermap operations");

    int entcnt;
    struct dpns_userinfo * entries = NULL;
    ret = dpns_getusrmap (&entcnt, &entries);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading usermap", sstrerror (serrno), 1);
        goto test1_end;
    }

    if ( entcnt > 1 )
        qsort(entries, entcnt, sizeof(struct dpns_userinfo), CnsUserInfoCompare);

    dpns_startsess(dpns_host, "dpns_enterusrmap test");

    int i;
    int created = 0;
    int rcnt = entcnt - 1;
    char userdn[1024];
    int newids[200];
    memset (newids, '\0', sizeof(int) * 200);
    for ( i = 32000; i > 0; --i )
    {
        while ( entries[rcnt].userid > i && rcnt > 0 ) --rcnt;

        if ( entries[rcnt].userid == i )
        {
            continue;
        }
        else
        {
            sprintf (userdn, "/DC=CH/DC=CERN/OU=REGCA/CN=Test user new %d", i);
            ret = dpns_enterusrmap (i, userdn);
            if ( ret != 0 )
            {
                error = 1;
                sprintf (errorstr, "Error registering user %d, %s", i, sstrerror (serrno));
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
    struct dpns_userinfo * entries2 = NULL;
    ret = dpns_getusrmap (&entcnt2, &entries2);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading usermap (2)", sstrerror (serrno), 1);
        goto test1_end;
    }

    if ( entcnt2 != entcnt + 200 )
    {
        error = 1;
        reportComponent (testdesc, "Unexpected usermap entries count", "", 1);
        goto test1_end;
    }

    if ( entcnt2 > 1 )
        qsort(entries2, entcnt2, sizeof(struct dpns_userinfo), CnsUserInfoCompare);

    dpns_startsess (dpns_host, "Erase session");
    for ( i = 199; i > 140; --i )
    {
        if ( newids[i] != 0 )
        {
            ret = dpns_rmusrmap (newids[i], NULL);
            if ( ret != 0 )
            {
                error = 1;
                reportComponent (testdesc, "Error during dpns_rmusrmap operations by id only", sstrerror (serrno), 1);
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
            while ( entries2[fulcount].userid != newids[i] && fulcount < entcnt2 ) ++fulcount;

            if ( fulcount == entcnt2 && i != 199 )
            {
                error = 1;
                reportComponent (testdesc, "Cannot find an added usermap entry in the list returned by dpns_getusrmap (2)", "", 1);
                dpns_endsess();
                goto test1_end;
            }

            if ( i % 2 )
            {
                ret = dpns_rmusrmap (entries2[fulcount].userid, entries2[fulcount].username);
                if ( ret != 0 )
                {
                    error = 1;
                    reportComponent (testdesc, "Error during dpns_rmusrmap operations by id and username", sstrerror (serrno), 1);
                    dpns_endsess();
                    goto test1_end;
                }
            }
            else
            {
                ret = dpns_rmusrmap (0, entries2[fulcount].username);
                if ( ret != 0 )
                {
                    error = 1;
                    reportComponent (testdesc, "Error during dpns_rmusrmap operations by username only", sstrerror (serrno), 1);
                    dpns_endsess();
                    goto test1_end;
                }
            }
        }
    }


    dpns_endsess();

    reportComponent (testdesc, "dpns_getusrmap returned 0 and the usermap mathches", "", 0);

    test1_end:

        dpns_startsess (dpns_host, "Cleanup session");
        for ( i = 0; i < 200; ++i )
            if ( newids[i] != 0 )
                dpns_rmusrmap (newids[i], NULL);
        dpns_endsess();
        if ( entries != NULL ) free (entries);
        if ( entries2 != NULL ) free (entries2);

    strcpy (testdesc, "Test 2:Call dpns_getusrmap with NULL pointer for the entries count");
    ret = dpns_getusrmap (NULL, &entries2);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getusrmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test2_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test2_end;
        }
    }
    else 
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrmap returned 0. Unexpected", "", 1);
        goto test2_end;
    }

    test2_end:

    strcpy (testdesc, "Test 3:Call dpns_getusrmap with NULL pointer for the entries");
    int ecount2;
    ret = dpns_getusrmap (&ecount2, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_getusrmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test3_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_getusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test3_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_getusrmap returned 0. Unexpected", "", 1);
        goto test3_end;
    }

    test3_end:

    strcpy (testdesc, "Test 4:Call dpns_enterusrmap with NULL pointer for the entries");
    ret = dpns_enterusrmap (32767, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test4_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test4_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_enterusrmap returned 0. Unexpected", "", 1);
        goto test4_end;
    }

    test4_end:

        dpns_rmusrmap (32767, NULL);

    strcpy (testdesc, "Test 5:Call dpns_enterusrmap with user which already exists (EEXIST)");
    ret = dpns_enterusrmap (-1, "New Test user for testing EEXIST");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Initial entry creation failed. dpns_enterusrmap returned 0. Unexpected", "", 1);
        goto test5_end;
    }

    ret = dpns_enterusrmap (-1, "New Test user for testing EEXIST");
    if ( ret != 0 )
    {
        if ( serrno == EEXIST )
        {
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is EEXIST", sstrerror (serrno), 0);
            goto test5_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test5_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_enterusrmap returned 0. Unexpected", "", 1);
        goto test5_end;
    }

    test5_end:

        dpns_rmusrmap (0, "New Test user for testing EEXIST");
        dpns_rmusrmap (0, "New Test user for testing EEXIST");

    strcpy (testdesc, "Test 6:Call dpns_enterusrmap with username length exceeding 255 (EINVAL)");
    char username[1024];
    memset (username, 70, 1023); username[1023] = '\0';
    ret = dpns_enterusrmap (-1, username);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test6_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_enterusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test6_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_enterusrmap returned 0. Unexpected", "", 1);
        goto test6_end;
    }

    test6_end:

    strcpy (testdesc, "Test 7:Call dpns_rmusrmap with username length exceeding 255 (EINVAL)");
    char lusername[1024];
    memset (lusername, 70, 1023); lusername[1023] = '\0';
    ret = dpns_rmusrmap (-1, lusername);
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_rmusrmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test7_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_rmusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test7_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmusrmap returned 0. Unexpected", "", 1);
        goto test7_end;
    }

    test7_end:

    strcpy (testdesc, "Test 8:Call dpns_rmusrmap with username which does not exist (EINVAL)");
    ret = dpns_rmusrmap (-1, "Some _wierd_ username which _does_ _not_ exist");
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_rmusrmap returned an error. serrno is EINVAL", sstrerror (serrno), 0);
            goto test8_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_rmusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test8_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmusrmap returned 0. Unexpected", "", 1);
        goto test8_end;
    }

    test8_end:


    strcpy (testdesc, "Test 9:Call dpns_rmusrmap - id and name doesn't point to the same entry (EINVAL)");
    ret = dpns_enterusrmap (-1, "Some _wierd_ username 1 which _does_ _not_ exist");
    ret += dpns_enterusrmap (-1, "Some _wierd_ username 2 which _does_ _not_ exist");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error creating test users for scenario", "", 1);
        goto test9_end;
    }

    uid_t uidfirst;
    ret = dpns_getusrbynam ("Some _wierd_ username 1 which _does_ _not_ exist", &uidfirst);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error getting userid by name", "", 1);
        goto test9_end;
    }

    ret = dpns_rmusrmap (uidfirst, "Some _wierd_ username 2 which _does_ _not_ exist");
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
            reportComponent (testdesc, "dpns_rmusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test9_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_rmusrmap returned 0. Unexpected", "", 1);
        goto test9_end;
    }

    test9_end:
       dpns_rmusrmap (0, "Some _wierd_ username 1 which _does_ _not_ exist");
       dpns_rmusrmap (0, "Some _wierd_ username 2 which _does_ _not_ exist");

    strcpy (testdesc, "Test 11:dpns_modifyusrmap: username is a NULL pointer (EFAULT)");

    ret = dpns_modifyusrmap (-1, NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
        {
            reportComponent (testdesc, "dpns_modifyusrmap returned an error. serrno is EFAULT", sstrerror (serrno), 0);
            goto test11_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modifyusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test11_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modifyusrmap returned 0. Unexpected", "", 1);
        goto test11_end;
    }

    test11_end:

    strcpy (testdesc, "Test 12:dpns_modifyusrmap: non-existent uid (EINVAL)");
    ret = dpns_getusrmap (&entcnt, &entries);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error reading usermap to find an unused UID", sstrerror (serrno), 1);
        goto test12_end;
    }

    int uidfound;
    if ( entcnt > 0 )
    {
        qsort(entries, entcnt, sizeof(struct dpns_userinfo), CnsUserInfoCompare);
        uidfound = entries[entcnt-1].userid + 1;
    }
    else
        uidfound = 101;

    ret = dpns_modifyusrmap (uidfound, "does not matter");
    if ( ret != 0 )
    {
        if ( serrno == EINVAL )
        {
            reportComponent (testdesc, "dpns_modifyusrmap returned an error. serrno is INVAL", sstrerror (serrno), 0);
            goto test12_end;
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "dpns_modifyusrmap returned an error. serrno is unexpected", sstrerror (serrno), 1);
            goto test12_end;
        }
    }
    else
    {
        error = 1;
        reportComponent (testdesc, "dpns_modifyusrmap returned 0. Unexpected", "", 1);
        goto test12_end;
    }

    test12_end:

    reportFooter ("");
    reportOverall (error);

    return error;
}


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
    char sfn[256], sfn2[256], sfn3[256], sfn4[256];
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_delreplicasbysfn <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_delreplicasbysfn tests...");

    // Test 1: Create four replicas and delete them alltogether
    strcpy (testdesc, "Test 1:Create four replicas and delete them alltogether");
    sprintf (filename, "%s/file_delrepsbysfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn, "%s:/fs1/file_delreps.1", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn, '-', 'P', "pool1", "/fs1", 'P', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (1)", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn2, "%s:/fs2/file_delreps.2", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn2, '-', 'V', "pool1", "/fs1", 'S', "setname_2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (2)", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn3, "%s:/fs1/file_delreps.3", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn3, '-', 'V', "pool1", "/fs1", 'P', "setname_3");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (3)", sstrerror(serrno), 1);
        goto test1_end;
    }

    sprintf (sfn4, "%s:/fs1/file_delreps.4", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn4, '-', 'D', "pool1", "/fs1", 'S', "setname_4");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (4)", sstrerror(serrno), 1);
        goto test1_end;
    }

    int statuscount;
    int *statuses;
    const char * sfns [] = { sfn, sfn2, sfn3, sfn4 };

    int i;
    ret = dpns_delreplicasbysfn (4, sfns, NULL, &statuscount, &statuses);
    if ( ret == 0 )
    {
        for (i = 0; i < 4; ++i)
        {
            if ( statuses[i] !=  0 )
            {
                sprintf (errorstr, "Error deleting replica %d, serrno=%d", i + 1, statuses[i]);
                reportComponent (testdesc, errorstr, sstrerror (statuses[i]), 1);
                error = 1;
                goto test1_end;
            }
        }
        free (statuses);
    }
    else
    {
        reportComponent (testdesc, "dpns_delreplicasbysfn failed", sstrerror (serrno), 1);
        error = 1;
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_delreplica call completed successfully", "", 0);

    test1_end:
        dpns_delreplica (NULL, NULL, sfn);
        dpns_delreplica (NULL, NULL, sfn2);
        dpns_delreplica (NULL, NULL, sfn3);
        dpns_delreplica (NULL, NULL, sfn4);
        dpns_unlink (filename);


    // Test 2: Create replicas and call dpns_delreplicasbysfn with one wrong 
    strcpy (testdesc, "Test 2:Call dpns_replicasbysfn with one non-existing entry");
    sprintf (filename, "%s/file_delrepsbysfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test2_end;
    }

    sprintf (sfn, "%s:/fs1/file_delreps.1", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn, '-', 'P', "pool1", "/fs1", 'P', "setname_1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (1)", sstrerror(serrno), 1);
        goto test2_end;
    }

    sprintf (sfn2, "%s:/fs2/file_delreps.2", dpns_host);
    ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfn2, '-', 'V', "pool1", "/fs1", 'S', "setname_2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot register replica (2)", sstrerror(serrno), 1);
        goto test2_end;
    }

    const char * sfnse [] = { sfn, "wrong.host:/non/existing/replica" };

    ret = dpns_delreplicasbysfn (2, sfnse, NULL, &statuscount, &statuses);
    if ( ret == 0 )
    {
        if ( statuses[0] !=  0 )
        {
            sprintf (errorstr, "Error deleting replica %d, serrno=%d, unexpected", i + 1, statuses[0]);
            reportComponent (testdesc, errorstr, sstrerror (statuses[0]), 1);
            error = 1;
            goto test2_end;
        }
        if ( statuses[1] !=  0 )
        {
            sprintf (errorstr, "Error deleting replica %d, serrno=%d, expected", i + 1, statuses[1]);
            reportComponent (testdesc, errorstr, sstrerror (statuses[1]), 0);
            goto test2_end;
        }
        else
        {
            reportComponent (testdesc, "Non-existing replica deleted successfully, unexpected", "", 1);
            error = 1;
            goto test2_end;
        }
        free (statuses);
    }
    else
    {
        reportComponent (testdesc, "dpns_delreplicasbysfn failed", sstrerror (serrno), 1);
        error = 1;
        goto test2_end;
    }

    test2_end:
        dpns_delreplica (NULL, NULL, sfn);
        dpns_delreplica (NULL, NULL, sfn2);
        dpns_unlink (filename);

    // Test 3: Request too big (E2BIG)
    strcpy (testdesc, "Test 3:Request too big (E2BIG)");
    sprintf (filename, "%s/file_delrepsbysfn", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test2_end;
    }

    char sfnx[1100];
    strcpy (sfnx, "some.host:/filesystem/directory/");
    memset (sfnx + 32, 70, 1024);

    dpns_startsess(dpns_host, "Bulk replica additions");
    for (i = 0; i < 1024; ++i )
    {
        sprintf (sfnx + 1056, "%d", i);
        ret = dpns_addreplicax (NULL, &dpns_fid, dpns_host, sfnx, '-', 'P', "pool1", "/fs1", 'P', "setname_1");
        if ( ret != 0 )
        {
            error = 1;
            sprintf (errorstr, "Cannot register replica %d", i);
            reportComponent (testdesc, errorstr, sstrerror(serrno), 1);
            goto test3_end;
        }
    }
    dpns_endsess();

    char files[1024][1100];
    char * sfnse2[1024];

    for (i = 0; i < 1024; ++i )
    {
        sprintf (sfnx + 1056, "%d", i);
        strcpy (files[i], sfnx);
        sfnse2[i] = files[i];
    }

    ret = dpns_delreplicasbysfn (1024, (const char **)sfnse2, NULL, &statuscount, &statuses);
    if ( ret == 0 )
    {
        for (i = 0; i < 1024; ++i)
        {
            if ( statuses[i] !=  0 )
            {
                sprintf (errorstr, "Error deleting replica %d, serrno=%d", i + 1, statuses[i]);
                reportComponent (testdesc, errorstr, sstrerror (statuses[i]), 1);
                error = 1;
                goto test1_end;
            }
        }
        free (statuses);
    }
    else
    {
        if ( serrno == E2BIG ||  serrno == SECOMERR )
        {
            reportComponent (testdesc, "dpns_delreplicasbysfn failed with E2BIG or SECOMERR", sstrerror (serrno), 0);
            goto test3_end;
        }
        else
        {
            reportComponent (testdesc, "dpns_delreplicasbysfn failed with unexpected serrno", sstrerror (serrno), 1);
            error = 1;
            goto test3_end;
        }
    }
 
    test3_end:
        dpns_startsess(dpns_host, "Bulk replica removals");
        for (i = 0; i < 1024; ++i )
        {
            sprintf (sfnx + 1056, "%d", i);
            ret = dpns_delreplica (NULL, NULL, sfnx);
            if ( ret != 0 )
            {
                error = 1;
                sprintf (errorstr, "Cannot remove replica %d", i);
                reportComponent (testdesc, errorstr, sstrerror(serrno), 1);
                goto test3_end;
            }
        }
        dpns_endsess();
        dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}


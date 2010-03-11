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
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_addreplica <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_addreplica tests...");

    // Test 1: Create a file in <BASE_DIR> and try register a replica.
    strcpy (testdesc, "Test 1:Create a file and try register some replicas");
    sprintf (filename, "%s/file_addrep", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs1/email/file_addrep.1", '-', 'P', "siteX", "/fs1");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 1", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs2/email/file_addrep.2", 'D', 'V', "siteX", "/fs2");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 2", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs3/email/file_addrep.3", 'P', 'V', "siteX", "/fs3");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create replica 3", sstrerror(serrno), 1);
        goto test1_end;
    }

    // Check the result with dpns_listreplica
    serrno = 0;
    struct dpns_filereplica * reps;
    dpns_list rlist;
    int rflag = CNS_LIST_BEGIN;
    int repcount = 0;
    int rerror = 0;
    while ( (reps = dpns_listreplica (filename, NULL, rflag, &rlist)) != NULL )
    {
        if ( strcmp (reps->sfn, "disk64.cern.ch:/fs1/email/file_addrep.1") != 0 &&
             strcmp (reps->sfn, "disk64.cern.ch:/fs2/email/file_addrep.2") != 0 &&
             strcmp (reps->sfn, "disk64.cern.ch:/fs3/email/file_addrep.3") != 0 )
            rerror = 1;

        if ( strcmp (reps->sfn, "disk64.cern.ch:/fs3/email/file_addrep.1") == 0 && 
             ( reps->status != '-' || reps->f_type != 'P' ) )
            rerror = 1;

        if ( strcmp (reps->sfn, "disk64.cern.ch:/fs3/email/file_addrep.1") == 0 &&
             ( reps->status != 'D' || reps->f_type != 'V' ) )
            rerror = 1;

        if ( strcmp (reps->sfn, "disk64.cern.ch:/fs3/email/file_addrep.1") == 0 &&
             ( reps->status != 'P' || reps->f_type != 'V' ) )
            rerror = 1;

        rflag = CNS_LIST_CONTINUE;
        ++repcount;
    }
    if ( serrno != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Error while reading replica list", sstrerror(serrno), 1);
        dpns_listreplica (filename, NULL, CNS_LIST_END, &rlist);
        goto test1_end;
    }
    dpns_listreplica (filename, NULL, CNS_LIST_END, &rlist);

    if ( repcount == 3 && rerror == 0 )
        reportComponent (testdesc, "dpns_addreplica calls completed successfully", "", 0);
    else
        reportComponent (testdesc, "dpns_addreplica calls OK but replica count check failed", "", 1);

    test1_end:

        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs1/email/file_addrep.1");
        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs2/email/file_addrep.2");
        dpns_delreplica (NULL, &dpns_fid, "disk64.cern.ch:/fs3/email/file_addrep.3");
        sprintf (filename, "%s/file_addrep", base_dir);
        ret = dpns_unlink (filename);








    // Test 3: Execute for non-existing file
    strcpy (testdesc, "Test 3:Execute for non-existing file(ENOENT)");
    struct dpns_fileid fid;
    strcpy (fid.server, dpns_host);
    fid.fileid = 0x0FFFFFFF;
    fid.fileid = fid.fileid << 32;
    fid.fileid += 0x0FFFFFFF;
    ret = dpns_addreplica (NULL, &dpns_fid, "disk64.cern.ch", "disk64.cern.ch:/fs3/email/file_addrep.3", 'P', 'V', "siteX", "/fs3");
    if ( ret != 0 )
    {
        if ( serrno == ENOENT )
            reportComponent (testdesc, "returns -1, ENOENT", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    // Test 4: Call dpns_addreplica with NULL arguments
    strcpy (testdesc, "Test 4:Call dpns_addreplica with NULL arguments(EFAULT)");
    ret = dpns_addreplica (NULL, NULL, NULL, NULL, 'P', 'V', "siteX", "/fs3");
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
            reportComponent (testdesc, "Returns -1. Serrno OK", sstrerror(serrno), 0);
        else
        {
            reportComponent (testdesc, "Returns -1. Unexpected serrno", sstrerror(serrno), 1);
            error = 1;
        }
    }
    else
    {
        reportComponent (testdesc, "Returns 0", "", 1);
        error = 1;
    }

    // Test 5: dpns_addreplica for a file in inaccessible directory (permission is not granted)
    strcpy (testdesc, "Test 5:dpns_addreplica access denied scenario (EACCES)");
    sprintf (dirname, "%s/dpns_addreplica_access", base_dir);
    sprintf (filename, "%s/dpns_addreplica_access/file_to_do", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret == 0 )
    {
        struct dpns_fileid fid;
        ret = dpns_creatx (filename, 0664, &fid);
        if ( ret != 0 )
        {
            reportComponent (testdesc, "Error creating file", sstrerror(serrno), 1);
            error = 1;
            goto test5_end;
        }
        ret = dpns_chmod (dirname, 0);
        if ( ret == 0 )
        {
            ret = dpns_addreplica (NULL, &fid, "disk64.cern.ch", "disk64.cern.ch:/fs3/email/file_addrep.3", 'P', 'V', "siteX", "/fs3");
            if ( ret != 0 )
            {
                if ( serrno == EACCES )
                    reportComponent (testdesc, "Returns -1", sstrerror(serrno), 0);
                else
                {
                    reportComponent (testdesc, "Returns -1", sstrerror(serrno), 1);
                    error = 1;
                }
            }
            else
            {
                reportComponent (testdesc, "Returns 0", "", 1);
                error = 1;
            }
        }
        else
        {
            error = 1;
            reportComponent (testdesc, "Error setting directory permissions", "", 1);
        }
    }
    else
    {
        reportComponent (testdesc, "Error creating inaccessible directory", sstrerror(serrno), 1);
        error = 1;
    }
    test5_end:
        dpns_chmod (dirname, 0770);
        dpns_unlink (filename);
        dpns_rmdir (dirname);

    reportFooter ("");
    reportOverall (error);

    return error;
}



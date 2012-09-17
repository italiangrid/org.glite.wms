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
    char messagex[256];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char * envvar;
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_setfsizec <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_setfsizec tests...");

    // Test 1: Create a file in <BASE_DIR> and set its size, checksum type and value
    strcpy (testdesc, "Test 1:Create a file and try set its size, checksum type and value");
    sprintf (filename, "%s/file_fsizec", base_dir);
    ret = dpns_creatx (filename, 0664, &dpns_fid);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create file", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filestatg fstatg;
    ret = dpns_statg (filename, NULL, &fstatg);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot dpns_statg file", sstrerror(serrno), 1);
        goto test1_end;
    }
    time_t mtime1 = fstatg.mtime;

    sleep (3);

    ret = dpns_setfsizec (filename, NULL, (u_signed64) 716253, "AD", "43835fa8");
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setfsizec completed with an error", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_statg (filename, NULL, &fstatg);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Second stat failed", sstrerror(serrno), 1);
        goto test1_end;
    }
    time_t mtime2 = fstatg.mtime;
    
    if ( mtime2 - mtime1 > 10 || mtime2 - mtime1 < 0)
    {
        error = 1;
        sprintf (messagex, "Error found when comparing mtimes (difference is %lu)", mtime2 - mtime1);
        reportComponent (testdesc, messagex, "", 1);
        goto test1_end;
    }

    if ( fstatg.filesize - 716253 != 0 )
    {
        error = 1;
        reportComponent (testdesc, "File size does not match", "", 1);
        goto test1_end;
    }

    if ( strcmp (fstatg.csumtype, "AD") != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Checksum type does not match", "", 1);
        goto test1_end;
    }

    if ( strcmp (fstatg.csumvalue, "43835fa8") != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Checksum value does not match", "", 1);
        goto test1_end;
    }

    reportComponent (testdesc, "dpns_setfsize worked as expected", "", 0);

    test1_end:

        sprintf (filename, "%s/file_fsizec", base_dir);
        ret = dpns_unlink (filename);

    reportFooter ("");
    reportOverall (error);

    return error;
}



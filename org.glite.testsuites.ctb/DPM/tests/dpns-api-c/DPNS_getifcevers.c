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
    char testdesc[256];
    char errorstr[256];

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_getifcevers <DPNS_HOST> <VERSION>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* version = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);

    reportHeader ("* Executing dpns_getifcevers tests...");

    strcpy (testdesc, "Test 1:Check client version");

    char gotver[256];

    ret = dpns_getifcevers (gotver);

    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getifcevers returns with error", sstrerror (serrno), 1);
    }
    else
    {
        if ( strcmp (version, gotver) == 0 )
            reportComponent (testdesc, "dpns_getifcevers returns OK. Version matches.", "", 0);
        else
        {
            error = 1;
            sprintf (errorstr, "dpns_getifcevers returns 0. Version does not match. Returned %s", gotver);
            reportComponent (testdesc, errorstr, "", 1);
        }
    }

    reportFooter ();
    reportOverall (error);

    return error;
}


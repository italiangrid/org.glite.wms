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

    if ( argc != 2 )
    {
        printf ("Usage: dpm-delreplica <sfn>\n");
        return -1;
    }

    ret = dpm_delreplica (argv[1]);
    if ( ret != 0 )
    {
        printf ("Error during dpm_delreplica: %s\n", sstrerror(serrno));
        return -1;
    }

    return 0;
}



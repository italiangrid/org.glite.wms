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
        printf ("Usage: dpns-setptime <file>\n");
        return -1;
    }

    time_t now = time (NULL);
    now = now + 128000;
    ret = dpns_setptime (argv[1], now);
    if ( ret != 0 )
    {
        printf ("Error during dpns_setptime: %s\n", sstrerror(serrno));
        return -1;
    }

    return 0;
}



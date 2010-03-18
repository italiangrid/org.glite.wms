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
    if ( argc != 2 )
    {
        printf ("Usage: dpns-creat <file>\n");
        return -1;
    }

    int ret = dpns_creat (argv[1], 0775);

    if ( ret != 0 )
        printf ("%s\n", sstrerror(serrno));

    return ret;
}



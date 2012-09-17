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

    if ( argc != 3 )
    {
        printf ("Usage: dpns-modifyreplica <replica> <rtype>\n");
        return -1;
    }

    ret = dpns_modreplicax (argv[1], NULL, "site1", "head32.cern.ch", "/fs1", argv[2][0]);
    if ( ret != 0 )
    {
        printf ("Error during dpns_modreplica: %s\n", sstrerror(serrno));
        return -1;
    }

    return 0;
}



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
        printf ("Usage: dpns-creat <file> <replica>\n");
        return -1;
    }

    struct dpns_fileid fid;
    struct dpns_filestat statbuf;

    ret = dpns_statx (argv[1], &fid, &statbuf);
    if ( ret != 0 )
    {
        printf ("Error during dpns_stat: %s\n", sstrerror(serrno));
        return -1;
    }

    ret = dpns_addreplica (NULL, &fid, getenv("DPNS_HOST"), argv[2], '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        printf ("Error during dpns_addreplica: %s\n", sstrerror(serrno));
        return -1;
    }

    return 0;
}



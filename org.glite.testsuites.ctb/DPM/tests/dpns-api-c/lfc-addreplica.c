#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <serrno.h>
#include <limits.h>
#include <unistd.h>
#include <openssl/pem.h>
#include <lfc_api.h>
#include "utils.h"

int main (int argc, char** argv)
{
    int ret;

    if ( argc != 3 )
    {
        printf ("Usage: lfc-creat <file> <replica>\n");
        return -1;
    }

    struct lfc_fileid fid;
    struct lfc_filestat statbuf;

    ret = lfc_statx (argv[1], &fid, &statbuf);
    if ( ret != 0 )
    {
        printf ("Error during lfc_stat: %s\n", sstrerror(serrno));
        return -1;
    }

    ret = lfc_addreplica (NULL, &fid, getenv("DPM_HOST"), argv[2], '-', 'P', "pool1", "/fs1");
    if ( ret != 0 )
    {
        printf ("Error during lfc_addreplica: %s\n", sstrerror(serrno));
        return -1;
    }

    return 0;
}



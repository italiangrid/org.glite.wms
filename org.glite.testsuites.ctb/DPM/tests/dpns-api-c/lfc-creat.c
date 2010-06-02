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
#include <uuid/uuid.h>
#include "utils.h"

int main (int argc, char** argv)
{
    if ( argc != 2 )
    {
        printf ("Usage: lfc-creat <file>\n");
        return -1;
    }

    char uuid[37];
    uuid_t uuidb;
    uuid_generate (uuidb);
    uuid_unparse (uuidb, uuid);

    int ret = lfc_creatg (argv[1], uuid, 0775);

    if ( ret != 0 )
        printf ("%s\n", sstrerror(serrno));

    return ret;
}



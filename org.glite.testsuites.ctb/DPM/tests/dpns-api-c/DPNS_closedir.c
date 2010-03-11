#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <errno.h>
#include <serrno.h>

int main (int argc, char** argv)
{
    int ret;
    int error = 0;
    dpns_DIR* dir;
    char testdesc[256];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_closedir <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];

    setenv ("DPNS_HOST", dpns_host, 1);

    printf ("\n* Executing dpns_closedir tests...\n");
    printf ("--------------------------------------------------------------------------------\n");

    // Test 1: Close successfully opened directory <BASE_DIR>
    strcpy (testdesc, "Test 1:Close successfully opened directory");
    dir = dpns_opendir (base_dir);
    if ( dir == NULL )
    {
        printf ("%s:CANNOT PREPARE ENVIRONMENT::FAILURE\n", testdesc);
        error = 1;
    }
    else
    {
        ret = dpns_closedir (dir);
        if ( ret == 0 )
            printf ("%s:RETURNS 0::SUCCESS\n", testdesc);
        else
        {
            printf ("%s:RETURNS -1:%s:FAILURE\n", testdesc, sstrerror(serrno));
            error = 1;
        }
    }

    // Test 2: Call dpns_closedir with NULL argument
    strcpy (testdesc, "Test 2:Call dpns_closedir with NULL argument(EFAULT)");
    ret = dpns_closedir (NULL);
    if ( ret != 0 )
    {
        if ( serrno == EFAULT )
            printf ("%s:RETURNS %d:%s:SUCCESS\n", testdesc, ret, sstrerror(serrno));
        else
        {
            printf ("%s:RETURNS %d:Unexpected serrno (%d) - %s:FAILURE\n", testdesc, ret, serrno, sstrerror(serrno));
            error = 1;
        }
    }
    else
    {
        printf ("%s:RETURNS 0::FAILURE\n", testdesc);
        error = 1;
    }

    printf ("--------------------------------------------------------------------------------\n");

    if ( error == 0 )
        printf ("Overall result:SUCCESS\n\n");
    else
        printf ("Overall result:FAILURE\n\n");

    return error;
}


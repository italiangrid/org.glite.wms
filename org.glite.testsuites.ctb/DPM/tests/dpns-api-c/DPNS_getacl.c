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
    dpns_DIR* dir;
    char testdesc[256];
    char dirname[CA_MAXPATHLEN + 2];
    char filename[CA_MAXPATHLEN + 2];
    char * envvar;
    char errorstr[256];
    char cert1[PATH_MAX];
    char cert2[PATH_MAX];
    struct dpns_fileid dpns_fid;

    if ( argc != 3 )
    {
        printf ("Usage: DPNS_getacl <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);
    reportHeader ("* Executing dpns_getacl tests...");

    // Test 1: Create a directory and obtain the ACL 
    strcpy (testdesc, "Create a directory and obtain the ACL");
    sprintf (dirname, "%s/dir_getacl", base_dir);
    ret = dpns_mkdir (dirname, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create directory", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_acl * list = NULL;
    list = malloc ( sizeof (struct dpns_acl) * 128 );
    if ( list == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot allocate memory for the ACL", "", 1);
        goto test1_end;
    }

    ret = dpns_getacl (dirname, 128, list);
    if ( ret == -1 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getacl call failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( ret < 6 )
    {
        error = 1;
        reportComponent (testdesc, "ACE count below the minimum of 6 entries", sstrerror(serrno), 1);
        goto test1_end;
    }

    int i;
    unsigned char bitmap = 0;
    for ( i = 0; i < ret; ++i )
    {
        switch ( list[i].a_type )
        {
            case CNS_ACL_USER_OBJ :

                if ( (bitmap & 0x01) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_USER_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x01;

                break;

            case CNS_ACL_USER :

                break;

            case CNS_ACL_GROUP_OBJ:

                if ( (bitmap & 0x02) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_GROUP_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x02;

                break;

            case CNS_ACL_GROUP :

                break;

            case CNS_ACL_MASK :

                break;

            case CNS_ACL_OTHER :

                if ( (bitmap & 0x04) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_OTHER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x04;

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_USER_OBJ : 

                if ( (bitmap & 0x10) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate DEFAULT CNS_ACL_USER_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x10;

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_USER : 

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_GROUP_OBJ : 

                if ( (bitmap & 0x20) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate DEFAULT CNS_ACL_GROUP_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x20;

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_GROUP : 

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_MASK : 

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_OTHER : 

                if ( (bitmap & 0x40) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate DEFAULT CNS_ACL_OTHER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x40;

                break;

            default :

                error = 1;
                reportComponent (testdesc, "Unknown ACE type found", "", 1);
                goto test1_end;
        }
    }

    if ( bitmap != 0x77 )
    {
        error = 1;
        sprintf (errorstr, "Obligatory entry is missing, bitmap is 0x%x", bitmap);
        reportComponent (testdesc, errorstr  , "", 1);
        goto test1_end;
    }

    test1_end:

        if ( list == NULL )
            free (list);
        sprintf (dirname, "%s/dir_getacl", base_dir);
        dpns_rmdir (dirname);


    // Test 2: Create a file and obtain the ACL
    strcpy (testdesc, "Create a file and obtain the ACL");
    sprintf (filename, "%s/file_getacl", base_dir);
    ret = dpns_creat (filename, 0775);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot create a file", sstrerror(serrno), 1);
        goto test2_end;
    }

    list = malloc ( sizeof (struct dpns_acl) * 128 );
    if ( list == NULL )
    {
        error = 1;
        reportComponent (testdesc, "Cannot allocate memory for the ACL", "", 1);
        goto test1_end;
    }

    ret = dpns_getacl (filename, 128, list);
    if ( ret == -1 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_getacl call failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( ret < 3 )
    {
        error = 1;
        reportComponent (testdesc, "ACE count below the minimum of 3 entries", sstrerror(serrno), 1);
        goto test1_end;
    }

    bitmap = 0;
    for ( i = 0; i < ret; ++i )
    {
        switch ( list[i].a_type )
        {
            case CNS_ACL_USER_OBJ :

                if ( (bitmap & 0x01) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_USER_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x01;

                break;

            case CNS_ACL_USER :

                break;

            case CNS_ACL_GROUP_OBJ:

                if ( (bitmap & 0x02) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_GROUP_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x02;

                break;

            case CNS_ACL_GROUP :

                break;

            case CNS_ACL_MASK :

                break;

            case CNS_ACL_OTHER :

                if ( (bitmap & 0x04) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_OTHER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                    bitmap = bitmap | 0x04;

                break;

            case CNS_ACL_DEFAULT | CNS_ACL_USER_OBJ :
            case CNS_ACL_DEFAULT | CNS_ACL_USER :
            case CNS_ACL_DEFAULT | CNS_ACL_GROUP_OBJ :
            case CNS_ACL_DEFAULT | CNS_ACL_GROUP :
            case CNS_ACL_DEFAULT | CNS_ACL_MASK :
            case CNS_ACL_DEFAULT | CNS_ACL_OTHER :

                error = 1;
                reportComponent (testdesc, "Unexpected  ACE type found for a file", "", 1);
                goto test1_end;

            default :

                error = 1;
                reportComponent (testdesc, "Unknown  ACE type found", "", 1);
                goto test1_end;
        }
    }

    if ( bitmap != 0x07 )
    {
        error = 1;
        sprintf (errorstr, "Obligatory entry is missing, bitmap is 0x%x", bitmap);
        reportComponent (testdesc, errorstr  , "", 1);
        goto test1_end;
    }

    test2_end:

        if ( list == NULL )
            free (list);
        sprintf (filename, "%s/file_getacl", base_dir);
        dpns_unlink (filename);


    reportFooter ("");
    reportOverall (error);

    return error;
}



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
        printf ("Usage: DPNS_setacl <DPNS_HOST> <BASE_DIR>\n");
        return -1;
    }

    char* dpns_host = argv[1];
    char* base_dir = argv[2];
    
    setenv ("DPNS_HOST", dpns_host, 1);
    reportHeader ("* Executing dpns_setacl tests...");

    // Test 1: Create a directory and reset the ACL with a new one
    strcpy (testdesc, "Create a directory and reset its ACL");
    sprintf (dirname, "%s/dir_setacl", base_dir);
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

    int i = -1;

    list[++i].a_type = CNS_ACL_USER_OBJ;
    list[i].a_id = 0;
    list[i].a_perm = 7;

    list[++i].a_type = CNS_ACL_GROUP_OBJ;
    list[i].a_id = 0;
    list[i].a_perm = 2;

    list[++i].a_type = CNS_ACL_OTHER;
    list[i].a_id = 0;
    list[i].a_perm = 1;

    list[++i].a_type = CNS_ACL_USER;
    list[i].a_id = 771;
    list[i].a_perm = 6;

    list[++i].a_type = CNS_ACL_GROUP;
    list[i].a_id = 772;
    list[i].a_perm = 6;

    list[++i].a_type = CNS_ACL_MASK;
    list[i].a_id = 0;
    list[i].a_perm = 4;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_USER_OBJ;
    list[i].a_id = 0;
    list[i].a_perm = 7;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_GROUP_OBJ;
    list[i].a_id = 0;
    list[i].a_perm = 1;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_OTHER;
    list[i].a_id = 0;
    list[i].a_perm = 1;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_USER;
    list[i].a_id = 881;
    list[i].a_perm = 7;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_GROUP;
    list[i].a_id = 882;
    list[i].a_perm = 4;

    list[++i].a_type = CNS_ACL_DEFAULT | CNS_ACL_MASK;
    list[i].a_id = 0;
    list[i].a_perm = 5;

    ret = dpns_setacl (dirname, i + 1, list);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "dpns_setacl call failed", sstrerror(serrno), 1);
        goto test1_end;
    }

    struct dpns_filestat fstat;
    ret = dpns_stat (dirname, &fstat);
    if ( ret != 0 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot stat dir_setacl", sstrerror(serrno), 1);
        goto test1_end;
    }

    if ( fstat.filemode != ( S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IXOTH ) )
    {
        error = 1;
        reportComponent (testdesc, "Filemode not correct", sstrerror(serrno), 1);
        goto test1_end;
    }

    ret = dpns_getacl (dirname, 128, list);
    if ( ret == -1 )
    {
        error = 1;
        reportComponent (testdesc, "Cannot read the ACL", sstrerror(serrno), 1);
        goto test1_end;
    }

    unsigned short bitmap = 0;
    for ( i = 0; i < ret; ++i )
    {
        switch ( list[i].a_type )
        {
            case CNS_ACL_USER_OBJ :

                if ( (bitmap & 0x0001) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_USER_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 7 )
                        bitmap = bitmap | 0x0001;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in CNS_ACL_USER_OBJ", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_USER :

                if ( (bitmap & 0x0002) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected CNS_ACL_USER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_id == 771 && list[i].a_perm == 6 )
                        bitmap = bitmap | 0x0002;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_id or a_perm in CNS_ACL_USER", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_GROUP_OBJ:

                if ( (bitmap & 0x0004) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_GROUP_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 2 )
                        bitmap = bitmap | 0x0004;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in CNS_ACL_GROUP_OBJ", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_GROUP :

                if ( (bitmap & 0x0008) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected CNS_ACL_GROUP in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_id == 772 && list[i].a_perm == 6 )
                        bitmap = bitmap | 0x0008;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_id or a_perm in CNS_ACL_GROUP", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_MASK :

                if ( (bitmap & 0x0010) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_MASK in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 4 )
                        bitmap = bitmap | 0x0010;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in CNS_ACL_MASK", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_OTHER :

                if ( (bitmap & 0x0020) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate CNS_ACL_OTHER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 1 )
                        bitmap = bitmap | 0x0020;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in CNS_ACL_OTHER", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_USER_OBJ :

                if ( (bitmap & 0x0040) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected DEFAULT CNS_ACL_USER_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 7 )
                        bitmap = bitmap | 0x0040;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in DEFAULT CNS_ACL_USER_OBJ", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_USER :

                if ( (bitmap & 0x0080) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected DEFAULT CNS_ACL_USER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_id == 881 && list[i].a_perm == 7 )
                        bitmap = bitmap | 0x0080;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_id or a_perm in DEFAULT CNS_ACL_USER", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_GROUP_OBJ :

                if ( (bitmap & 0x0100) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected DEFAULT CNS_ACL_GROUP_OBJ in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 1 )
                        bitmap = bitmap | 0x0100;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in DEFAULT CNS_ACL_GROUP_OBJ", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_GROUP :

                if ( (bitmap & 0x0200) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Unexpected DEFAULT CNS_ACL_GROUP in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_id == 882 && list[i].a_perm == 4 )
                        bitmap = bitmap | 0x0200;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_id or a_perm in DEFAULT CNS_ACL_GROUP", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_MASK :

                if ( (bitmap & 0x0400) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate DEFAULT CNS_ACL_MASK in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 5 )
                        bitmap = bitmap | 0x0400;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in DEFAULT CNS_ACL_MASK", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            case CNS_ACL_DEFAULT | CNS_ACL_OTHER :

                if ( (bitmap & 0x0800) == 1 )
                {
                    error = 1;
                    reportComponent (testdesc, "Dublicate DEFAULT CNS_ACL_OTHER in list", sstrerror(serrno), 1);
                    goto test1_end;
                }
                else
                {
                    if ( list[i].a_perm == 1 )
                        bitmap = bitmap | 0x0800;
                    else
                    {
                        error = 1;
                        reportComponent (testdesc, "Unexpected a_perm in DEFAULT CNS_ACL_OTHER", sstrerror(serrno), 1);
                        goto test1_end;
                    }
                }
                break;

            default :

                error = 1;
                reportComponent (testdesc, "Unknown ACE type found", "", 1);
                goto test1_end;
        }
    }

    if ( bitmap != 0x0FFF )
    {
        error = 1;
        sprintf (errorstr, "Obligatory entry is missing, bitmap is 0x%x", bitmap);
        reportComponent (testdesc, errorstr  , "", 1);
        goto test1_end;
    }

    reportComponent (testdesc, "ACL contents verified successfully", "", 0);

    test1_end:
        dpns_rmdir (dirname);

    reportFooter ("");
    reportOverall (error);

    return error;
}



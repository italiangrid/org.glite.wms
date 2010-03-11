#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpns_api.h>
#include <dirent.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <serrno.h>
#include <openssl/pem.h>

#include "utils.h"

//
// Retrive the certificate subject from a PEM file
//
int getSubject (char* filename, char* subject, int size)
{
    if ( filename == NULL )
        return -1;

    FILE * fp = fopen (filename, "r");
    if ( fp == NULL )
        return -1;

    X509 * cert;
    cert = PEM_read_X509 (fp, NULL, NULL, NULL);
    fclose (fp);
    if ( cert == NULL )
        return -1;

    X509_NAME_oneline (X509_get_subject_name (cert), subject, size);
    char* pindex = strstr (subject, "/CN=proxy");
    if ( pindex != NULL )
        *pindex = '\0';

    return 0;
}

//
// Used dpns_readdir to return the list of files
// in a directory including the deleted ones
//
dir_contents* getDirListFull (char* path)
{
    dpns_DIR * dir;
    struct dirent * dentry;

    if ( path == NULL || strlen (path) == 0 )
        return NULL;

    dir = dpns_opendir (path);
    if ( dir == NULL ) return NULL;

    dir_contents* dc = malloc (sizeof(dir_contents));
    dc->size = 0;

    serrno = 0;
    while ( dentry = dpns_readdir (dir) )
    {
        strcpy ((dc->list)[dc->size], dentry->d_name);

        if ( dc->size == 255 )
            break;
        else
            ++(dc->size);
    }
    if ( dentry == NULL && serrno != 0 )
    {
        free (dc);
        dpns_closedir (dir);
        return NULL;
    }
    dpns_closedir (dir);

    return dc;
}

//
// Used dpns_readdir to return the list of files
// in a directory without the deleted ones
//
dir_contents* getDirList (char* path)
{
    dpns_DIR * dir;
    struct dpns_direnstat * dsentry;

    if ( path == NULL || strlen (path) == 0 )
        return NULL;

    dir = dpns_opendir (path);
    if ( dir == NULL ) return NULL;

    dir_contents* dc = malloc (sizeof(dir_contents));
    dc->size = 0;

    serrno = 0;
    while ( dsentry = dpns_readdirx (dir) )
    {
        if ( dsentry->status != 'D' )
        {
            strcpy ((dc->list)[dc->size], dsentry->d_name);
            ++(dc->size);
        }
        if ( dc->size == 256 )
            break;
    }
    if ( dsentry == NULL && serrno != 0 )
    {
        free (dc);
        dpns_closedir (dir);
        return NULL;
    }
    dpns_closedir (dir);

    return dc;
}

//
// Populate a directory with n files and directories
// named file_n and dir_n
//
// Returns 0 on success, -1 on error
//
int populateDir (char* path, int n)
{
    int cnt, cnt2;
    int ret;
    int error = 0;

    char filename[CA_MAXPATHLEN + 1];

    for ( cnt = 0; cnt < n; ++cnt )
    {
        sprintf (filename, "%s/file_%d", path, cnt);
        if ( dpns_access (filename, F_OK) != 0 && serrno == ENOENT )
        {
           ret = dpns_creat (filename, 0664);
           if ( ret != 0 ) { error = 1; break; }
        }
        else
        {
            error = 1;
            break;
        }

        sprintf (filename, "%s/dir_%d", path, cnt);
        ret = dpns_mkdir (filename, 0775);
        if ( ret != 0 )
        {
            error = 1;
            sprintf (filename, "%s/file_%d", path, cnt);
            dpns_unlink (filename);
            break;
        }
    }
    if ( error == 1 )
    {
        for ( cnt2 = 0; cnt2 < cnt; ++cnt2 )
        {
            sprintf (filename, "%s/file_%d", path, cnt2);
            dpns_unlink (filename);
            sprintf (filename, "%s/dir_%d", path, cnt2);
            dpns_rmdir (filename);
        }
        return -1;
    }
    else
        return 0;
}

void cleanupDir (char* path, int n)
{
    int cnt;

    char filename[CA_MAXPATHLEN + 1];

    for ( cnt = 0; cnt < n; ++cnt )
    {
        sprintf (filename, "%s/file_%d", path, cnt);
        dpns_unlink (filename);
        sprintf (filename, "%s/dir_%d", path, cnt);
        dpns_rmdir (filename);
    }
}

int verifyContents (dir_contents* dc, int n)
{
    int i;
    char* number;
    int numval;
    int exponent;
    int matches = 0;
    char filemap[256];
    char dirmap[256];

    if ( dc->size != 2 * n || n < 0 || n > 255 )
        return 1;

    for ( i = 0; i < 256; ++i )
    {
        filemap[i] = 0;
        dirmap[i] = 0;
    }

    for ( i = 0; i < dc->size; ++i )
    {
        if ( strstr (dc->list[i], "file_") == dc->list[i] )
        {
            numval = 0;
            number = (dc->list[i]) + 5;
            exponent = exp10l (strlen (number) - 1);
            while ( *number != '\0' )
            {
                if ( *number < '0' || *number > '9' )
                {
                    numval = -1;
                    break;
                }
                else
                    numval += exponent * (*number - '0');
                number += 1;
                exponent /= 10;
            }
            if ( numval != -1 )
                filemap[numval] = 1;
            else 
                return 1;
        }
        if ( strstr (dc->list[i], "dir_") == dc->list[i] )
        {
            numval = 0;
            number = (dc->list[i]) + 4;
            exponent = exp10l (strlen (number) - 1);
            while ( *number != '\0' )
            {
                if ( *number < '0' || *number > '9' )
                {
                    numval = -1;
                    break;
                }
                else
                    numval += exponent * (*number - '0');
                number += 1;
                exponent /= 10;
            }
            if ( numval != -1 )
                dirmap[numval] = 1;
            else
                return 1;
        }
    }

    for ( i = 0; i < n; ++i )
        if ( filemap[i] == 0 || dirmap[i] == 0 )
            return 1;

    return 0;
}


void reportHeader (char *message)
{
    printf ("\n%s\n", message);
    printf ("--------------------------------------------------------------------------------\n");
}

void reportComponent (char* testdesc, char* message, char* serrnostr, int result)
{
    printf ("%s:%s:%s:%s\n", testdesc, message, serrnostr, result == 0 ? "SUCCESS" : "FAILURE");
}

void reportFooter (char* message)
{
    printf ("--------------------------------------------------------------------------------\n");
}

void reportOverall (int error)
{
    if ( error == 0 )
        printf ("Overall result:SUCCESS\n\n");
    else
        printf ("Overall result:FAILURE\n\n");
}


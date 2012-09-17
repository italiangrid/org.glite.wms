#include <serrno.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dpns_api.h"
#include "dpm_api.h"

char   * mypath;
char   * myfile;
char   * mynewfile;
char   * mylink;
char   * message;
char   * buffer;
char   * mychar;
char   * myserver;

struct stat * statbuf;
struct dpns_acl *acl;
struct dpns_filestat *dpnsstatbuf;
struct dpns_direnstat * mydestat;
dpns_DIR * mydp;

int    myflags;
int    ret;
int    globalret;
int    fd;

struct file_info {
        int   fd;
        char  *path;
        char  *storagepath;
        int   creopen;
        int   mode;
        int   count;
};


int report(char * message, int retval) {
 if (retval == 0 ) {
  printf("  %s          \t\t\t[ OK ]\n", message);
 } else {
  printf("  %s          \t\t\t[ Failed ]\n", message);
 }
}

int serrno_report(int serrorcode) {
    printf("       Ret. code: %d, message: %s\n", serrorcode, sstrerror(serrorcode));
}

int handle_result(int ret) {
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }
}

int main( int argc, char * argv[] ) {

 globalret = 0;
 mypath    = calloc(200,1);
 myfile    = calloc(200,1);
 mynewfile = calloc(200,1);
 mylink    = calloc(200,1);
 message   = calloc(200,1);
 buffer    = calloc(200,1);
 statbuf   = calloc(sizeof(stat),1);
 dpnsstatbuf   = calloc(500,1);
 acl       = calloc(sizeof(acl),200);
 mychar    = calloc(200,1);
 
  
 if ( argc > 1 ) {
  strcpy(mypath, argv[1]);
 } else {
  printf(" Usage: \%s <nodename> <testpath>\n", argv[0]);
  return 1;
 }

 sprintf(myfile,"%s/testfile",mypath);
 sprintf(mynewfile,"%s/testfile2",mypath);

 strcpy(message,"Starting session. \t\t[ dpns_startsess ]");

 myserver = malloc (200);
 strcpy (myserver, getenv("DPNS_HOST"));

 ret = dpns_startsess(myserver,"DPNS C API test");
 handle_result(ret);

 strcpy(message,"Creating a dir. \t\t[ dpns_mkdir     ]");
 ret = dpns_mkdir(mypath,0777);
 handle_result(ret);

 strcpy(message,"Creating a file. \t\t[ dpns_creat     ]");
 ret = dpns_creat(myfile,0777);
 handle_result(ret);

 strcpy(message,"Changing permission \t\t[ dpns_chmod     ]");
 ret = dpns_chmod(mypath,0777);
 handle_result(ret);

 strcpy(message,"Getting ACLs of dir. \t\t[ dpns_getacl    ]");
 ret = dpns_getacl(mypath,100,acl);
 handle_result(ret);

 strcpy(message,"Renaming files  \t\t[ dpns_rename    ]");
 ret = dpns_rename(myfile,mynewfile);
 handle_result(ret);

 strcpy(message,"Checking access  \t\t[ dpns_access    ]");
 ret = dpns_access(mynewfile,R_OK);
 handle_result(ret);

 strcpy(message,"Get information     \t\t[ dpns_stat      ]");
 ret = dpns_stat(mynewfile,dpnsstatbuf);
 handle_result(ret);

 strcpy(message,"Opening directory   \t\t[ dpns_opendir   ]");
 mydp = dpns_opendir(mypath);
 if ( mydp == NULL ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

 serrno = 0;
 strcpy(message,"Reading directory   \t\t[ dpns_readdir   ]");
 while ( (mydestat = dpns_readdirx(mydp)) != NULL) {
 }
 if ( serrno != 0 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }
 
 
 strcpy(message,"Closing directory  \t\t[ dpns_closedir  ]");
 ret = dpns_closedir(mydp);
 handle_result(ret);

 strcpy(message,"Removing file  \t\t[ dpns_unlink    ]");
 ret = dpns_unlink(mynewfile);
 handle_result(ret);

 strcpy(message,"Removing directory  \t\t[ dpns_rmdir     ]");
 ret = dpns_rmdir(mypath);
 handle_result(ret);

 strcpy(message,"Closing session   \t\t[ dpns_endsess   ]");
 ret = dpns_endsess();
 handle_result(ret);
 
 strcpy(message,"\n  Overall DPNS C API test result: \t\t ");
 if ( globalret == -1 ) {
    report(message, 1);
 } else {
    report(message,0);
 }

 printf("\n");
 return globalret;
}


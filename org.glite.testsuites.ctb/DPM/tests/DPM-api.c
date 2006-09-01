/*
 Test program to test DPM/DPNS C API
           
                      -- Gergely Debreczeni
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "dpns_api.h"
#include "dpm_api.h"
#include "serrno.h"

extern int serrno;
extern char *sstrerror(int serrno_value );
extern void sperror(char * message );


void samPrintOK(void) {
 printf("<font color='green'> - OK </font>\n");
}

void samPrintERROR(void) {
 printf("<font color='red'>ERROR: </font> ");
}

void samPrintWARNING(void) {
 printf("<font color='blue'> WARNING: </font> ");
}

/*

char * mkdirErrors(int err){

 char * message;
 switch(err) {
       ENOENT:  message=" A component of path prefix does not exist  or  path  is  a\
                    null pathname.";
                break;

       EACCES:  message=" Search  permission  is  denied  on a component of the path\
                    prefix or write permission  on  the  parent  directory  is\
                    denied.";
                break;

       EFAULT:  message="Path is a NULL pointer.";
                break;

       EEXIST:  message="Path exists already.";
                break;

       ENOTDIR: message="A component of path prefix is not a directory.";
                break;

       ENOSPC:  message="The name server database is full.";
                break;

       ENAMETOOLONG:  message="The  length of path exceeds CA_MAXPATHLEN or the length of\
                    a path component exceeds CA_MAXNAMELEN.";
                     break;
       SECOMERR:  message="Communication error.";
                  break;
       SENOSHOST: message="Host unknown.";
                break;

       SENOSSERV: message="Service unknown.";
                break;

       ENSNACT:   message="Name server is not running or is being shutdown.";
                break;
     }
}
*/

int main(void) {

// Common variables

 int myretval, myint ;
 int i,j,k;
 char *sameok = calloc(10,1);
 char *sameerror = calloc(10,1);
 char const * mysameokenv = "SAM_OK";
 char const * mysameerrenv = "SAM_ERROR";
 char *mypath = "/dpm";
 char * myerrormess = "";

// DPNS variables

 char const * mydpnshostenv = "DPNS_HOST";
 char *mydpnshost = calloc(20,1) ;
 struct dpns_filestat mystatbuf;
 struct dirent * mydirent = calloc(1000,1);
 dpns_DIR * mydir = calloc(20,1);

// DPM variables 
 char const * mydpmhostenv = "DPM_HOST";
 char *mydpmhost = calloc(20,1) ;
 struct dpm_pool *mypools;
 struct dpm_fs *myfss;

// Start calling the functions 

 printf("<b>Starting DPNS C-API test: </b> \n\n");

// Determining DPM host name

 printf("\n<ul><li>Determining DPNS host name. ");
 mydpmhost = getenv(mydpmhostenv);
 mydpnshost = getenv(mydpnshostenv);
 if ((sameok == NULL ) || (sameerror == NULL )) {
  samPrintERROR();
  printf(" Cannot determine DPNS host name from enviroment. Exiting.\n");
  exit(-1);
 } else  {
  samPrintOK();
  printf(" DPNS_HOST = %s\n",mydpnshost);
  printf(" DPM_HOST  = %s\n",mydpmhost);
 }

// Looking for path
 mypath = getenv("DPM_TEST_PATH");
 if (mypath == NULL ) {
  samPrintERROR();
  printf("Cannot determine testing path. Exiting.\n");
  exit(-1);
 }
 
 printf(" Test path used: %s",mypath);

// Deleting possible existing directory, do not care the errors  
 myretval = dpns_rmdir(mypath);

// Start testing different functions

 printf("\n<li>Creating a DPNS directory: %s ",mypath);
 myretval = dpns_mkdir(mypath,777);
 if (myretval == 0 ) {
  samPrintOK();
 } else {
  myerrormess=sstrerror(serrno);
  printf("\n\n");
  samPrintWARNING();
  printf(" Failed to create directory: %s \n Error code: %d\n Error message: %s\n",mypath,serrno,myerrormess); 
  printf(" API may works but no point on keep testing. Exiting.\n </ul>\n");
  exit(1);
 }

// Get info about the directory

 printf("\n<li>Getting info about the directory : %s ",mypath);
 myretval = dpns_stat(mypath, &mystatbuf);

 if (myretval == 0 ) {
  samPrintOK();
 } else {
  printf("\n\n");
  samPrintWARNING();
  printf("Failed to read directory information: %s \nError code: %d\nError message: %s\n",mypath,serrno,myerrormess);
  printf("API may works but no point on keep testing. Exiting.\n </ul>\n");
  exit(1);
 }

 printf(" The retrived info: \n");
 printf("       FileID   :       %d\n", mystatbuf.fileid); 
 printf("       Mode     :       %d\n", mystatbuf.filemode); 
 printf("       Nlink    :       %d\n", mystatbuf.nlink); 
 printf("       UID      :       %d\n", mystatbuf.uid); 
 printf("       GID      :       %d\n", mystatbuf.gid); 
 printf("       Filesize :       %d\n", mystatbuf.filesize); 
 printf("       ATime    :       %d\n", mystatbuf.atime); 
 printf("       MTime    :       %d\n", mystatbuf.mtime); 
 printf("       CTime    :       %d\n", mystatbuf.ctime); 
 printf("       Fileclass:       %d\n", mystatbuf.fileclass); 
 printf("       Status   :       %c\n", mystatbuf.status); 

 printf("\n <li>Removing directory: %s",mypath);
 myretval = dpns_rmdir(mypath);
 if (myretval == 0 ) {
  samPrintOK();
 } else {
  myerrormess=sstrerror(serrno);
  printf("\n\n");
  samPrintWARNING();
  printf(" Failed to remove directory: %s \n Error code: %d\n Error message: %s\n",mypath,serrno,myerrormess);
  printf(" API may works but no point on keep testing. Exiting.\n </ul>\n");
  exit(1);
 }

 printf("</ul>\n");
 printf("<b>Starting DPM C-API test: </b> \n\n");
 
 printf("\n<ul><li>Get pool names : %s ", mypath);
 myretval = dpm_getpools(&myint, &mypools);
 if (myretval == 0 ) {
  samPrintOK();
 } else {
  printf("\n\n");
  samPrintWARNING();
  printf(" Failed to get pool names: Error code: %d\n Error message: %s\n",serrno,myerrormess);
  printf(" API may works but no point on keep testing. Exiting.\n </ul>\n");
  exit(1);
}

 printf(" Pool information:\n");
 for (i = 0; i < myint; i++) {
   printf("       Pool name     : %s\n", (mypools + i)->poolname);
   printf("       Defsize       : %d\n", (mypools + i)->defsize);
   printf("       Capacity      : %d\n", (mypools + i)->capacity);
   printf("       Free space    : %d\n", (mypools + i)->free);
 }

 printf("</ul>\n");
 return(0);
}

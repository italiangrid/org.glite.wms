
#include "rfio_api.h"
#include "rfio_errno.h"
#include "rfio_constants.h"

#include <serrno.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char   * mypath;
char   * myfile;
char   * mynewfile;
char   * mylink;
char   * message;
char   * buffer;
char   * mycommand;
char   * mylines;

FILE   * pfile;

struct stat * statbuf;

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

int main( int argc, char * argv[] ) {

 globalret = 0;
 mypath    = calloc(200,1);
 myfile    = calloc(200,1);
 mynewfile = calloc(200,1);
 mylink    = calloc(200,1);
 message   = calloc(200,1);
 buffer    = calloc(200,1);
 statbuf   = calloc(sizeof(stat),1);
 mycommand = calloc(200,1);
 mylines   = calloc(2000,1);

 strcpy(mycommand,"lxb1921.cern.ch:/bin/ls");
  
 if ( argc > 1 ) {
  strcpy(mypath, argv[1]);
 } else {
  printf(" Usage: \%s <nodename> <testpath>\n", argv[0]);
  return 1;
 }

// rfio_popen should be disabled on a DPM

// rfio_popen

 strcpy(message,"Testing remote execution \t[ rfio_popen ]");
 pfile = rfio_popen(mycommand, "r");
 if ( pfile == NULL ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

// fgets(mylines,10,pfile);

// rfio_pclose
 strcpy(message,"Closing remote execution \t[ rfio_pclose ]");
 ret = rfio_pclose(pfile);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }


// rfio_mkdir

 strcpy(message,"Creating directory \t\t[ rfio_mkdir ]");
 ret = rfio_mkdir(mypath, 0777);
 report(message, ret);
 if ( ret !=0 ) {serrno_report(serrno);}
 if ( ret !=0 ) {globalret=-1;}

// rfio_open

 strcpy(message,"Creating a file \t\t[ rfio_open  ]");
 sprintf(myfile,"%s/testfile",mypath);

 fd = rfio_open(myfile, O_TRUNC | O_CREAT |  O_RDWR , 0777);
 if ( fd == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

// rfio_write

 sprintf(buffer,"This is an rfio API test file.\n");
 strcpy(message,"Writing to file \t\t[ rfio_write ]");
 ret = rfio_write(fd, buffer , 20);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

// rfio_lseek

 strcpy(message,"Seeking in the file \t\t[ rfio_lseek ]");
 ret = rfio_lseek(fd, 0, SEEK_SET );
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

// rfio_read

 strcpy(message,"Read from the file \t\t[ rfio_read  ]");
 strcpy(buffer,"\n");
 ret = rfio_read(fd, buffer, 20 );
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }
 

// rfio_close
  
 strcpy(message,"Closing the file \t\t[ rfio_close ]");
 ret = rfio_close(fd);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }
 
// rfio_rename

 sprintf(mynewfile,"%s/testfile2",mypath);
 strcpy(message,"Renaming the file \t\t[ rfio_rename ]");
 ret = rfio_rename(myfile,mynewfile);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }
 

// rfio_chmod 
 
 strcpy(message,"Changing permission \t\t[ rfio_chmod ]");
 ret = rfio_chmod(mynewfile, S_IRUSR | S_IWUSR);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

// rfio_access

 strcpy(message,"Checking access \t\t[ rfio_access ]");
 ret = rfio_access(mynewfile, R_OK);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }


// rfio_unlink

 strcpy(message,"Deleting the file \t\t[ rfio_unlink ]");
 ret = rfio_unlink(mynewfile);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }


// rfio_rmdir

 strcpy(message,"Deleting the dir. \t\t[ rfio_rmdir ]");
 ret = rfio_rmdir(mypath);
 if ( ret == -1 ) {
    report(message, 1);
    serrno_report(serrno);
    globalret=-1;
 } else {
    report(message,0);
 }

 
 strcpy(message,"\n  Overall RFIO C API test result: \t\t ");
 if ( globalret == -1 ) {
    report(message, 1);
 } else {
    report(message,0);
 }

 printf("\n");
 return globalret;
}


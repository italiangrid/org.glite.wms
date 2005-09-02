/*
	File:glite-wms-wmproxy-dirmanager.c
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/



#define DEFAULT_ADJUST_DIRECTORY_GROUP "edguser"
#define DEFAULT_ADJUST_DIRECTORY_MODE  0770

#ifdef ALLOW_COMMAND_LINE_OVERRIDE
#define ADJUST_DIRECTORY_USAGE "Usage:%s \n[-v] verbosity\n[-h] help\n[-c uid] [-g group] [-m mode] directory name, ...\n"
#define ADJUST_DIRECTORY_GETOPT_STRING "vhc:g:m:"
#else
#define ADJUST_DIRECTORY_USAGE "Usage:%s  \n[-v] verbosity\n[-h] help\n[-c uid] directory name, ...\n"
#define ADJUST_DIRECTORY_GETOPT_STRING "vhc:"
#endif

#include <stdio.h>

#include <string.h> // strdup
#include <stdlib.h> // exit
#include <errno.h> // errno
#include <grp.h> // getgrnam
#include <unistd.h> // getopt


#define ADJUST_DIRECTORY_ERR_NO_ERROR      0
#define ADJUST_DIRECTORY_ERR_OPTIONS       1
#define ADJUST_DIRECTORY_ERR_OUT_OF_MEMORY 2
#define ADJUST_DIRECTORY_ERR_NO_SUCH_GROUP 3
#define ADJUST_DIRECTORY_ERR_STAT          4
#define ADJUST_DIRECTORY_ERR_CHOWN         8
#define ADJUST_DIRECTORY_ERR_CHMOD         16
#define ADJUST_DIRECTORY_ERR_NOT_A_DIR     32
#define ADJUST_DIRECTORY_ERR_MKDIR         64


#include <sys/types.h>  // mode_t
#include <errno.h> // errno
#include <unistd.h> // rmdir, chown
#include <sys/stat.h> // stat


extern char *optarg;
extern int optind, opterr, optopt;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif







/************************************
*  check_dir
************************************/
int check_dir(char *dir, int opt_create, mode_t new_mode, gid_t new_group , uid_t create_uid ){
	/* Eventually, change the permissions of the named directories
	Continue on errors, and switch on the appropriate status bits. */
	// used to manage return values:
	int ret;
	struct stat stat_result;
	int summary_status = ADJUST_DIRECTORY_ERR_NO_ERROR ;

	// testing that is a directory
	ret = stat(dir, &stat_result);
	if ((ret < 0) && (errno == ENOENT) && opt_create){
		// Optionally create the directory in case it doesn't exist
		// Trying to create the directory
  		// printf("trying to create directory %s (uid: %d gid: %d)\n",dir, create_uid, new_group);
		if (mkdir(dir, new_mode) < 0){
			// fprintf(stderr,"Cannot create dir %s:%s\n",dir,strerror(errno));
			return ADJUST_DIRECTORY_ERR_MKDIR;
		}else{
				if (chown(dir,create_uid,new_group) < 0){
					// fprintf(stderr,"Cannot change owner of %s to %d:%s\n",dir,create_uid,strerror(errno));
					// fprintf(stderr,"Trying to remove %d\n",dir);
				if (rmdir(dir) < 0){
					// fprintf(stderr,"Cannot remove %s:%s\n",dir,strerror(errno));
				}
				return ADJUST_DIRECTORY_ERR_CHOWN;
				}
		}
		ret = stat(dir, &stat_result);
	}
	if (ret < 0){
		/* Stat of dir failed */
		// fprintf(stderr,"Cannot access %s:%s\n",dir,strerror(errno));
		return ADJUST_DIRECTORY_ERR_STAT;
	}
	if (S_ISDIR(stat_result.st_mode)) {
		ret = chown(dir, -1, new_group);
		if (ret < 0){
			// fprintf(stderr,"Cannot change group of %s to 0%o:%s\n",dir,new_group,strerror(errno));
			return ADJUST_DIRECTORY_ERR_CHOWN;
		}
		ret = chmod(dir, new_mode);
		if (ret < 0){
			// fprintf(stderr,"Cannot change mode of %s to 0%o:%s\n",dir,new_mode,strerror(errno));
			return ADJUST_DIRECTORY_ERR_CHMOD;
		}
	} else {
		/* Not a directory */
		// fprintf(stderr,"%s is not a directory. Skipping\n",dir);
		return ADJUST_DIRECTORY_ERR_NOT_A_DIR;
	}
	return summary_status;
}



int main (int argc, char *argv[]){
	int opt_verbose = FALSE;
	int opt_create  = FALSE;
	uid_t create_uid;
	char *new_group_s = DEFAULT_ADJUST_DIRECTORY_GROUP;
	gid_t new_group;
	mode_t new_mode   = DEFAULT_ADJUST_DIRECTORY_MODE;
	int summary_status;
	long int st_result;
	char *end_ptr;
	char c;
	int i;
	if (argc<2){
		fprintf(stderr,ADJUST_DIRECTORY_USAGE,argv[0]);
		exit(ADJUST_DIRECTORY_ERR_OPTIONS);
	}
	while ((c=getopt(argc,argv,ADJUST_DIRECTORY_GETOPT_STRING))!=EOF){
	switch (c){
		case 'h':
		printf(ADJUST_DIRECTORY_USAGE,argv[0]);
		exit(ADJUST_DIRECTORY_ERR_NO_ERROR);

		case 'v':
		opt_verbose = TRUE;
		break;

		case 'c':
		st_result = strtol(optarg, &end_ptr, 0);
		if (*end_ptr == '\0')
			{
			create_uid = st_result;
			opt_create = TRUE;
			}
		else
			{
			fprintf(stderr,ADJUST_DIRECTORY_USAGE,argv[0]);
			exit(ADJUST_DIRECTORY_ERR_OPTIONS);
			}
		break;
	#ifdef ALLOW_COMMAND_LINE_OVERRIDE
		case 'g':
		new_group_s = strdup(optarg);
		break;
		case 'm':
		st_result = strtol(optarg, &end_ptr, 0);
		if (*end_ptr == '\0')
			{
			new_mode = st_result;
			}
		else
			{
			fprintf(stderr,ADJUST_DIRECTORY_USAGE,argv[0]);
			exit(ADJUST_DIRECTORY_ERR_OPTIONS);
			}
		break;
	#endif /* defined ALLOW_COMMAND_LINE_OVERRIDE */
		default:
		fprintf(stderr,ADJUST_DIRECTORY_USAGE,argv[0]);
		exit(1);
		} // End of switch
	} // End of getopt() loop */

	st_result = strtol(new_group_s, &end_ptr, 0);
	if (*end_ptr == '\0'){
		new_group = st_result;
	}else{
		struct group *gr_result;
		gr_result = getgrnam(new_group_s);
		if (gr_result != NULL)
		{
		new_group = gr_result->gr_gid;
		}
		else
		{
		/* Error in gr_result */
		if (errno == ENOMEM)
		{
		fprintf(stderr,"%s: Out of Memory.\n",argv[0]);
		exit(ADJUST_DIRECTORY_ERR_OUT_OF_MEMORY);
		}
		else
		{
		fprintf(stderr,"%s: Cannot find any group named %s.\n",
			argv[0],new_group_s);
		exit(ADJUST_DIRECTORY_ERR_NO_SUCH_GROUP);
		}
		}
	}
	if (opt_verbose) {
		printf("%s: Changing directories to group %d and mode 0%o\n",argv[0],new_group, new_mode);
	}
	summary_status = ADJUST_DIRECTORY_ERR_NO_ERROR;

	for (i=optind; i<argc; i++){
		summary_status |= check_dir(argv[i], opt_create, new_mode, new_group , create_uid );
	}
	if (summary_status!=ADJUST_DIRECTORY_ERR_NO_ERROR) {
		printf("Warning!! some error occurred");
	}
	exit(summary_status);
}

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



#include "wmpdaemon.h"

extern char *optarg;
extern int optind, opterr, optopt;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

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
	if (opt_verbose) printf("%s: Changing directories to group %d and mode 0%o\n",argv[0],new_group, new_mode);
	summary_status = ADJUST_DIRECTORY_ERR_NO_ERROR;

	for (i=optind; i<argc; i++){
		summary_status |= check_dir(argv[i], opt_create, new_mode, new_group , create_uid );
	}
	exit(summary_status);
}

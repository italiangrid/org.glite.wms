/*
 * $Id$
 *
 * File: wmpdaemon.cpp
 *
 * Author(s):  Francesco Prelz <francesco.prelz@mi.infn.it >
                  Alessandro Maraschini <alessandro.maraschini@datamat.it>
 *
 * Description:
 *
 * This apis meant to be called while SUID root.
 *  check_dir method changes the group and mode of an (optionally created if -c is
 * specified) directory to a group and mode that can be built into
 * the program or specified on the command line
 * (in case ALLOW_COMMAND_LINE_OVERRIDE is defined at build time).
 * check_proxy  method scan the specified directory and delete all the expired proxy certificates
 * Revision history:
 *
 * 23-Mar-2004 Created
 */

#include "wmpdaemon.h"
#include <errno.h> // errno
#include <unistd.h> // rmdir, chown
#include <sys/stat.h> // stat

/************************************
*  check_proxy
************************************/
void check_proxy(char* directory){
	
}
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

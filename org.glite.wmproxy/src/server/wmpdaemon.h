/*
	File: wmpdaemon.h
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
#ifndef GLITE_WMS_WMPROXY_WMDAEMON_H
#define GLITE_WMS_WMPROXY_WMPDAEMON_H

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
/**
* Manage information for specified directory
* @param dir the directory to be checked
* @param opt_create whether the directory has to be created
* @param new_mode the new mode to be set for the directory
* @param new_group the new group to be set for the directory
* @param create_uid the new uid to be set for the directory
*/
int check_dir(char *dir, int opt_create, mode_t new_mode, gid_t new_group , uid_t create_uid );
/**
* Check the specified directory and delete expired proxies
*/
void check_proxy(char* directory);
#endif // GLITE_WMS_WMPROXY_WMPDAEMON_H



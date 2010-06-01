/**
 * @file corrupt.c
 * @author Alejandro Álvarez Ayllón, CERN
 * @version 0.1
 *
 * @section LICENSE
 *
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS
 * OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @section DESCRIPTION
 *
 * This small program receives a DPNS path of a file, and alter its checksum
 * so we "corrupt" the file. This is useful to force corrupted transmisions, for testing.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>
#include "dpns_api.h"
#include "serrno.h"

// Verbose flag
static int verbose_flag = 0;

/** 
 * Get the string associated with an error code
 *
 * @param code The error code
 * @return A pointer to the string
 */
const char *get_error_string(unsigned code)
{
  const char *error_msg;
	static char buffer[128];

  switch(serrno)
    {
			case ENOENT:
				error_msg = "File does not exist\n";
				break;
      case EINVAL:
        error_msg = "The length of comment is too long\n";
        break;
      case SENOSHOST:
        error_msg = "Host unknown\n";
        break;
      case SENOSSERV:
        error_msg = "Service unknown\n";
        break;
      case SECOMERR:
        error_msg = "Communication error\n";
        break;
      case ENSNACT:
        error_msg = "The name server is not running or is being shutdown\n";
        break;
      default:
				sprintf(buffer, "Unknown error code %i\n", serrno);
        error_msg = buffer;
    }
  return error_msg;
}

/**
 * Main function
 * @param argn The number of arguments
 * @param argv An array of strings containing the arguments
 * @return 0 if success, otherwise, an error code
 */
int main(int argn, char **argv)
{
  char *dpns_host = 0x00, *path = 0x00;
  char log_comment[300];

  // Get parameters
  int option_index = 0;
  static struct option long_options[] =
    {
      {"verbose", no_argument, &verbose_flag, 1},
      {"dpns",    required_argument, 0x00,    'd'},
			{"help",    no_argument, 0x00, 'h'},
      {0,0,0,0}
    };
  int c;

  while((c = getopt_long(argn, argv, "-vhd:", long_options, &option_index)) != -1)
  {
    switch(c)
    {
			case 'h':
				printf("Usage:");
				printf("\t%s [--verbose|-v] [(--dpns|-d) <DPNS host>] [--help|-h] <GUID>\n",
							 argv[0]);
				return 0;
				break;
      case 'v':
        verbose_flag = 1;
        break;
      case 'd':
        dpns_host = optarg;
        break;
      case 1:
				path = optarg;
				break;
      case 0: case '?':
        break;
      default:
        return -1;
    }
  }

  // The path is mandatory
  if(!path) {
    fprintf(stderr, "A file must be specified\n");
		return -1;
	}

  // If the DPNS is not set, try to get it from the environment
  dpns_host = getenv("DPNS_HOST");
  if(!dpns_host) {
    fprintf(stderr, "The DPNS host was not specified, and is not defined in the environment\n");
		return -1;
	}

  // VERBOSE
  if(verbose_flag)
    printf("DPNS Host:\t%s\nFile:\t\t%s\n",
           dpns_host, path);

  // Open session
  sprintf(log_comment, "Modifying file hash: %s", path);
  if(dpns_startsess(dpns_host, log_comment) != 0)
  {
    fprintf(stderr, "The session could not be started\n");
    fputs(get_error_string(serrno), stderr);
    return -2;
  }

  // Get file information
	struct dpns_filestatg file_stat;

	if(verbose_flag)
		printf("Recovering file information\n");

	if(dpns_statg(path, 0x00, &file_stat) != 0) {
		fprintf(stderr, "The file information could not be recovered\n");
		fputs(get_error_string(serrno), stderr);
		return -2;
	}

	if(verbose_flag) {
		char buffer[80];
		
		strftime(buffer, sizeof(buffer), "%H:%M:%S %m-%d-%Y", localtime(&file_stat.mtime));
	
		printf("File information:\n");
		printf("\tUnique ID:\t%i\n", file_stat.fileid);
		printf("\tGUID:\t%s\n",      file_stat.guid);
		printf("\tOwner ID:\t%i\n",  file_stat.uid);
		printf("\tModified:\t%s\n",  buffer);
		printf("\tChecksum Type:\t%s\n",  file_stat.csumtype);
		printf("\tChecksum Value:\t%s\n", file_stat.csumvalue);
	}

	// If there is no hash set, cancel
	if(file_stat.csumvalue[0] == '\0') {
		fprintf(stderr, "The checksum of the file is not set. Can not be corrupted\n");
		return -3;
	}

  // Alter hash (something simple is enough)
	if(file_stat.csumvalue[0] == '0')
		file_stat.csumvalue[0] = '1';
	else
		file_stat.csumvalue[0] = '0';

	if(verbose_flag)
		printf("The corrupted checksum will be %s\n", file_stat.csumvalue);

	// Update file information with corrupted hash
	if(dpns_setfsizec(path, 0x00, file_stat.filesize,
                file_stat.csumtype, file_stat.csumvalue) != 0) {
		fprintf(stderr, "The checksum could not be modified");
		fputs(get_error_string(serrno), stderr);
		return -2;
	}
	
	printf("Checksum modified!!\n");

  // End session
  if(dpns_endsess() != 0)
  {
    fprintf(stderr, "The session could not be properly closed\n");
    fputs(get_error_string(serrno), stderr);
    return -2;
  }
}


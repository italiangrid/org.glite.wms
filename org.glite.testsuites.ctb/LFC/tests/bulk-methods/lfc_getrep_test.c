#include <sys/types.h>
#define NSTYPE_LFC  1
#include "lfc_api.h"
#include <stdio.h>
#include "serrno.h"
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <sys/time.h>

/** helper_uuidgen : generate a UUID and return it as a string.  The
 * parameter must point to enough space to store a CA_MAXGUIDLEN long
 * string */
void helper_uuidgen(char *uuid_buf) {
  uuid_t uuid;
  uuid_generate(uuid);
  uuid_unparse(uuid, uuid_buf);
}
double my_clock2()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

/** helper_make_path : make a path below the test hierarchy. We assume
    path_buf is of size CA_MAXPATHLEN+1 */

void helper_make_path(char* path_buf, const char *subpath, int case_select) 
{
	
	char prefix_local_src[100]="file:/afs/cern.ch/project/gd/labadie/srmv2_test/src_files";
	char prefix_lfc_destin[100]="srm://lxb5409.cern.ch/dpm/cern.ch/home/dteam/generated/lfc_tests";
	char prefix_srm_destin[100]="srm://wn4.epcc.ed.ac.uk/dpm/epcc.ed.ac.uk/home/dteam/srmv2_tests";
	char prefix_lfn[100]="lfn:/grid/dteam/generated/bulk_tests";
        char prefix_lfn_lfc[100]="/grid/dteam/generated/bulk_tests";

	char prefix_local_destin[100]="file:/afs/cern.ch/project/gd/labadie/srmv2_test/dest_files";
        char prefix_srm_destin_dc[100]="srm://wn3.epcc.ed.ac.uk/pnfs/epcc.ed.ac.uk/data/dteam";
	char prefix_srm_destin_dc_nb[100]="srm://wn3.epcc.ed.ac.uk:8443/srm/managerv2?SFN=/pnfs/epcc.ed.ac.uk/data/dteam";
	char prefix_srm_destin_dpm_nb[100]="srm://wn4.epcc.ed.ac.uk:8446/srm/managerv2?SFN=/dpm/epcc.ed.ac.uk/home/dteam";

	switch(case_select)
	{
		case 1:// case it is a souce local file 
			sprintf(path_buf,"%s/%s",prefix_local_src,subpath);
			break;
		case 2: //case it a srm file
			sprintf(path_buf,"%s/%s",prefix_srm_destin,subpath);
			break;
		case 3: //case it is a lfn
			sprintf(path_buf,"%s/%s",prefix_lfn,subpath);
			break;
		case 4: //case it is a destin local file
			sprintf(path_buf,"%s/%s",prefix_local_destin,subpath);
			break;
		case 5: //case it is a destin local file
			sprintf(path_buf,"%s/%s",prefix_srm_destin_dc,subpath);
			break;
		case 6:
			sprintf(path_buf,"%s/%s",prefix_srm_destin_dpm_nb,subpath);
			break;
		case 7:
			sprintf(path_buf,"%s/%s",prefix_srm_destin_dc_nb,subpath);
			break;
		case 8:
			sprintf(path_buf,"%s/%s",prefix_lfc_destin,subpath);
			break;

		case 9:
			sprintf(path_buf,"%s/%s",prefix_lfn_lfc,subpath);
			break;

		default:
			strcpy(path_buf,"Error in creating the path");
	}

	
}

int main(int argc, char** argv)
{
  int rescode=0;
  int result=0;
char name_dest[30];
int i=0;
int flags;
int start_index;
int end_index;
 char guid_current[CA_MAXGUIDLEN+1];
 char path_dest[CA_MAXPATHLEN+1];
 char lfn_dest[CA_MAXPATHLEN+1];
 char filename [100];
struct timeval tv;
char lfc_host[100];
	struct timezone tz;
	struct tm *tm;
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
struct lfc_fileid file_uniqueid;
struct lfc_filestatg statbuf;
struct lfc_filereplica *lp;
char ** guid_list;
char ** path_list;

int j = 0;
lfc_list listp;
int nb_files_get =0;
int nb_to_get;
int test_type;
double start_time,end_time, elapsed_time;
int nbstatuses=0;
int * statuses;
int force = 0;
struct lfc_filereplicas *rep_entries;
 struct lfc_filereplica *rep_entries_tmp;
helper_uuidgen(guid_current);
sprintf(filename,"%s",guid_current);
fprintf(stdout,"Identifier of this session: log=%s \n",filename);


if(argv[1]!=NULL)
	start_index=atoi(argv[1]);
else
{
	fprintf(stdout,"Start index is missing \n");
	return -1;

}
if(argv[2]!=NULL)
	end_index=atoi(argv[2]);
else
{
	fprintf(stdout,"End index is missing \n");
	return -1;

}
if(argv[3]!=NULL)
        strcpy(lfc_host,argv[3]);
else
{
        fprintf(stdout,"The lfc host name is missing \n");
        return -1;

}

if(argv[4]!=NULL)
        test_type=atoi(argv[4]);
else
{
        fprintf(stdout,"Test type is missing \n");
        return -1;

}
guid_list=(char**)malloc((end_index-start_index)*sizeof(char*));

if(guid_list == NULL) {
	free(guid_list);
	fprintf(stderr,"Could not allocate memory \n");
	return -1;

}

for(i=start_index;i<end_index;i++) {
	guid_list[i-start_index]=(char*)malloc((CA_MAXGUIDLEN+1)*sizeof(char));

	if(guid_list[i-start_index]==NULL) {
		for(j=0;j<i;j++)
			free(guid_list[j]);
		free(guid_list);
		fprintf(stderr,"Could not allocate memory \n");
		return -1;
	}
		
}


fprintf(stdout,"Log generated at %02d/%02d/%02d  %d:%02d:%02d %d test type : %d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,test_type);// need to add the date
nb_to_get = end_index -start_index;

if (test_type != 3) {
	memset(&file_uniqueid, 0, sizeof(struct lfc_fileid));
	
        fprintf(stdout,"Allocating memory.\n");
	path_list=(char**)malloc((end_index-start_index)*sizeof(char*));
	if(path_list == NULL) {
		free(path_list);
		fprintf(stderr," - Could not allocate memory \n");
		for(j=0;j<end_index - start_index;j++)
			free(guid_list[j]);
		free(guid_list);
		return -1;
	}

	for(i=start_index;i<end_index;i++) {
		path_list[i-start_index]=(char*)malloc((CA_MAXPATHLEN+1)*sizeof(char));
		if(path_list[i-start_index]==NULL) {
			for(j=0;j<i;j++) 
				free(path_list[j]);
			free(path_list);
			for(j=0;j<end_index - start_index;j++)
				free(guid_list[j]);
			free(guid_list);
			fprintf(stderr," -Could not allocate memory \n");
			return -1;
		}
		sprintf(name_dest,"bulk_test_%02d",i);
		helper_make_path(lfn_dest, name_dest, 9);
		strcpy(path_list[i-start_index],lfn_dest);
	}


	rescode = lfc_getreplicasl(end_index - start_index, path_list, "", &nbstatuses, &rep_entries);
	j = 0;
	if (rescode == 0) {
		for (i = 0; i < nbstatuses; i++) {
			if (i == 0) 
				strcpy(guid_list [j], (rep_entries + i)->guid);
			else {
				j++;
				if (strncmp((rep_entries + i)->guid, guid_list [j], 36) && (rep_entries + i)->errcode == 0) {
					strcpy(guid_list [j], (rep_entries + i)->guid);
					
				}
			}
		}
	} else {
		fprintf(stderr,"lfc_getreplicasl : Could get the list of guids %s \n", sstrerror(serrno));
		return -1;
	}
	if (j != (end_index - start_index - 1) ) {
			for(j=0;j<end_index - start_index;j++) {
				free(guid_list[j]);
				free(path_list[j]);
			}
			free (guid_list);
			free (path_list);
			free (rep_entries);
			fprintf(stderr," Could not get the guids \n");
			return -1;
	}

}

gettimeofday(&tv, &tz);
tm=localtime(&tv.tv_sec);
fprintf(stdout,"At %02d/%02d/%02d  %d:%02d:%02d %d : starting using getreplicas files and start index=%d and end_index = %d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec, start_index, end_index);// need to add the date
start_time = my_clock2();

switch (test_type) {

	
	case 1:
		rescode=lfc_startsess (lfc_host,"Starting adding replicas"); 
		if(rescode!=0) {
			gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't start a session rescode=%d and serrno=%d and errmess=\"%s\" \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,serrno,sstrerror(serrno));// need to add the date
			for(j=0;j<end_index - start_index;j++)
			free(guid_list[j]);
			free(guid_list);
			return -1;
		}
		for(i=start_index;i<end_index;i++) {
			//sprintf(name_dest,"bulk_test_%02d",i);
			//helper_make_path(lfn_dest, name_dest,9);
			//helper_make_path(path_dest, name_dest,8);
			rescode= lfc_getreplica(NULL, guid_list[i-start_index], NULL, &nbstatuses, &rep_entries_tmp);
			if(rescode != 0) {
 				gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
				fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : Couldn't get replica for lfn=%s , rescode=%d , errmess=%s , index=%d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,guid_list[i-start_index],rescode,sstrerror(serrno), i);// need to add the date
			} else {
				nb_files_get = end_index - start_index;
			}
		}
		(void)lfc_endsess ();
		free (path_list);
		break;
	case 2:
		rescode = lfc_getreplicas (end_index-start_index, guid_list, "lxb5409.cern.ch", &nbstatuses, &rep_entries);
		if(rescode != 0) {
			gettimeofday(&tv, &tz);
        	tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't getreplicas for rescode=%d and  and nbstatuses=%d and serrno=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses,sstrerror(serrno));// need to add the dat
		} else {
			nb_files_get = end_index - start_index;
		}
		free (path_list);
		break;

	case 3:
		path_list=(char**)malloc((end_index-start_index)*sizeof(char*));
		if(path_list == NULL) {
			free(path_list);
			fprintf(stderr,"could not allocate memory \n");
			for(j=0;j<end_index - start_index;j++)
				free(guid_list[j]);
			free(guid_list);
			return -1;
		}
		for(i=start_index;i<end_index;i++) {
			path_list[i-start_index]=(char*)malloc((CA_MAXPATHLEN+1)*sizeof(char));
			if(path_list[i-start_index]==NULL) {
				for(j=0;j<i;j++)
					free(path_list[j]);
				free(path_list);
				fprintf(stderr,"could not allocate memory \n");
				return -1;
			}
			sprintf(name_dest,"bulk_test_%02d",i);
			helper_make_path(lfn_dest, name_dest,9);
			strcpy(path_list[i-start_index],lfn_dest);
			
		}
		rescode = lfc_getreplicasl(end_index - start_index, path_list, "lxb5409.cern.ch", &nbstatuses, &rep_entries);
		if(rescode != 0) {
			gettimeofday(&tv, &tz);
        	tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't lfc_getreplicasl for rescode=%d and  and nbstatuses=%d and serrno=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses,sstrerror(serrno));// need to add the dat
		} else {
			nb_files_get = end_index - start_index;
		}
		free(path_list);
		break;

	
	default:
		break;
}

end_time = my_clock2();
elapsed_time = end_time - start_time;
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : nb of files get %d and max files to get %d and time =%f \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,nb_files_get, nb_to_get, elapsed_time);// need to add the date
for(j=0;j<end_index - start_index;j++)
	free(guid_list[j]);
free(guid_list);

free(rep_entries);

return rescode;

}



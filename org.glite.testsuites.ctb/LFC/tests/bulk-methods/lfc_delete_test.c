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

char lfc_prefix[100];
char srm_prefix[100];

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
	char prefix_srm_destin[100]="srm://wn4.epcc.ed.ac.uk/dpm/epcc.ed.ac.uk/home/dteam/srmv2_tests";
	char prefix_lfn[100]="lfn:/grid/dteam/generated/srmv2_tests";
	char prefix_local_destin[100]="file:/afs/cern.ch/project/gd/labadie/srmv2_test/dest_files";
        char prefix_srm_destin_dc[100]="srm://wn3.epcc.ed.ac.uk/pnfs/epcc.ed.ac.uk/data/dteam";
	char prefix_srm_destin_dc_nb[100]="srm://wn3.epcc.ed.ac.uk:8443/srm/managerv2?SFN=/pnfs/epcc.ed.ac.uk/data/dteam";
	char prefix_srm_destin_dpm_nb[100]="srm://wn4.epcc.ed.ac.uk:8446/srm/managerv2?SFN=/dpm/epcc.ed.ac.uk/home/dteam";

        char prefix_lfn_lfc[100]="/grid/dteam/generated/lfc_bulk2/";
	char prefix_lfc_destin[100]="srm://lxb1917.cern.ch/dpm/cern.ch/home/dteam/generated/lfc_bulk2/";

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
			sprintf(path_buf,"%s/%s",srm_prefix,subpath);
			break;

		case 9:
			sprintf(path_buf,"%s/%s",lfc_prefix,subpath);
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
int nb_files_deleted =0;
int nb_to_delete;
int test_type;
double start_time,end_time, elapsed_time;
int nbstatuses=0;
int * statuses;
int force = 0;
struct lfc_filestatus *lfc_statuses_temp;

helper_uuidgen(guid_current);


if(argc != 7 ) {
 fprintf(stdout,"Number of arguments is wrong!\n");
 fprintf(stdout,"Usage: ./LFC_DEL_TEST lfc_prefix srm_prefix start_index end_index lfc_host_name test_num\n");
 return -1;
}

        strcpy(lfc_prefix,argv[1]);
        strcpy(srm_prefix,argv[2]);
	start_index=atoi(argv[3]);
	end_index=atoi(argv[4]);
        strcpy(lfc_host,argv[5]);
        test_type=atoi(argv[6]);

guid_list=(char**)malloc((end_index-start_index)*sizeof(char*));
if(guid_list == NULL) {
	free(guid_list);
	fprintf(stderr,"could not allocate memory \n");
	return -1;

}

for(i=start_index;i<end_index;i++) {
	guid_list[i-start_index]=(char*)malloc((CA_MAXGUIDLEN+1)*sizeof(char));
	if(guid_list[i-start_index]==NULL) {
		for(j=0;j<i;j++)
			free(guid_list[j]);
		free(guid_list);
		fprintf(stderr,"could not allocate memory \n");
		return -1;
	}
		
}
if (test_type != 5 && test_type != 4) {
memset(&file_uniqueid, 0, sizeof(struct lfc_fileid));
nb_to_delete = end_index -start_index;
fprintf(stdout,"Log file generated at %02d/%02d/%02d  %d:%02d:%02d %d test type : %d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,test_type);// need to add the date
rescode=lfc_startsess (lfc_host,"Starting adding replicas"); 
if(rescode!=0)
{
	fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't start a session rescode=%d and serrno=%d and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,result,serrno,sstrerror(serrno));// need to add the date
	for(j=0;j<end_index - start_index;j++)
		free(guid_list[j]);
	free(guid_list);
	return -1;

}

for(i=start_index;i<end_index;i++) {
	sprintf(name_dest,"bulk_test_%02d",i);
	helper_make_path(lfn_dest, name_dest,9);
	rescode=lfc_statg (lfn_dest, NULL, &statbuf); 
	if(rescode == 0)
		strcpy(guid_list[i-start_index], statbuf.guid);
	else {
		strcpy(guid_list[i-start_index], "none");
		nb_to_delete --;
 		gettimeofday(&tv, &tz);
        	tm=localtime(&tv.tv_sec);
		fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't get the guid for lfn=%s rescode=%d and  and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,lfn_dest,rescode,sstrerror(serrno));// need to add the date

	}
		
}
(void)lfc_endsess ();
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : starting deleting files \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec);// need to add the date
}
start_time = my_clock2();
switch (test_type) {

	case 1:
		for(i=start_index;i<end_index;i++) {
			sprintf(name_dest,"bulk_test_%02d",i);
			helper_make_path(lfn_dest, name_dest,9);
			helper_make_path(path_dest, name_dest,8);
//                        fprintf(stdout,"--- %s,%s,%s,",lfn_dest,path_dest,guid_list[i-start_index]);
			rescode=lfc_delreplica(guid_list[i-start_index], &file_uniqueid, path_dest); 
			if(rescode == 0) {
				rescode = lfc_unlink(lfn_dest);
				if (rescode == 0)
					nb_files_deleted ++;
				else {
					gettimeofday(&tv, &tz);
        				tm=localtime(&tv.tv_sec);
					fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delreplica for lfn=%s rescode=%d and  and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,lfn_dest,rescode,sstrerror(serrno));// need to add the date
				}
			} else {
	 			gettimeofday(&tv, &tz);
        			tm=localtime(&tv.tv_sec);
				fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't unlink for lfn=%s rescode=%d and  and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,lfn_dest,rescode,sstrerror(serrno));// need to add the date
			}
		}
		break;
	case 2:
		rescode=lfc_startsess (lfc_host,"Starting adding replicas"); 
		if(rescode!=0) {
			 gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't start a session rescode=%d and serrno=%d and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,serrno,sstrerror(serrno));// need to add the date
			for(j=0;j<end_index - start_index;j++)
			free(guid_list[j]);
			free(guid_list);
			return -1;
		}
		for(i=start_index;i<end_index;i++) {
			sprintf(name_dest,"bulk_test_%02d",i);
			helper_make_path(lfn_dest, name_dest,9);
			helper_make_path(path_dest, name_dest,8);
			rescode=lfc_delreplica (guid_list[i-start_index], &file_uniqueid, path_dest); 
			if(rescode == 0) {

				rescode = lfc_unlink(lfn_dest);
				if (rescode == 0)
					nb_files_deleted ++;
				else {
			 		gettimeofday(&tv, &tz);
        				tm=localtime(&tv.tv_sec);
					fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delreplica for lfn=%s rescode=%d and  and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,lfn_dest,rescode,sstrerror(serrno));// need to add the date
				}

			} else {
 				gettimeofday(&tv, &tz);
        			tm=localtime(&tv.tv_sec);
				fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't unlink for lfn=%s rescode=%d and  and errmess=%s \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,lfn_dest,rescode,sstrerror(serrno));// need to add the date
				}
		}
		(void)lfc_endsess ();
		break;
	case 3:
		rescode = Cns_delfilesbyguid(end_index - start_index, guid_list, 1, &nbstatuses, &statuses);
		if(rescode == 0)
			nb_files_deleted = end_index - start_index;
		else
		{
 			gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delfilesbyguid for rescode=%d and  and nbstatuses=%d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses);// need to add the dat

		}
		break;

	case 4:
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
		rescode = Cns_delfilesbyname(end_index - start_index, path_list, 1, &nbstatuses, &statuses);
		if(rescode == 0)
			nb_files_deleted = end_index - start_index;
		else
		{ 
			gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delfilesbyguid for rescode=%d and  and nbstatuses=%d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses);// need to add the dat

		}
		free(path_list);
		break;

	case 5:
		strcpy(name_dest,"bulk_test_aa%%");
		rescode = Cns_delfilesbypattern("/grid/dteam/generated/lfc_tests", name_dest,1, &nbstatuses, &lfc_statuses_temp);
		if(rescode == 0) {
			 gettimeofday(&tv, &tz);
        		 tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : delfilesbypattern for nbstatuses=%d  \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,nbstatuses);// need to add the dat
			for (i = 0; i < nbstatuses; i++) {
				if ((lfc_statuses_temp +i)->errcode == 0) 
					nb_files_deleted ++;
				else
				     fprintf (stdout, "error %s %d\n", (lfc_statuses_temp + i)->name, (lfc_statuses_temp + i)->errcode);
			}
		}
		else
		{
			 gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delfilesbypattern for rescode=%d and  and nbstatuses=%d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses);// need to add the dat

		}
		break;

	case 6:
		rescode = Cns_delreplicas(end_index - start_index, guid_list, "lxb5409.cern.ch", &nbstatuses, &lfc_statuses_temp);
		if(rescode == 0)
			nb_files_deleted = end_index - start_index;
		else
		{
 			gettimeofday(&tv, &tz);
        		tm=localtime(&tv.tv_sec);
			fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : couldn't delfilesbypattern for rescode=%d and  and nbstatuses=%d \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,rescode,nbstatuses);// need to add the dat

		}
		break;

	default:
		break;
}

end_time = my_clock2();
elapsed_time = end_time - start_time;
	gettimeofday(&tv, &tz);
	tm=localtime(&tv.tv_sec);
fprintf(stdout,"at %02d/%02d/%02d  %d:%02d:%02d %d : nb of files deleted %d and max files to delete %d and time =%f \n",tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100, tm->tm_hour, tm->tm_min,tm->tm_sec, tv.tv_usec,nb_files_deleted, nb_to_delete, elapsed_time);// need to add the date
for(j=0;j<end_index - start_index;j++)
	free(guid_list[j]);
free(guid_list);
return rescode;

}



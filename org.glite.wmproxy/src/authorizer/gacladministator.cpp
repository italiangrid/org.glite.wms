
#include "gacladministator.h"


using namespace std;

/*
	long options
*/
static const struct option long_options[] = {
    { "user",               		1,NULL,'u'},
    { "allow",                	1,NULL,'a'},
    { "deny",                	1,NULL,'d'},
     { "check",                 	1,NULL,'c'},
    { "rawname",                 	1,NULL,'n'},
    { "rawvalue",           	1,NULL,'v'},
    { "help",                 	0,NULL,'h'}
};
/*
	short options
*/
static const char* const short_options = "u:a:d:c:n:v:f:h";

/*
	usage message
*/
static void usage(const char *exe)
{
	fprintf(stderr,"%s: -a | -d | -c [-u] [-n -v] [-h] operation args \n",exe);
	fprintf(stderr, "-a | -allow <permission>\n");
	fprintf(stderr, "-d | -deny <permission>\n");
	fprintf(stderr, "-c | -check <permission>\n");
	fprintf(stderr, "-u | -user <username>	(default is any-user)\n");
	fprintf(stderr, "-n | -rawname <rawname>\n");
	fprintf(stderr, "-v | -rawname <rawvalue>\n");
	fprintf(stderr, "-h | -help (prints this message)\n");
	exit (-1);
}

int allow_permission(std::string file, std::string user,std::string permission,std::string rawname = "" ,  std::string rawvalue= "" ){

	int code,result = -1;
	GaclManager gacl (file);

	code = gacl.getPermissionCode( permission);

	if ( strcmp( (char*)user.c_str() , GACLMGR_ANYUSER) == 0 ){

		switch (code) {
			case GACLMGR_READ :{
				result = gacl.allowReadToAnyUser( );
				break;
			}
			case GACLMGR_EXEC :{
				//std::cout << "ADD_PERMISSION -5" << std::endl;
				result = gacl.allowExecToAnyUser( );
				break;
			}
			case GACLMGR_LIST	:{
				result = gacl.allowListToAnyUser( );
				break;
			}
			case GACLMGR_WRITE :{
				result = gacl.allowWriteToAnyUser( );
				break;
			}
			default :{
				break;
			}

		}

	} else {
		result = gacl.allowPermission (user, permission, rawname, rawvalue);
	}

	return result;

};

int deny_permission(std::string file, std::string user, std::string permission,std::string rawname = "" ,  std::string rawvalue= "" ){

	int code,result = -1;
	GaclManager gacl (file);

	code = gacl.getPermissionCode( permission);

	if ( strcmp((char*)user.c_str() , GACLMGR_ANYUSER) == 0 ){

		switch (code) {
			case GACLMGR_READ :{
				result = gacl.denyReadToAnyUser( );
				break;
			}
			case GACLMGR_EXEC :{
				result = gacl.denyExecToAnyUser( );
				break;
			}
			case GACLMGR_LIST	:{
				result = gacl.denyListToAnyUser( );
				break;
			}
			case GACLMGR_WRITE :{
				result = gacl.denyWriteToAnyUser( );
				break;
			}
			default :{
				break;
			}

		}

	} else {
		result = gacl.denyPermission (user, permission, rawname, rawvalue);
	}

	return result;

};

bool check_permission(std::string file, std::string user, std::string permission) {

	int code = -1;
	bool allow = false;
	GaclManager gacl (file,false);

	code = gacl.getPermissionCode( permission);
	//cout << "pemission=" << permission << "-code=" << code << "\n";
	if ( strcmp((char*)user.c_str() , GACLMGR_ANYUSER) == 0 ){

		switch (code) {
			case GACLMGR_READ :{
				allow = gacl.checkAllowReadForAnyUser( );
				break;
			}
			case GACLMGR_EXEC :{
				allow = gacl.checkAllowExecForAnyUser( );
				break;
			}
			case GACLMGR_LIST	:{
				allow = gacl.checkAllowListForAnyUser( );
				break;
			}
			case GACLMGR_WRITE :{
				allow = gacl.checkAllowWriteForAnyUser( );
				break;
			}
			default :{
				break;
			}

		}

	} else {
		allow = gacl.checkAllowPermission (user, permission);
	}

	return allow ;

};

/*
	main
*/
int main (int argc,char **argv)
{

	std::string user = "";
	std::string rawname = "";
	std::string rawvalue = "";
	std::string permission = "";
	std::string file = "";
	int operation = 0;
	int allow = -1 ;
	int next_option;
  	const char* program_name = NULL;
	bool a = false;
	bool d = false;
	bool c = false;
	bool u = false;
	bool n = false;
	bool v = false;
	bool f = false;

 	program_name = argv[0];


	// check options
	do {
		next_option = getopt_long(argc,
					argv,
					short_options,
					long_options,
					NULL);
		//cout << "next_option=" << next_option << endl;

		switch(next_option) {
			case 'u': {
				user = optarg;
				u = true;
				//cout <<"user=" << user<< "\n";
				break;
			}
			case 'a': {
				permission = optarg ;

				a=true;
				if (d==true| c== true){
					fprintf (stderr, "-allow | -deny | -check can not be used at the same time\n");
					usage (program_name);
				}
				operation = ALLOW_PERMISSION ;
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			case 'd': {
				permission = optarg ;
				d =true;
				if (a==true || c== true){
					fprintf (stderr,"-allow | -deny | -check can not be used at the same time\n");
					usage (program_name);
				}
				operation = DENY_PERMISSION ;
				//cout <<"deny permission=" << permission << "\n";
				break;
			}

			case 'c': {
				permission = optarg ;
				cout <<"check  permission=" << permission << "\n";
				c =true;
				if (a==true || d==true){
					fprintf (stderr,"-allow | -deny | -check can not be used at the same time\n");
					usage (program_name);
				}
				operation = CHECK_PERMISSION ;
				//cout <<"check  permission=" << permission << "\n";
				break;

			}
			case 'n': {
				rawname = optarg ;
				//cout <<"rawname=" << rawname << "\n";
				break;
			}

			case 'v': {
				rawvalue = optarg ;
				//cout <<"rawvalue=" << rawvalue << "\n";
				break;
			}
			case 'f': {
				file = optarg ;
				f = true;
				//cout <<"file=" << file << "\n";
				break;
			}

			case -1:{
				break;
			}

			default:
				usage (program_name);
			break;
		}
	} while (next_option != -1);


	// sets the user default if  it is not set
	if (u==false) {
		user = GACLMGR_ANYUSER;
	}

	// sets the file default if it is not set
	if (f==false){
		file = DEFAULT_GACL_FILE;
	}

	// checks raw. name and value
	if ( n== true || v== true ){
		if ( !( n==true && v == true && u == true) ){
			fprintf (stderr, "missing parameter: -user -rawname -rawvalue\n");
			usage (program_name);
		}

	}

	// if no operation was set
	if ( !a && !d && !c ){
		fprintf (stderr, "no operation set: -allow | -deny | -check ");
		usage (program_name);
	}


	try {
		switch (operation){

			case ALLOW_PERMISSION :{
				allow_permission(file, user,permission, rawname, rawvalue) ;
				break;
			}
			case DENY_PERMISSION :{
				deny_permission(file, user,permission, rawname, rawvalue) ;
				break;
			}

			case CHECK_PERMISSION :{
				allow = check_permission(file, user, permission) ;
					//cout << "allow = " << allow << endl;
				break;
			}
			default :{
				fprintf (stderr, "no operation set: -allow | -deny | -check\n ");
				usage (program_name);
				break;
			}


		}
	} catch( GaclException &exc){
		cout << flush << exc.what ( ) << endl ;
	}

	return 0;
}

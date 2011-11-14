/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

#include <iostream>
#include <vector>
#include "security/wmpgaclmanager.h"


extern "C" {
#include <getopt.h>
}

using namespace std;
namespace authorizer 	 = glite::wms::wmproxy::authorizer;

/*
	short option constants
*/

// short option: file
static const char SHORTOPT_FILE = 'f';
// short option: add
static const char SHORTOPT_ADD = 'z';
// short options: allow
static const char SHORTOPT_ALLOW_READ = 'R' ;
static const char SHORTOPT_ALLOW_LIST = 'T';
static const char SHORTOPT_ALLOW_WRITE = 'W';
static const char SHORTOPT_ALLOW_EXEC = 'E';
// short options: deny
static const char SHORTOPT_DENY_READ = 'r';
static const char SHORTOPT_DENY_LIST = 't';
static const char SHORTOPT_DENY_WRITE = 'w';
static const char SHORTOPT_DENY_EXEC = 'e';
// short options: check
static const char SHORTOPT_CHECK_READ = 'a';
static const char SHORTOPT_CHECK_LIST = 'b';
static const char SHORTOPT_CHECK_WRITE = 'c';
static const char SHORTOPT_CHECK_EXEC = 'd';
// short options: delete
static const char SHORTOPT_DELETE = 'D';
// short options: type of credential
static const char SHORTOPT_ANY = 'A';
static const char SHORTOPT_PERSON = 'P';
static const char SHORTOPT_DNL = 'L';
static const char SHORTOPT_VOMS = 'V';
//short options: drain
static const char SHORTOPT_EN_DRAIN = 'N';
static const char SHORTOPT_DIS_DRAIN = 'S';
//short options: new file
static const char SHORTOPT_NEW = 'C';
//short options: help
static const char SHORTOPT_HELP = 'H';


/*
	short options
*/
static const char* short_options = "f:RTWErtweabcdDAP:L:V:NSCH" ;

/*
	long options
*/
static const struct option long_options[] = {
    { "file",                			1,NULL, SHORTOPT_FILE },
    { "add",                			0,NULL, SHORTOPT_ADD },
    { "allow-read",                	0,NULL, SHORTOPT_ALLOW_READ },
    { "allow-list",                	0,NULL, SHORTOPT_ALLOW_LIST },
    { "allow-write",                	0,NULL, SHORTOPT_ALLOW_WRITE },
    { "allow-exec",                	0,NULL, SHORTOPT_ALLOW_EXEC },
    { "deny-read",                	0,NULL, SHORTOPT_DENY_READ },
    { "deny-list",                	0,NULL, SHORTOPT_DENY_LIST },
    { "deny-write",                	0,NULL, SHORTOPT_DENY_WRITE},
    { "deny-exec",                	0,NULL, SHORTOPT_DENY_EXEC},
    { "check-read",                	0,NULL, SHORTOPT_CHECK_READ},
    { "check-list",                	0,NULL, SHORTOPT_CHECK_READ},
    { "check-write",                	0,NULL, SHORTOPT_CHECK_WRITE },
    { "check-exec",                	0,NULL, SHORTOPT_CHECK_EXEC},
    { "delete",                		0,NULL, SHORTOPT_DELETE},
    { "person",                 		1,NULL, SHORTOPT_PERSON },
    { "dnl",                 			1,NULL, SHORTOPT_DNL},
    { "any",           			0,NULL, SHORTOPT_ANY},
    { "voms",           			1,NULL, SHORTOPT_VOMS},
    { "enable-drain",           	0,NULL, SHORTOPT_EN_DRAIN },
    { "disable-drain",           	0,NULL, SHORTOPT_DIS_DRAIN},
    { "new",           			0,NULL, SHORTOPT_NEW},
    { "help",                 		0,NULL, SHORTOPT_HELP},
    {0, 0, 0, 0}
};
/*
	code operations
*/
static const int	 NO_OPER = 0 ;
static const int	 ALLOW_PERMISSION 	= 1;
static const int DENY_PERMISSION = 2;
static const int CHECK_PERMISSION = 3;
static const int ADD = 4;
static const int DELETE = 5;
static const int DRAIN_ON = 6;
static const int DRAIN_OFF = 7;

static void ending (int code){
	exit (code);
};

static void for_usage (const char *exe) {
	fprintf(stderr,"\nusage:\n");
	fprintf(stderr,"%s <cred-type>  <operation> [other options]\n\n",exe);
	fprintf(stderr,"--help  to see the complete usage message\n\n");
	fprintf(stderr,"Gacl administrator tool\n");
	fprintf(stderr,"Copyright (C) 2005 by DATAMAT SpA\n\n");
	fprintf(stderr, "Please report any bug at:\n\t\tegee@datamat.it\n\n");
	ending (0);
};

/*
	usage message
*/
static void usage(const char *exe) {
	fprintf(stderr,"\nusage:\n");
	fprintf(stderr,"%s <cred-type>  <operation> [other options]\n",exe);
	fprintf(stderr,"where\n\n");
	// CREDENTIAL
	// =============================
	fprintf(stderr,"users credential type options (mandatory): \n");
	fprintf (stderr, "\twarning: use double quotes if the arguments of the following group of options contains blank spaces\n");
	//person
	fprintf (stderr, "\t(eg. --person \"/C=IT/O=XYZ/OU=Personal Certificate\" \n\n");
	fprintf(stderr, "\t--person \"<person-DN>\"\tuser's certificate DN\n\n");
	// dnl
	fprintf(stderr, "\t--dnl \"<DN-list URL>\"\tfor groups of people using plain text DN Lists,\n");
	fprintf(stderr, "\t\t\t\tthat is, lists of people's certificate DNsDN List has a URL which uniquely identifies the list\n");
	fprintf(stderr, "\t\t\t\te.g. \"https://www.gridpp.ac.uk/dn-lists/gridpp\" )\n\n");
	// voms
	fprintf(stderr, "\t--voms <voms-DN>\tuser's VOMS certificate fqan\n\n");
	// any-user
	fprintf(stderr, "\t--any\t\t\tany user\n\n");
	// OPERATIONS
	fprintf(stderr,"operation (mandatory): \n");
	fprintf(stderr,"\tit is possible:\n\t\t- to enable or to disable \"drain\";\n\t\t- or to specify a group of \"allow\" and \"deny\" permission\n");
	fprintf(stderr,"\t\t- or to specify a group of permission which has to be checked\n");
	fprintf(stderr,"\t\t- or to remove a credential entry\n\n");
	fprintf(stderr, "\t--enable-drain\tdenies exec permission to any user\n");
	fprintf(stderr, "\t--disable-drain\tundenies exec permission to any user(if it is denied)\n\n");
	fprintf(stderr, "\t--allow-read\tallows read permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--allow-write\tallows write permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--allow-list\tallows list permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--allow-exec\tallows exec permission to user(s) specified by credential option\n\n");
	fprintf(stderr, "\t--deny-read\tdenies read permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--deny-write\tdenies write permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--deny-list\tdenies list permission to user(s) specified by credential option\n");
	fprintf(stderr, "\t--deny-exec\tdenies exec permission operation to user(s) specified by credential option\n\n");
	fprintf(stderr, "\t--check-read\tchecks if  user(s) specified by credential option has(have) read permission\n");
	fprintf(stderr, "\t--check-write\tchecks if  user(s) specified by credential option has(have) write permission\n");
	fprintf(stderr, "\t--check-list\tchecks if  user(s) specified by credential option has(have) list permission\n");
	fprintf(stderr, "\t--check-exec\tchecks if  user(s) specified by credential option has(have) exec permission\n\n");
	fprintf(stderr, "\t--delete\tremoves the entry related to user(s) specified by credential option\n\n");
	// other options
	fprintf(stderr,"not mandatory options: \n");
	fprintf(stderr, "\t--file <path>\tnon-standard location of gacl file (standard is  ./.gacl )\n\n");
	fprintf(stderr, "\t--new\t\tcreates a new file if it doesn't exist ; otherwise replaces the old one\n\n");
	fprintf(stderr, "\t--help\t\tdisplays this message\n\n");
	fprintf(stderr,"Gacl administrator tool\n");
	fprintf(stderr,"Copyright (C) 2005 by DATAMAT SpA\n\n");
	fprintf(stderr, "Please report any bug at:\n\t\tegee@datamat.it\n\n");

	ending (0);
}
/*
	utilty method: contains
		checks if the vector (vect) contain
		the element (elem)
*/
bool contains( vector<int> vect, int elem) {

	bool contained=false;
	vector<int>::iterator it ;
	for (it = vect.begin(); it!=vect.end(); ++it) {
		if  ( *it == elem ){
			contained = true;
			break;
		}
	}

	return contained;
}


/*
	main
*/
int main (int argc,char **argv)
{
	string user = "";
	string file = "";
	int allow = -1 ;
	int next_option;
	int size = 0;
  	const char* prg_name = NULL;

	vector<unsigned int> checks ;
	string fmsg = "";
	// operations
	vector<int> operations;
	vector<int>::iterator it2 ;
	// flags: allowed permission
	bool ar = false;
	bool aw = false;
	bool al = false;
	bool ae= false;
	// flags: denied permission
	bool dr = false;
	bool dw = false;
	bool dl = false;
	bool de= false;
	// flags: operations (allow, deny, check)
	bool a = false;
	bool d = false;
	bool c = false;
	// flags: delete operation
	bool del = false;
	// flags: add operation
	bool add = false;
	// flags: drainoperation
	bool drain = false ;
	// flags: path location
	bool f = false;
	// flags: creates new file if gacl exists
	bool new_file = false;
	// init of permission and credtype
	authorizer::WMPgaclPerm allow_perm = authorizer::GaclManager::WMPGACL_NOPERM;
	authorizer::WMPgaclPerm deny_perm = authorizer::GaclManager::WMPGACL_NOPERM;
	authorizer::GaclManager::WMPgaclCredType cred_type = authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE ;
	// vector of permission type for the various operations
	vector<string> allow_vect;
	vector<string> deny_vect;
	vector<string> check_vect;

	// program name
 	prg_name = argv[0];

	// check options
	do {
		next_option = getopt_long_only(argc,
					argv,
					short_options,
					long_options,
					NULL);
		//cout << "option=" << next_option << endl;
		switch(next_option) {
			// file
			case SHORTOPT_FILE: {
				file = optarg;
				f = true;
				break;
			}
			// allow-read
			case SHORTOPT_ALLOW_READ: {
				if ( dr || c || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--allow-read cannot be used together the following options:\n");
					fprintf (stderr, "--deny-read ; --check-[xxx] ; --enable-drain ;  --disable-drain\n\n");
					ending(-1);
				}
				// allow flags
				ar=true;
				a = true;
				// operation type
				if ( ! contains(operations,ALLOW_PERMISSION ) ){
					operations.push_back(ALLOW_PERMISSION );
				}
				allow_perm = allow_perm | authorizer::GaclManager::WMPGACL_READ ;
				allow_vect.push_back("read");
				//cout << "allow read" << "\n";
				break;
			}
			// allow-list
			case SHORTOPT_ALLOW_LIST: {
				if ( dl || c  || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--allow-list cannot be used together the following options:\n");
					fprintf (stderr, "--deny-list ; --check-[xxx] ; --enable-drain ; --disable-drain\n\n");
				}
				// allow flags
				al=true;
				a = true;
				// operation type
				if ( ! contains(operations,ALLOW_PERMISSION ) ){
					operations.push_back(ALLOW_PERMISSION );
				}
				allow_perm = allow_perm | authorizer::GaclManager::WMPGACL_LIST;
				allow_vect.push_back("list");
				//cout <<"deny permission=" << permission << "\n";
				break;
			}
			// allow-write
			case SHORTOPT_ALLOW_WRITE: {
				if ( dw || c  || drain ) {
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--allow-write cannot be used together the following options:\n");
					fprintf (stderr, "--deny-write ; --check-[xxx] ; --enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// allow flags
				aw=true;
				a = true;
				// operation type
				if ( ! contains(operations,ALLOW_PERMISSION ) ){
					operations.push_back(ALLOW_PERMISSION );
				}
				allow_perm = allow_perm | authorizer::GaclManager::WMPGACL_WRITE;
				allow_vect.push_back("write");
				break;
			}
			// allow-exec
			case SHORTOPT_ALLOW_EXEC: {
				if ( de || c  || drain ) {
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--allow-exec cannot be used together the following options:\n");
					fprintf (stderr, "--deny-exec ; --check-[xxx] ; --enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// allow flags
				ae=true;
				a = true;
				// operation type
				if ( ! contains(operations,ALLOW_PERMISSION ) ){
					operations.push_back(ALLOW_PERMISSION );
				}
				allow_perm = allow_perm | authorizer::GaclManager::WMPGACL_EXEC;
				allow_vect.push_back("exec");
				//cout <<"deny permission=" << permission << "\n";
				break;
			}
			// deny-read
			case SHORTOPT_DENY_READ: {
				if ( ar || c || drain   ) {
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--deny-read cannot be used together the following options:\n");
					fprintf (stderr, "--allow-read ; --check-[xxx] ; --enable-drain ;  --disable-drain\n\n");
					ending(-1 );
				}
				// deny flags
				dr=true;
				d = true;
				// operation type
				if ( ! contains(operations,DENY_PERMISSION ) ){
					operations.push_back(DENY_PERMISSION );
				}
				deny_perm = deny_perm | authorizer::GaclManager::WMPGACL_READ ;
				deny_vect.push_back("read");
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// deny-list
			case SHORTOPT_DENY_LIST: {
				if ( dl || c  || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--deny-list cannot be used together the following options:\n");
					fprintf (stderr, "--allow-list ; --check-[xxx] ; --enable-drain ;  --disable-drain\n\n");
					ending(-1 );
				}
				// deny flags
				de=true;
				d = true;
				// operation type
				if ( ! contains(operations,DENY_PERMISSION ) ){
					operations.push_back(DENY_PERMISSION );
				}
				deny_perm = deny_perm | authorizer::GaclManager::WMPGACL_LIST;
				deny_vect.push_back("list");
				//cout <<"deny permission=" << permission << "\n";
				break;
			}
			// deny-write
			case SHORTOPT_DENY_WRITE: {
				if ( aw || c  || drain ) {
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--deny-write cannot be used together the following options:\n");
					fprintf (stderr, "--allow-write ; --check-[xxx] ; --enable-drain ;  --disable-drain\n\n");
					ending(-1 );
				}
				// deny flags
				dw=true;
				d = true;
				// operation type
				if ( ! contains(operations,DENY_PERMISSION ) ){
					operations.push_back(DENY_PERMISSION );
				}
				deny_perm = deny_perm | authorizer::GaclManager::WMPGACL_WRITE;
				deny_vect.push_back("write");
				//cout <<"deny permission=" << permission << "\n";
				break;
			}
			// deny-exec
			case SHORTOPT_DENY_EXEC: {
				if ( ae || c || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--deny-exec cannot be used together the following options:\n");
					fprintf (stderr, "--allow-exec ; --check-[xxx] ; --enable-drain ;  --disable-drain\n\n");
					ending(-1 );
				}
				// deny flags
				de=true;
				d = true;
				// operation type
				if ( ! contains(operations,DENY_PERMISSION ) ){
					operations.push_back(DENY_PERMISSION );
				}
				deny_perm = deny_perm | authorizer::GaclManager::WMPGACL_EXEC;
				deny_vect.push_back("exec");
				//cout <<"deny permission=" << permission << "\n";
				break;
			}
			// check-read
			case SHORTOPT_CHECK_READ: {
				//checks if any allow and deny operations have been already requested
				if ( a || d || drain  ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--check-[xxx] options cannot be used together the following other options:\n");
					fprintf (stderr, "--allow-[xxx] ; --deny-[xxx] ; -enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// "CHECK" flag
				c=true;
				// operation vector
				if ( ! contains(operations, CHECK_PERMISSION ) ){
					operations.push_back(CHECK_PERMISSION );
				}
				// permission to be checked
				checks.push_back( authorizer::GaclManager::WMPGACL_READ ) ;
				check_vect.push_back("read");
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// check-list
			case SHORTOPT_CHECK_LIST: {
				//checks if any allow and deny operations have been already requested
				if ( a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--check-[xxx] options cannot be used together the following other options:\n");
					fprintf (stderr, "--allow-[xxx] ; --deny-[xxx] ; -enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// "CHECK" flag
				c=true;
				// operation vector
				if ( ! contains(operations, CHECK_PERMISSION ) ){
					operations.push_back(CHECK_PERMISSION );
				}
				checks.push_back( authorizer::GaclManager::WMPGACL_LIST ) ;
				check_vect.push_back("list");
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// check-write
			case SHORTOPT_CHECK_WRITE : {
				//checks if any allow and deny operations have been already requested
				if ( a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--check-[xxx] options cannot be used together the following other options:\n");
					fprintf (stderr, "--allow-[xxx] ; --deny-[xxx] ; -enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// "CHECK" flag
				c=true;
				// operation vector
				if ( ! contains(operations, CHECK_PERMISSION ) ){
					operations.push_back(CHECK_PERMISSION );
				}
				// permission to be checked
				checks.push_back( authorizer::GaclManager::WMPGACL_WRITE) ;
				check_vect.push_back("write");
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// check-exec
			case SHORTOPT_CHECK_EXEC : {
				//checks if any allow and deny operations have been already requested
				if ( a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--check-[xxx] options cannot be used together the following other options:\n");
					fprintf (stderr, "--allow-[xxx] ; --deny-[xxx] ; -enable-drain ; --disable-drain\n\n");
					ending(-1 );
				}
				// "CHECK" flag
				c=true;
				// operation vector
				if ( ! contains(operations, CHECK_PERMISSION ) ){
					operations.push_back(CHECK_PERMISSION );
				}
				// permission to be checked
				checks.push_back( authorizer::GaclManager::WMPGACL_EXEC) ;
				check_vect.push_back("exec");
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// delete
			case SHORTOPT_DELETE : {
				//checks if any check and allow and deny operations have been already requested
				if (c || a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--delete is incompatible with the other operations (allow, deny, check, drain)\n\n");
					ending(-1 );
				}
				// DELETE flag
				del=true;
				// operation vector
				if ( ! contains(operations, DELETE ) ){
					operations.push_back(DELETE );
				}
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// delete
			case SHORTOPT_ADD : {
				//checks if any check and allow and deny operations have been already requested
				if (c || del ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--add is incompatible other operations like check and delete\n\n");
					ending(-1 );
				}
				// DELETE flag
				add=true;
				// operation vector
				if ( ! contains(operations, ADD ) ){
					operations.push_back(ADD);
				}
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// person entry
			case SHORTOPT_PERSON : {
				// checks if another credential type option has been already set
				if ( cred_type != authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE){
					fprintf (stderr, "\nError: multiple credential type as input\n");
					fprintf (stderr, "Specify only one type of credential: any-user | person | dnlist | voms\n\n");
				}
				// userDN
				user = optarg ;
				// credential type
				cred_type = authorizer::GaclManager::WMPGACL_PERSON_TYPE;
				//cout <<"person=[" << user<< "]\n";
				break;
			}
			// voms entry
			case SHORTOPT_VOMS: {
				// checks if another credential type option has been already set
				if ( cred_type != authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE){
					fprintf (stderr, "\nError: multiple credential type as input.\n");
					fprintf (stderr, "Specify only one type of credential: any-user | person | dnlist | voms\n\n");
				}
				// user VOMS fqan
				user = optarg ;
				// credential type
				cred_type = authorizer::GaclManager::WMPGACL_VOMS_TYPE;
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// dn-list entry
			case SHORTOPT_DNL: {
				// checks if another credential type option has been already set
				if ( cred_type != authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE){
					fprintf (stderr, "\nError: multiple credential type as input\n");
					fprintf (stderr, "Specify only one type of credential: any-user | person | dnlist | voms\n\n");
				}
				// dnl list URL
				user = optarg ;
				// credential type
				cred_type = authorizer::GaclManager::WMPGACL_DNLIST_TYPE;
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// any-user entry
			case SHORTOPT_ANY: {
				// checks if another credential type option has been already set
				if ( cred_type != authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE){
					fprintf (stderr, "\nError: multiple credential type as input\n");
					fprintf (stderr, "Specify only one type of credential: any-user | person | dnlist | voms\n\n");
				}
				// no raw-string for "any-user" credential
				user = "" ;
				// credential type
				cred_type = authorizer::GaclManager::WMPGACL_ANYUSER_TYPE ;
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// drain operation: on
			case SHORTOPT_EN_DRAIN : {
				//checks if any check, allow, deny and drain operations have been already requested
				if (c || a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--enable-drain is incompatible with other operations (allow, deny, check) and --disable-drain\n\n");
				}
				// DRAIN flag
				drain=true;
				// operation vector
				operations.push_back(DRAIN_ON) ;
				// credential type: any-user
				cred_type = authorizer::GaclManager::WMPGACL_ANYUSER_TYPE ;
				// no raw-string for "any-user" credential
				user = "";
				// denies exec operation
				deny_perm = authorizer::GaclManager::WMPGACL_EXEC;
				//cout <<"allow permission=" << permission << "\n";
				break;
			}
			// drain operation: off
			case SHORTOPT_DIS_DRAIN : {
				//checks if any check, allow, deny and drain operations have been already requested
				if (c || a || d || drain ){
					fprintf (stderr, "\nError: bad matching of the input arguments for operations.\n");
					fprintf (stderr, "--disable-drain is incompatible with other operations (allow, deny, check) and --disable-drain\n\n");
					ending(-1 );
				}
				// DRAIN flag
				drain=true;
				// operation vector
				operations.push_back(DRAIN_OFF) ;
				// credential type: any-user
				cred_type = authorizer::GaclManager::WMPGACL_ANYUSER_TYPE ;
				// no raw-string for "any-user" credential
				user = "";
				// allows exec operation
				allow_perm = authorizer::GaclManager::WMPGACL_EXEC;
				// cout <<"allow permission=" << permission << "\n";
				break;
			}
			// new file
			case SHORTOPT_NEW : {
				new_file = true;
				// cout <<"allow permission=" << permission << "\n";
				break;
			}
			// new file
			case SHORTOPT_HELP: {
				usage(prg_name);
				break;
			}
			case -1 :{
				break ;
			}

			default:
				for_usage(prg_name );
			break;
		}
	} while (next_option != -1);
	// No arguments specified
	if (argc==1){
		for_usage(prg_name );
		return 0;
	}
	// checks if options contain a credential type at least
	if ( cred_type == authorizer::GaclManager::WMPGACL_UNDEFCRED_TYPE){
		fprintf (stderr, "\nError: missing argument.\nNo credential type specified (--person | --anyuser | --voms | --dnlist )\n\n");
		ending(-1 );
	}
	// checks if only a credential type has been set
	if ( (cred_type != authorizer::GaclManager::WMPGACL_ANYUSER_TYPE) && drain ){
		fprintf (stderr, "\nError: wrong credential type.\nDrain operation is only for any-user; person, dnlist, voms are not allowed\n\n");
		ending(-1 );
	}

	// checks if options contain an operation at least
	if ( !( c || d  || a || drain || del || add ) ){
		fprintf (stderr,  "\nError: missing argument(s) (no operation specified)\n--help to see information on the operation options.\n\n");
		ending(-1 );
	}
	// sets the file default if it is not set
	if (f==false) {
		file = authorizer::GaclManager::WMPGACL_DEFAULT_FILE ;
	}
	if ( new_file) {
		fprintf (stdout, "\nCreating new gacl file: %s\n\n", file.c_str());
	} else {
		fprintf (stdout, "\nLoading the gacl file: %s\n\n", file.c_str());
	}
	// gacl manager
	authorizer::GaclManager gacl (file, new_file);
	if (!operations.empty()){
		if (cred_type== authorizer::GaclManager::WMPGACL_ANYUSER_TYPE){
			fmsg += "For any user:\n" ;
		} else {
			fmsg += "For user: " + user+ "\n" ;
			fmsg += "with credential type: " + gacl.getCredentialTypeString(cred_type) + "\n";
			fmsg += "the following operation(s) ha-s(ve) been performed:\n";
		}
	}
	// OPERATION:
	for (  it2 = operations.begin() ; it2 != operations.end() ; it2++ ) {

		switch (*it2){
			case ALLOW_PERMISSION :{
				if (gacl.hasEntry(cred_type, user) ){
					gacl.allowPermission(cred_type, user, allow_perm) ;
				} else{
					gacl.addEntry(cred_type, user, authorizer::GaclManager::WMPGACL_NOPERM);
					gacl.setAllowPermission(cred_type, user, allow_perm) ;
				}
				fmsg += "- allowed the following permission: " ;
				size = allow_vect.size() ;
				for (int i = 0; i < size ; i++){
					fmsg += allow_vect[i];
					if (i==(size-1)) {
						fmsg += "\n" ;
					} else {
						fmsg += ", ";
					}
				}
				break;
			}
			case DENY_PERMISSION :{
				if (gacl.hasEntry(cred_type, user)){
					gacl.denyPermission(cred_type, user, deny_perm) ;
				} else {
					gacl.addEntry(cred_type, user, authorizer::GaclManager::WMPGACL_NOPERM) ;
					gacl.setDenyPermission(cred_type, user, deny_perm) ;
				}
				fmsg += "- denied the following permission: " ;
				size = deny_vect.size() ;
				for ( int i = 0; i < size ; i++){
					fmsg += deny_vect[i];
					if (i==(size-1)) {
						fmsg += "\n" ;
					} else {
						fmsg += ", ";
					}
				}
				break;
			}
			case CHECK_PERMISSION :{
				size = checks.size( );
				for ( int i = 0; i < size ; i++){
					allow = gacl.checkAllowPermission(cred_type, user,checks[i]) ;
					fmsg += "- checking of permission : ";
					fmsg += check_vect[i] ;
					if (allow){
						fmsg += " is allowed.\n" ;
					} else {
						fmsg += " is not allowed\n" ;
					}
				}
				break;
			}
			case ADD :{
				gacl.addEntry(cred_type, user);
				fmsg += "- Added user : " + user + "\n";
				fmsg += "   with read permission\n";
				break;
			}
			case DELETE :{
				string errors = "";
				if (gacl.removeEntry(cred_type, user, errors) < 0){
					fprintf (stderr, "\nError: gacl entry removal failed;%s\n", errors.c_str());
					ending(-1 );
				}
				fmsg += "- cancellation of the entry\n";
				break;
			}
			case DRAIN_ON :{
				if (gacl.checkDenyPermission(cred_type,user,deny_perm) ){
					fprintf (stderr,"Error: couldn't enable DRAIN\nIt is already active; exec permission is currently denied for any user\n\n" );
					ending(-1);
				} else
				gacl.denyPermission(cred_type, user,deny_perm) ;
				fmsg += "- Drain (exec permission has been denied)\n";
				break;
			}
			case DRAIN_OFF :{
				if (gacl.checkAllowPermission(cred_type,user,allow_perm) ){
					fprintf (stderr,"Error: couldn't disable DRAIN\nIt is not active; exec permission is currently allowed for any user)\n\n" );
					ending(-1);
				} else {
					gacl.allowPermission(cred_type, user,allow_perm);
					fmsg += "- Disabled drain (exec permission has been allowed)\n";
				}
				break;
			}
			default :{
				fprintf (stderr, "Error : no operation set.\n--help to see information on the operation options.\n\n");
				ending(-1);
				break;
			}
		}

	}
	// Saves the file
	if ( !c ) {
		gacl.saveGacl( );
		fmsg += "\nGacl saved in the file: " + gacl.getFilepath( ) + "\n";
	}
	// Output Message
	fprintf (stdout, "=====================================================================\n\n");
	fprintf (stdout, "\tGacl Administration Tool\n\n");
	fprintf (stdout, "%s\n", fmsg.c_str( ) );
	fprintf (stdout, "=====================================================================\n");

	return 0;
}

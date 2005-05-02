/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html

	glite-wmp-job-output

*/

#include "wmp_job_examples.h"
#include <vector>

// CURL
#include <curl/curl.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/types.h>

using namespace std;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapi::examples::utilities;

static const char* LONGOPT_SERVICE = "endpoint" ;
static const char* LONGOPT_JOBID = "jobid";
static const char* LONGOPT_DIR = "dir";
static const char* LONGOPT_LIST = "listonly";
static const char* LONGOPT_VERBOSE = "verbose";
static const char* LONGOPT_HELP = "help";

static const char SHORTOPT_SERVICE = 'e';
static const char SHORTOPT_JOBID = 'j';
static const char SHORTOPT_DIR = 'd';
static const char SHORTOPT_LIST = 'l';
static const char SHORTOPT_VERBOSE = 'v';
static const char SHORTOPT_HELP = '?';
/*
	long options
*/

static const struct option long_options[] = {
    { LONGOPT_SERVICE,              required_argument,	NULL, SHORTOPT_SERVICE},
    { LONGOPT_JOBID,			required_argument,	NULL, SHORTOPT_JOBID },
    { LONGOPT_DIR,			required_argument,	NULL, SHORTOPT_DIR},
    { LONGOPT_LIST,			no_argument , 		NULL, SHORTOPT_LIST},
    { LONGOPT_VERBOSE,		no_argument ,		NULL, SHORTOPT_VERBOSE},
    { LONGOPT_HELP,			no_argument ,		NULL, SHORTOPT_HELP},
    {0, 0, 0, 0}
};

/*
	short options
*/
static const char* const short_options = "e:j:o:lv?";

static void short_usage(const char *exe)
{
	fprintf(stderr,"\nUsage: %s arguments [options]\n",exe);
	fprintf(stderr,"where \n");
	fprintf(stderr,"arguments (mandatory):\n");
	fprintf(stderr,"--%s, -%c\t\t<wmproxy-service_url>\n",
		LONGOPT_SERVICE, SHORTOPT_SERVICE);
	fprintf(stderr,"--%s, -%c	\t\t<jobid string> | <jobid file>\n",
		LONGOPT_JOBID , SHORTOPT_JOBID);
	fprintf(stderr,"\noptions (not mandatory):\n\n");

	fprintf(stderr,"\t--%s, -%c\t\t<endpoint URL> (mandatory if %s is not set)\n",
		LONGOPT_SERVICE , SHORTOPT_SERVICE, GLITE_WMPROXY_ENDPOINT);

	fprintf(stderr,"--%s, -%c	\t\t<output directory>\n",
		LONGOPT_DIR , SHORTOPT_DIR);

	fprintf(stderr,"--%s, -%c	\n",
		LONGOPT_LIST , SHORTOPT_LIST);

	fprintf(stderr,"\t--%s -%c\n",
		LONGOPT_VERBOSE , SHORTOPT_VERBOSE);

	fprintf(stderr,"--%s, -%c	\n",
		LONGOPT_HELP , SHORTOPT_HELP);

	fprintf(stderr,"\nWMProxy Testing User Interface\n");
	fprintf(stderr,"Copyright (C) 2005 by DATAMAT SpA\n\n");
	fprintf(stderr, "Please report any bug at:\n\t\tfabrizio.pacini@datamat.it\n");

	exit (-1);
}
/*
	usage message
*/
static void long_usage(const char *exe)
{
	fprintf(stderr,"\nUsage: %s arguments\n",exe);
	fprintf(stderr,"where  \n");
	fprintf(stderr,"arguments (mandatory):\n");
	fprintf(stderr, "\t-%c url\n", SHORTOPT_SERVICE);
	fprintf(stderr, "\t--%s url\n", LONGOPT_SERVICE);
	fprintf(stderr, "\twmproxy service URL (e.g.: https://<server>:<port>/<wmProxy-fcgi>)\n\n");

	fprintf(stderr, "\t-%c jobid_string | jobid_file", SHORTOPT_JOBID);
	fprintf(stderr, "\t--%s jobid_string | jobid_file\n", LONGOPT_JOBID);


	fprintf(stderr,"\noptions (not mandatory):\n\n");

	fprintf(stderr, "\t-%c url\n", SHORTOPT_SERVICE);
	fprintf(stderr, "\t--%s url\n", LONGOPT_SERVICE);
	fprintf(stderr, "\twmproxy endpoint URL, overriding any setting of %s;\n",
		GLITE_WMPROXY_ENDPOINT);
	fprintf(stderr, "\tmandatory if the %s is not set.\n", GLITE_WMPROXY_ENDPOINT);
	fprintf(stderr, "\t(e.g.: https://<server>:<port>/<wmProxy-fcgi>)\n\n");


	fprintf(stderr, "\t-%c\n", SHORTOPT_LIST);
	fprintf(stderr, "\t--%s\n", LONGOPT_LIST);
	fprintf(stderr, "\the job output files will be only listed.\n");

	fprintf(stderr, "\t-%c out_dir\n", SHORTOPT_DIR);
	fprintf(stderr, "\t--%s out_dir\n", LONGOPT_DIR);
  	fprintf(stderr, "\tthe directory of the local machine where the output file will be downloaded\n\n");

	fprintf(stderr, "\t-%c\n", SHORTOPT_LIST);
	fprintf(stderr, "\t--%s\n", LONGOPT_LIST);
  	fprintf(stderr, "\tonly the listening (the files are not downloaded on the local machine)\n\n");

	fprintf(stderr, "\t-%c\n", SHORTOPT_VERBOSE);
	fprintf(stderr, "\t--%s\n", LONGOPT_VERBOSE);
	fprintf(stderr, "\tEnables verbose output\n\n");

	fprintf(stderr, "\t--help, -?\n");
	fprintf(stderr, "\tdisplays this message\n\n");

	fprintf(stderr,"\nWMProxy Testing User Interface\n");
	fprintf(stderr,"Copyright (C) 2005 by DATAMAT SpA\n\n");
	fprintf(stderr, "Please report any bug at:\n\t\tfabrizio.pacini@datamat.it\n");

	exit (-1);
}
void listResult(vector <pair<std::string , long> > &vect, const string jobid )
{
	vector <pair<string , long> >::iterator it ;
	string name = "";
	long size = 0;
	char s_size[1024] = "";

	string outmsg = "*************************************************************\n";
	outmsg += "OUTPUT FILE LIST for the job :\n\n";
	outmsg += "jobid:\n\t" + jobid + "\n\n";
	if ( vect.empty() ){
		outmsg += "no files for the job\n";
	} else {
		for ( it = vect.begin() ; it != vect.end() ; it++ ){
			name = it->first ;
			sprintf (s_size, "%lx", size);
			outmsg += " - " + name + "\t\tsize:\t [ " + s_size + " ]\n" ;
		}
	}

	outmsg += "\n*************************************************************\n";

	fprintf (stdout, "\n%s", outmsg.c_str());
}

struct httpfile { char *filename; FILE* stream; } ;

int storegprbody(void *buffer, size_t size, size_t nmemb, void *stream)
 {
	struct httpfile *out=(struct httpfile*)stream;
	if(out && !out->stream) {
		// open stream
		out->stream=fopen(out->filename, "wb");
		if(!out->stream) {
			return -1;
		}
   	}
  	return fwrite(buffer, size, nmemb, out->stream);
 }

void getFiles (vector <pair<std::string , long> > &vect, string* dirout, bool verbose  )
{
	CURL *curl = NULL;
	vector <pair<std::string , long> >::iterator it ;
	string source = "" ;
	string destination = "" ;
	string file = "" ;
	CURLcode res;
	char *proxy = NULL;
	unsigned int p = 0;

	// user proxy
	proxy = getenv ("X509_USER_PROXY");
	if ( ! proxy ){
		proxy = (char*)malloc (1024);
		sprintf (proxy, "/tmp/x509up_u%d", geteuid());
	 }


	// curl init
	curl_global_init(CURL_GLOBAL_ALL);
 	curl = curl_easy_init();

	if ( curl ) {
		/*
		params.req = NULL;
		params.len = 0;
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &params);
		*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, storegprbody);

		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,  "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLCERT,     proxy);

		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEY,      proxy);
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);


		curl_easy_setopt(curl, CURLOPT_CAPATH, "/etc/grid-security/certificates/");
		//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET-OUTPUT-FILES");
		//curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		/*
		asprintf(&delheader, "Delegation-ID: %s", delegid);
		headerlist = curl_slist_append(headerlist, delheader);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		*/

		if ( verbose ){
			cout << "file transferring ....." << endl;
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			cout << "user proxy : " << proxy << endl;
		}
		//curl_easy_setopt(curl, CURLOPT_DEBUGDATA,     debugfp);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debugfunction);

		for ( it = vect.begin( ) ; it != vect.end( ) ; it++ ){
			source = it->first ;


			curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
			p = source.find_last_of("/");

			if ( p != string::npos ){
				destination = source.substr( p+1 , source.size( ) ) ;
			}

			if ( dirout ){
				destination = *dirout + "/" + destination ;
			}
			if ( verbose ){
				cout << "source : " << source << "\n";
				cout << "destination : " << destination<< "\n";
			}

			struct httpfile params={
				(char*)destination.c_str() ,
 				NULL
  			};
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &params);

			res = curl_easy_perform(curl);

		}

	}
}
/*
	main
*/
int main (int argc,char **argv)
{

	int next_option = -1;
 	char *prg_name = argv[0];
	char *serv_ev = NULL;
	string *service = NULL;
	string *jobid = NULL;
	string *dir_out = NULL;
	string errmsg = "";
	bool list_only = false;
	bool verbose = false;


	ConfigContext *cfs = NULL;
	vector <pair<std::string , long> > files ;

	// check options
	do {

		next_option = getopt_long(argc,
					argv,
					short_options,
					long_options,
					NULL);
cout << "a\n";
		switch(next_option) {
			case SHORTOPT_SERVICE: {
				service = new string(optarg);
				//cout << "debug - service=[" << *service << "]" << endl;
				break;
			}
			case SHORTOPT_JOBID: {
				jobid = new string(optarg);
				//cout << "debug - jobid=[" << *jobid << "]" << endl;
				break;
			}
			case SHORTOPT_DIR: {
				dir_out= new string(optarg);
				//cout << "debug - out=[" << *fout << "]" << endl;
				break;
			}
			case SHORTOPT_LIST: {
				list_only = true;
				//cout << "debug - out=[" << *fout << "]" << endl;
				break;
			}
			case SHORTOPT_VERBOSE: {
				verbose = true ;
				break;
			}
			case SHORTOPT_HELP: {
				long_usage (prg_name);
				break;
			}
			case -1: {cout << "-1\n";
				break;
			}
			default:{cout << "default\n";
				short_usage (prg_name);
				break;
			}

		} // switch
	} while (next_option != -1);

	// service
	if ( !service ){
		serv_ev = getenv (GLITE_WMPROXY_ENDPOINT);
		if ( ! serv_ev ){
			fprintf (stderr, "error: unknown wmproxy endpoint URL\n");
			fprintf (stderr, "set the %s environment variable\n",GLITE_WMPROXY_ENDPOINT );
			fprintf(stderr, "or use the --%s <endpoint_URL> (-%c <endpoint_URL>) option\n",
						LONGOPT_SERVICE, SHORTOPT_SERVICE);
			short_usage(prg_name);
		} else {
			service = new string(serv_ev);
		}
	}
	// jobid
	if ( !jobid ){
		errmsg = errmsg.assign("--").append(LONGOPT_JOBID).append(" <job identifier>");
		fprintf (stderr, "error: the following mandatory option is missing:\n%s\n", errmsg.c_str() );
		short_usage(prg_name);
	} else {
		// checks if the jobid string is a path to file
		string * j_id =  jobidFromFile ( *jobid );
		if ( j_id ) {
			jobid = new string ( *j_id ) ;
		}
	}

	if ( verbose ){
		cout << "\nGetting job output files list ...." << endl ;
		cout << "jobid : " << *jobid << endl;
		cout << "endpoint :  " << *service << endl ;
	}

	try {
/*
		// config context
		cfs = new ConfigContext("", service->c_str(), GLITE_TRUSTED_CERTS);

		// start
	 	files = getOutputFileList (*jobid, cfs);

		if ( verbose ) {
			cout << "\nInput Sandbox file list : successfully received" << endl;
		}
		// displays result message (in case it is saved into the user file)
		listResult( files, *jobid );
*/

		files.push_back ( make_pair ( "https://10.3.1.43:4573/SandboxDir/3a/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2f3aQINVgbhZS0PNGDu5nz0Q/output/f1.txt" ,
			7 ) );
		files.push_back ( make_pair ("https://10.3.1.43:4573/SandboxDir/3a/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2f3aQINVgbhZS0PNGDu5nz0Q/output/f2.txt" ,
			7 ) );
		dir_out = new string( "/home/msotty/tmp" );

		if ( ! list_only ) {
			getFiles (files, dir_out, verbose) ;
		}

	} catch( BaseException &b_ex) {
		fprintf ( stderr, "Unable to retrieve the output job file list for the job:\n%s\n\nException caught:\n", jobid->c_str());
		errmsg = handle_exception ( b_ex );
		fprintf (stderr, "%s\n", errmsg.c_str() );
		exit (-1);
	}
	return 0;
}

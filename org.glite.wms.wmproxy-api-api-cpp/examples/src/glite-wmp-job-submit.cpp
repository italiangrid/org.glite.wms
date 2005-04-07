/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html

	glite-wmp-job-submit

*/

#include "wmp_job_examples.h"
#include <vector>
#include <fstream>

// AD
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/ExpDagAd.h"


#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "requestad/extractfiles.h"

// CURL
#include <curl/curl.h>

#include <sys/stat.h>
#include <fcntl.h>


using namespace std;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapi::examples::utilities;
using namespace glite::wms::jdl;

static const char* LONGOPT_SERVICE = "endpoint" ;
static const char* LONGOPT_JDL= "jdl-file" ;
static const char* LONGOPT_DELEGATION = "delegation";
static const char* LONGOPT_OUTPUT = "output";
static const char* LONGOPT_VERBOSE = "verbose";
static const char* LONGOPT_HELP = "help";

static const char SHORTOPT_SERVICE = 'e';
static const char SHORTOPT_JDL = 'f';
static const char SHORTOPT_DELEGATION = 'd';
static const char SHORTOPT_OUTPUT = 'o';
static const char SHORTOPT_VERBOSE = 'v';
static const char SHORTOPT_HELP = '?';
/*
	long options
*/

static const struct option long_options[] = {
    { LONGOPT_SERVICE,              required_argument,	NULL, SHORTOPT_SERVICE},
    { LONGOPT_JDL,           		required_argument,	NULL, SHORTOPT_JDL},
    { LONGOPT_DELEGATION,	required_argument,	NULL, SHORTOPT_DELEGATION },
    { LONGOPT_OUTPUT,		required_argument,	NULL, SHORTOPT_OUTPUT},
    { LONGOPT_VERBOSE,		no_argument ,		NULL, SHORTOPT_VERBOSE},
    { LONGOPT_HELP,			no_argument ,		NULL, SHORTOPT_HELP},
    {0, 0, 0, 0}
};

/*
	short options
*/

static const char* const short_options = "e:f:d:o:v?";

static void short_usage(const char *exe)
{
	fprintf(stderr,"\nUsage: %s arguments [options] \n",exe);
	fprintf(stderr,"where  \n");
	fprintf(stderr,"arguments (mandatory):\n");

	fprintf(stderr,"\t--%s, -%c	 <delegation string>\n",
		LONGOPT_DELEGATION , SHORTOPT_DELEGATION);

	fprintf(stderr,"\noptions (not mandatory):\n\n");

	fprintf(stderr,"\t--%s, -%c		<endpoint URL> (mandatory if the %s is not set)\n",
		LONGOPT_SERVICE , SHORTOPT_SERVICE, GLITE_WMPROXY_ENDPOINT);

	fprintf(stderr,"\t--%s -%c		<output file>\n",
		LONGOPT_OUTPUT , SHORTOPT_OUTPUT);
	fprintf(stderr,"\noptions (not mandatory):\n\n");

	fprintf(stderr,"\t--%s -%c		<output dir>\n",
		LONGOPT_OUTPUT , SHORTOPT_OUTPUT);

	fprintf(stderr,"\t--%s -%c\n",
		LONGOPT_VERBOSE , SHORTOPT_VERBOSE);

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
	fprintf(stderr,"\nUsage: %s arguments [options] \n",exe);
	fprintf(stderr,"where  \n");
	fprintf(stderr,"arguments (mandatory):\n");

	fprintf(stderr, "\t-%c file_path\n", SHORTOPT_JDL);
	fprintf(stderr, "\t--%s file_path\n",LONGOPT_JDL);
	fprintf(stderr, "\tthe file containing the JDL describing the job to be submitted.\n\tfile_path can be either a simple name or an absolute path (on the submitting machine).\n\n");

	fprintf(stderr, "\t-%c del_string",SHORTOPT_DELEGATION);
	fprintf(stderr, "\t--%s del_string\n",LONGOPT_DELEGATION);
	fprintf(stderr, "\tuser delegation-id string\n\n");

	fprintf(stderr,"options (not mandatory):\n");

	fprintf(stderr, "\t-%c url\n", SHORTOPT_SERVICE);
	fprintf(stderr, "\t--%s url\n", LONGOPT_SERVICE);
	fprintf(stderr, "\twmproxy endpoint URL, overriding any setting of %s;\n",
		GLITE_WMPROXY_ENDPOINT);
	fprintf(stderr, "\tmandatory if the %s is not set.\n", GLITE_WMPROXY_ENDPOINT);
	fprintf(stderr, "\t(e.g.: https://<server>:<port>/<wmProxy-fcgi>)\n\n");

	fprintf(stderr, "\t-%c out_file\n", SHORTOPT_OUTPUT);
	fprintf(stderr, "\t--%s out_file\n", LONGOPT_OUTPUT);
  	fprintf(stderr, "\twrites the generated edg_jobId assigned to the submitted job in the file specified by out_file.\n\tout_file can be either a simple name or an absolute path (on the submitting machine).\n\n");

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

int debugfunction(CURL *curl, curl_infotype type, char *s, size_t n, void *p)
{
  //fwrite(s, sizeof(char), n, (FILE *) p);

  fprintf (stderr, "==debug> %s\n", s);

  return 0;
}

 size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
   size_t retcode;

   /* in real-world cases, this would probably get this data differently
      as this fread() stuff is exactly what the library already would do
      by default internally */
   retcode = fread(ptr, size, nmemb, (FILE*)stream);

  fprintf(stderr, "*** We read %d bytes from file \n", retcode);

   return retcode;
 }

bool checkFreeQuota ( vector<pair<string,string> > files, long offset, long limit, bool verbose )
 {
	int hd = 0;
	string file = "";
	struct stat file_info;
	FILE* fptr = NULL;
	vector<pair<string,string> >::iterator it ;
	bool not_exceed = true;

	long size = offset;

	if ( verbose ) {
		cout << "\nchecking user free quota.....";
	}

	for ( it = files.begin() ;  it != files.end() ; it++ ) {

		file = it->first;
		fptr = fopen(file.c_str(), "rb");

		if (fptr == NULL) {
			cerr << "\nerror : InputSandbox file not found (" << file<< ")" << endl;
			exit(-1);
		}

		hd = open(file.c_str(), O_RDONLY) ;
		fstat(hd, &file_info);
		close(hd) ;
		size += file_info.st_size ;
		if ( size > limit) {
			not_exceed = false;
			fclose (fptr);
			break;
		}

		fclose (fptr);
	}

	if ( verbose ) {
		if ( not_exceed ){
			cout << " ok\n";
			cout << "\nfree quota is " << limit << " ; InputSandbox file size is " << size << " byte." << endl;
		} else{
			cout << " failed\n";
		}
	}

	return not_exceed ;
 }

void transferFiles (vector<pair<string,string> > files, bool verbose)
{

	CURL *curl = NULL;
	vector <pair<string , string> >::iterator it ;

	string source = "" ;
	string destination = "" ;
	string file = "" ;
	CURLcode res;
	int hd = 0;
	struct stat file_info;
	FILE * hd_src  = NULL;
	char *proxy = NULL;
	unsigned int p = 0;
	long	httpcode = 0;
/*
	vector<string> sources ;
	string dest_uri = "https://10.3.1.43:4573/SandboxDir/YJ/https_3a_2f_2fgundam.cnaf.infn.it_3a9000_2fYJ4-65P-_5f4H-nfYxSBpXLg/input";
	sources.push_back ("/home/msotty/Repository/cern/org.glite.wms.wmproxy-api-cpp/examples/bin/glite-wmp-job-register");
	sources.push_back("/home/msotty/Repository/cern/org.glite.wms.wmproxy/src/authorizer/wmp-gacl-admin");

*/
	// user proxy
	proxy = getenv ("X509_USER_PROXY");
	if ( ! proxy ){
		proxy = (char*)malloc (1024);
		sprintf (proxy, "/tmp/x509up_u%d", geteuid());
	 }



	// curl init
	curl_global_init(CURL_GLOBAL_ALL);
 	curl = curl_easy_init();

	 if(curl) {

	 	if ( verbose ) {
			cout << "Input Sandbox file transferring ....\n---------------------------" << endl;
			cout << "user proxy : " << proxy << endl;
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		}
		// curl options: proxy
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
     		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLCERT,	proxy);
       		curl_easy_setopt(curl, CURLOPT_SSLKEY, proxy);
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
      		 curl_easy_setopt(curl, CURLOPT_CAPATH, "/etc/grid-security/certificates");

		// curl options: no verify the ssl properties
		 curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
      		 curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

		// enable uploading
		curl_easy_setopt(curl, CURLOPT_PUT, 1);

		// name of the file to be transferred
		for ( it = files.begin() ; it != files.end() ; it++ ){
			source = it->first ;
			p = source.find_last_of("/");

			if ( p != string::npos ){
				file = source.substr( p+1 , source.size( ) ) ;
			} else{
				file = file.assign(source);
			}


			destination = it->second + "/" + file  ;

			if ( verbose ) {
				cout << "InputSandbox file : " << source << endl;
				cout << "destination URI : " << destination << endl;
			}


			// curl options: destination
			curl_easy_setopt(curl,CURLOPT_URL, destination.c_str());

			// curl options: source
			hd_src = fopen(source.c_str(), "rb");
			if (hd_src == NULL) {
				cerr << "\nerror : InputSandbox file not found (" << source << ")" << endl;
				exit(-1);
			}
			curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

			// curl options: file size
			hd = open(source.c_str(), O_RDONLY) ;
			fstat(hd, &file_info);
			close(hd) ;
			curl_easy_setopt(curl,CURLOPT_INFILESIZE, file_info.st_size);

			if ( verbose ) {
				cout << "InputSandbox file : " << source << endl;
				cout << "destination URI : " << destination << endl;
				cout << "size : " << file_info.st_size << " byte"<< endl;
				cout << "\ntransferring ..... " ;
			}


			// transferring ......
			res = curl_easy_perform(curl);
			cout << "res="<< res << "\n";

			if ( res != 0 ){
				cerr << "\nerror: couldn't transfer the InputSandbox file : " + source << endl;
				cerr << "to " + destination << endl ;
				curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
				cerr << "http error code : " << httpcode << endl;
			} else {
				if ( verbose ) {
						cout << "success" << endl ;
				}
			}

			fclose(hd_src);
		}
	// cleanup
	curl_easy_cleanup(curl);
	}

}

void submission(const JobIdApi &j_id, const string endpoint, Ad ad, ExpDagAd *dag, ConfigContext *cfs, long limit, const string *fout, bool verbose )
{
	string jobid = "";
	string *dest_uri = NULL;
	string protocol = "";
	string source = "";
	string *node = NULL;
	vector <string> files ;
	vector <string> dest ;
	vector <JobIdApi*> children;
	//vector <pair<string, long> >::iterator it1;
	vector <string>::iterator it1;
	vector <JobIdApi*>::iterator it2;
	JobIdApi* child = NULL;

	vector <pair<string, string> > transfer_vect ;

	// Main job: JOB-ID
	jobid = j_id.jobid ;

	// Main  job: First part of the result message
	string outmsg = "*************************************************************\n";
	outmsg += "REGISTER OPERATION :\n\n\n";
	outmsg += "your job has been successfully registered by the end point:\n\n";

	outmsg += "\t" + endpoint + "\n\n";
	jobid = j_id.jobid ;
	outmsg += "with the jobid:\n\n\t" + jobid + "\n\n";
	node = j_id.nodeName ;
	if (node && node->size() > 0 ){
		outmsg += "node: " + *node + "\n\n";
	}

	// Main job: InputSandbox files
	files = ad.getStringValue  ( JDL::INPUTSB) ;


	// http destination URI for the main job
	dest = getSandboxDestURI (jobid, cfs);
	for (it1 = dest.begin() ; it1 != dest.end() ; it1++){
		dest_uri = new string( *it1 );
		if ( dest_uri->substr (0, 4) == "http" ){
			cout << "http !!!\n" ;
			break;
		} else {
			dest_uri = NULL;
		}
	}
	if ( dest_uri){
		cout << "dest_uri=" << *dest_uri << "\n";
		*dest_uri += "/input" ;

	}

	// inserts InputSandbox file
	for ( it1 = files.begin( ) ; it1 != files.end() ; it1++ ){
		source = *it1;
		transfer_vect.push_back( make_pair (source, *dest_uri) );
	}


	// Children vector
	children = j_id.children ;

	// Second part of the result message (children)
	if ( ! children.empty() ){
		outmsg += "\tchildren:\n";
		outmsg += "\t-----------------------------\n\n";

		for ( it2 = children.begin() ; it2 != children.end(); it2++){
			child = *it2;
			if (child){
				outmsg += "\tjobid: " + child->jobid + "\n";
				node = child->nodeName ;

				//files = dag->getStringValue  (*node, JDL::INPUTSB) ;

//cout << "insb=[" << insb << "]\n";
				if (node && node->size() > 0 ){
					outmsg += "\tnode: " + *node + "\n\n";
				}
				outmsg += "-----------------------------\n";
			}
		}
	}
	outmsg += "*************************************************************\n";


	// checks if the InputSanbox file exceeds the user free quota
	if ( limit > 0 ){
		if ( ! checkFreeQuota (transfer_vect, 0, limit, verbose ) ){
			cerr << "\nerror: unable to perform the submission" << endl;
			cerr << "(the size of your files in the JDL InputSanbox file exceeds your endpoint free quota)"<< endl;
			cerr << "endpoint : " << endpoint << endl;
			exit(-1);
		}
	}

	// InputSandbox file transferring
	transferFiles (transfer_vect, verbose);

	// Displays the output message
	fprintf (stdout, "\n%s", outmsg.c_str());

	// saves the result
	if (fout){
		if ( saveToFile(*fout, outmsg) < 0 ){
			fprintf (stderr, "\nwarning: unable to write the result in %s\n", fout->c_str());
		} else{
			fprintf (stdout, "\nThis message has been saved in %s\n", fout->c_str() );
		}
	}
}
/*
void getOperationResult(const JobIdApi &j_id, const string endpoint, Ad ad, ExpDagAd *dag, ConfigContext *cfs, long limit, const string *fout )
{
	string jobid = "";
	string *dest_uri = NULL;
	string protocol = "";
	string *node = NULL;
	vector <string> files ;
	vector <string> dest ;
	vector <JobIdApi*> children;
	//vector <pair<string, long> >::iterator it1;
	vector <string>::iterator it1;
	vector <JobIdApi*>::iterator it2;
	JobIdApi* child = NULL;





	jobid = j_id.jobid ;
	files = ad.getStringValue  ( JDL::INPUTSB) ;

	if ( limit > 0 ){
		if ( ! checkFreeQuota (files, 0, limit) ){
			cerr << "\nerror: unable to perform the submission" << endl;
			cerr << "(the size of your files in the JDL InputSanbox file exceeds your endpoint free quota)"<< endl;
			cerr << "endpoint : " << endpoint << endl;
			exit(-1);
		}
	}

	// http destination URI for the main job
	dest = getSandboxDestURI (jobid, cfs);

	for (it1 = dest.begin() ; it1 != dest.end() ; it1++){
		dest_uri = new string( *it1 );
		if ( dest_uri->substr (0, 4) == "http" ){
			cout << "http !!!\n" ;
			break;
		} else {
			dest_uri = NULL;
		}
	}

	if ( dest_uri){
		cout << "dest_uri=" << *dest_uri << "\n";
		*dest_uri += "/input" ;
		transferFiles (files, *dest_uri);
	}

	string outmsg = "*************************************************************\n";
	outmsg += "REGISTER OPERATION :\n\n\n";
	outmsg += "your job has been successfully registered by the end point:\n\n";

	outmsg += "\t" + endpoint + "\n\n";
	jobid = j_id.jobid ;
	outmsg += "with the jobid:\n\n\t" + jobid + "\n\n";
	node = j_id.nodeName ;
	if (node && node->size() > 0 ){
		outmsg += "node: " + *node + "\n\n";
	}

	children = j_id.children ;


	if ( ! children.empty() ){
		outmsg += "\tchildren:\n";
		outmsg += "\t-----------------------------\n\n";

		for ( it2 = children.begin() ; it2 != children.end(); it2++){
			child = *it2;
			if (child){
				outmsg += "\tjobid: " + child->jobid + "\n";
				node = child->nodeName ;

string insb = dag->getNodeAttribute (*node, JDL::INPUTSB) ;

cout << "insb=[" << insb << "]\n";
				if (node && node->size() > 0 ){
					outmsg += "\tnode: " + *node + "\n\n";
				}
				outmsg += "-----------------------------\n";
			}
		}
	}
	outmsg += "*************************************************************\n";

	fprintf (stdout, "\n%s", outmsg.c_str());

	// saves the result
	if (fout){
		if ( saveToFile(*fout, outmsg) < 0 ){
			fprintf (stderr, "\nwarning: unable to write the result in %s\n", fout->c_str());
		} else{
			fprintf (stdout, "\nThis message has been saved in %s\n", fout->c_str() );
		}
	}
}
*/
/*
	main
*/
int main (int argc,char **argv)
{

	int next_option = -1;
 	char *prg_name = argv[0];
	string *service =NULL;
	char* serv_ev = NULL;
	string *jdl = NULL;
	string *del_id = NULL;
	string *fout =NULL;
	string errmsg = "";
	string dag_jdl = "";
	vector<string> files ;
	pair<long, long> free_quota ;
	long soft_limit = 0;
	bool verbose = false;
	ConfigContext *cfs = NULL;

	
	// check options
	do {
		next_option = getopt_long (argc,
					argv,
					short_options,
					long_options,
					NULL);

		switch(next_option) {
			case SHORTOPT_SERVICE: {
				service = new string(optarg);
				//cout << "debug - service=[" << *service << "]" << endl;
				break;
			}
			case SHORTOPT_JDL : {
				jdl = new string(optarg);
				//cout << "debug - jdl=[" << *jdl << "]" << endl;
				break;
			}
			case SHORTOPT_DELEGATION: {
				del_id = new string(optarg);
				//cout << debug - "del_id=[" << *del_id << "]" << endl;
				break;
			}
			case SHORTOPT_OUTPUT: {
				fout = new string(optarg);
				//cout << "debug - out=[" << *fout << "]" << endl;
				break;
			}
			case SHORTOPT_VERBOSE: {
				verbose = true ;
				break;
			}
			case SHORTOPT_HELP : {
				long_usage (prg_name);
				break;
			}
			case -1: {
				break;
			}
			default:{
				short_usage (prg_name);
				break;
			}

		} // switch
	} while (next_option != -1);

	// checks if the mandatory attribute are missing
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

	if ( verbose ){
		cout << "\nSubmitting job to : " << *service << endl ;
	}

	// jobid
	if ( !jdl ){
		errmsg = errmsg.assign("--").append(LONGOPT_JDL).append(" <JDL file>");
		fprintf (stderr, "error: the following mandatory option is missing:\n%s\n", errmsg.c_str() );
		short_usage(prg_name);
	} else {

		FILE *fptr = fopen ( jdl->c_str(), "r" );
		if ( ! fptr ){
			cerr << "\nerror: no such JDL file (" << *jdl << ")\n\n" << endl;
			exit(-1);
		}
		fclose(fptr);
	}

	// jobid
	if ( !del_id ){
		errmsg = errmsg.assign("--").append(LONGOPT_DELEGATION).append(" <delegation identifier>");
		fprintf (stderr, "error: the following mandatory option is missing:\n%s\n", errmsg.c_str() );
		short_usage(prg_name);
	}

	try {

		// config context
		cfs = new ConfigContext("", *service, GLITE_TRUSTED_CERTS);

		// reads JDL from file on the  local machine
		Ad ad ;
		ExpDagAd *dag = NULL;
		ad.fromFile (*jdl);
		string jdl_string = ad.toString( );

		// DAG JOB
		if ( ad.hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
			dag = new ExpDagAd ( jdl_string );
			dag_jdl = dag->toString() ;
			if ( verbose ) {
				cout << "\nDAG job :" << endl ;
				cout << "\ndag-JDL=[" << dag_jdl << "]" << endl;
			}

			dag->expand( );
			jdl_string = jdl_string.assign(dag_jdl) ;
		}

		if ( verbose ) {
			cout << "\nJDL=[" << jdl_string << "]\n";
			cout << "\nRegisterring job ....  " << endl ;
		}

		// register
		JobIdApi jobid = jobRegister( jdl_string , *del_id, cfs);

		if ( verbose ) {
			cout << "success" << endl ;
		}

		// user free quota
		free_quota = getFreeQuota(cfs);
		soft_limit = free_quota.first;
		soft_limit = 50 ;
		/*
		if ( soft_limit > 0 ){

			if ( ! checkFreeQuota (files, soft_limit) ){
				cerr << "\nerror: unable to perform the submission" << endl;
				cerr << "(the size of your files in the JDL InputSanbox file exceeds your endpoint free quota)"<< endl;
				cerr << "endpoint : " << *service << endl;
				exit(-1);
			}
		}
*/
		// displays result message (in case it is saved into the user file)
		submission(jobid, *service, ad, dag,  cfs, soft_limit,  fout, verbose );




	} catch( BaseException &b_ex) {
		fprintf ( stderr, "Unable to register the job\n\nException caught:\n" );
		errmsg = handle_exception ( b_ex );
		fprintf (stderr, "%s\n", errmsg.c_str() );
		exit (-1);
	}

	return 0;
}

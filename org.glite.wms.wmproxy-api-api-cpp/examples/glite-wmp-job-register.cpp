/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html

	glite-wmp-job-register

*/
#include "glite/wms/jdl/JobAd.h"
#include "wmp_job_examples.h"

#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"

using namespace std;
using namespace glite::wms::jdl;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapi::examples::utilities;


static const char* LONGOPT_SERVICE = "endpoint" ;
static const char* LONGOPT_JDL= "jdl-file" ;
static const char* LONGOPT_DELEGATION = "delegation";
static const char* LONGOPT_OUTPUT = "output";
static const char* LONGOPT_HELP = "help";

static const char SHORTOPT_SERVICE = 'e';
static const char SHORTOPT_JDL = 'f';
static const char SHORTOPT_DELEGATION = 'd';
static const char SHORTOPT_OUTPUT = 'o';
static const char SHORTOPT_HELP = '?';
/*
	long options
*/

static const struct option long_options[] = {
    { LONGOPT_SERVICE,              1,NULL, SHORTOPT_SERVICE},
    { LONGOPT_JDL,           		1,NULL, SHORTOPT_JDL},
    { LONGOPT_DELEGATION,	1,NULL, SHORTOPT_DELEGATION },
    { LONGOPT_OUTPUT,		1,NULL, SHORTOPT_OUTPUT},
    { LONGOPT_HELP,			0,NULL, SHORTOPT_HELP},
    {0, 0, 0, 0}
};

/*
	short options
*/
//static const char* const short_options = "u:a:d:c:n:v:f:h";
static const char* const short_options = "e:f:d:o:?";

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

	fprintf(stderr, "\t--help, -?\n");
	fprintf(stderr, "\tdisplays this message\n\n");
	fprintf(stderr,"\nWMProxy Testing User Interface\n");
	fprintf(stderr,"Copyright (C) 2005 by DATAMAT SpA\n\n");
	fprintf(stderr, "Please report any bug at:\n\t\tfabrizio.pacini@datamat.it\n");

	exit (-1);
}
void getOperationResult(const JobIdApi &j_id, const string endpoint, const string *fout )
{
	string *node = NULL;
	vector <JobIdApi*> children;
	vector <JobIdApi*>::iterator it;
	JobIdApi* child = NULL;

	string outmsg = "*************************************************************\n";
	outmsg += "REGISTER OPERATION :\n\n\n";
	outmsg += "your job has been successfully registered by the end point:\n\n";

	outmsg += "\t" + endpoint + "\n\n";
	outmsg += "with the jobid:\n\n\t" + j_id.jobid + "\n\n";
	node = j_id.nodeName ;
	if (node && node->size() > 0 ){
		outmsg += "node: " + *node + "\n\n";
	}

	children = j_id.children ;

	if ( ! children.empty() ){
		outmsg += "\tchildren:\n";
		outmsg += "\t-----------------------------\n\n";

		for ( it = children.begin() ; it != children.end();++it){
			child = *it ;
			cout <<  (*it)->jobid << "\n";
			if (child){
				outmsg += "\tjobid: " + child->jobid + "\n";
				node = child->nodeName ;
				if (node && node->size() > 0 ){
					outmsg += "\tnode: " + *node + "\n\n";
				}
				outmsg += "\t-----------------------------\n";
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

	ConfigContext *cfs = NULL;
	Ad ad ;
	ExpDagAd *dag = NULL;
	
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
	// jobid

	if ( !jdl ){
		errmsg = errmsg.assign("--").append(LONGOPT_JDL).append(" <JDL file>");
		fprintf (stderr, "error: the following mandatory option is missing:\n%s\n", errmsg.c_str() );
		short_usage(prg_name);
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

		// reads JDL from file on local machine
		ad.fromFile (*jdl);
		// reads JDL from file on the  local machine
		Ad ad ;

		ad.fromFile (*jdl);

		string jdl_string = ad.toString( );

		cout << "ad=[" << jdl_string << "]\n";

		if ( ad.hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
			cout << "dag !\n";
			dag = new ExpDagAd ( jdl_string );
			//ifstream jdl_file ( jdl->c_str() );
			//dag = new ExpDagAd (jdl_file );
			cout << "dag=[" << dag->toString() << "]\n";
			dag->expand( );
			jdl_string = jdl_string.assign( dag->toString()) ;

		} else
		 	cout << "no dag !\n";

		cout << "JDL=[" << jdl_string << "]\n";

		// register
		JobIdApi jobid = jobRegister( ad.toString() , *del_id, cfs);

		// displays result message (in case it is saved into the user file)
		getOperationResult( jobid, *service, fout );

	} catch( BaseException &b_ex) {
		fprintf ( stderr, "Unable to register the job\n\nException caught:\n" );
		errmsg = handle_exception ( b_ex );
		fprintf (stderr, "%s\n", errmsg.c_str() );
		exit (-1);
	}

	return 0;
}



#include "joboutput.h"
#include <string>
// streams
#include<sstream>
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// CURL
#include <curl/curl.h>

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;


namespace glite {
namespace wms{
namespace client {
namespace services {

JobOutput::JobOutput (){

	// init of the string  attributes
	input = NULL;
	dir = NULL;
	config = NULL;
        vo = NULL;
	logfile = NULL;
	// init of the boolean attributes
  	version = false;
  	noint = false;
	debug = false ;
         // parameters
        wmpEndPoint = NULL ;
        proxyFile = NULL ;
        trustedCert = NULL ;
	// option object
	opts = NULL ;
};

void JobOutput::readOptions ( int argc,char **argv){
	ostringstream err ;
	// init of option objects object
        opts = new Options(Options::JOBOUTPUT) ;
        // reads the input arguments
	opts->readOptions(argc, (const char**)argv);
	 // config & vo(no together)
        config= opts->getStringAttribute( Options::CONFIG ) ;
        vo = opts->getStringAttribute( Options::VO ) ;
	if (vo && config){
		err << "the following options cannot be specified together:\n" ;
		err << opts->getAttributeUsage(Options::VO) << "\n";
		err << opts->getAttributeUsage(Options::CONFIG) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// others options
        input = opts->getStringAttribute( Options::INPUT ) ;
	dir = opts->getStringAttribute( Options::DIR ) ;
	logfile = opts->getStringAttribute( Options::LOGFILE ) ;
 	// user proxy
	proxyFile = (char*)getProxyFile(NULL);
	if (!proxyFile){
		throw WmsClientException(__FILE__,__LINE__,
				"getProxyFile", DEFAULT_ERR_CODE,
				"Missing Proxy", "unable to determine the proxy file" );
	}
 	// trusted Certs
	trustedCert =  (char*)getTrustedCert(NULL);
	if (!trustedCert){
		throw WmsClientException(__FILE__,__LINE__,
			"getProxyFile", DEFAULT_ERR_CODE,
			"Directory Not Found", "unable to determine the trusted certificate directory" );
	}
         // list of jobids
 	jobIds = opts->getJobIds( );
        // configuration context
        cfgCxt = new ConfigContext(proxyFile, wmpEndPoint, trustedCert);
 }
 /*
 *	prints the results on the std output
 */
void JobOutput::listResult(std::vector <std::pair<std::string , long> > &files, const std::string jobid )
{
	vector <pair<string , long> >::iterator it ;
	string name = "";
	long size = 0;
	char s_size[1024] = "";
	// output message
	string outmsg = "*************************************************************\n";
	outmsg += "OUTPUT FILE LIST for the job :\n\n";
	outmsg += "jobid:\n\t" + jobid + "\n\n";
	if ( files.empty() ){
		outmsg += "no files for the job\n";
	} else {
		for ( it = files.begin() ; it != files.end() ; it++ ){
			name = it->first ;
			sprintf (s_size, "%lx", size);
			outmsg += " - " + name + "\t\tsize:\t [ " + s_size + " ]\n" ;
		}
	}

	outmsg += "\n*************************************************************\n";
	// prints the output message
	fprintf (stdout, "\n%s", outmsg.c_str());
}

/*
* writing callback for curl operations
*/
int JobOutput::storegprBody(void *buffer, size_t size, size_t nmemb, void *stream)
 {
	struct httpfile *out_stream=(struct httpfile*)stream;
	if(out_stream && !out_stream->stream) {
		// open stream
		out_stream->stream=fopen(out_stream->filename, "wb");
		if(!out_stream->stream) {
			return -1;
		}
   	}
  	return fwrite(buffer, size, nmemb, out_stream->stream);
 }
/*
*	file downloading
*/
void JobOutput::getFiles (std::vector <std::pair<std::string , long> > &files)
{
	CURL *curl = NULL;
	vector <pair<std::string , long> >::iterator it ;
	string source = "" ;
	string destination = "" ;
	string file = "" ;
	CURLcode res;
	string *proxy = NULL;
	unsigned int p = 0;
        // error messages
	ostringstream err ;
	// user proxy
	// checks the user proxy pathname
	if (!proxyFile){
		throw WmsClientException(__FILE__,__LINE__,
			"getFiles", DEFAULT_ERR_CODE,
			"Missing Proxy",
                        "unable to determine the proxy file" );
	}
	// checks the trusted cert dir
	if (!trustedCert){
		throw WmsClientException(__FILE__,__LINE__,
			"transferFiles", DEFAULT_ERR_CODE,
			"Directory Not Found",
                        "unable to determine the trusted certificate directory" );
	}
	// curl init
	curl_global_init(CURL_GLOBAL_ALL);
 	curl = curl_easy_init();
	if ( curl ) {
		// writing function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, storegprBody);
		// user proxy
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,  "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLCERT, proxyFile);
		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEY,      proxy->c_str());
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
		//trusted certificate directory
		curl_easy_setopt(curl, CURLOPT_CAPATH, trustedCert);
		//ssl options (no verify)
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		// verbose message
		if ( debug ){
			cout << "file transferring ....." << endl;
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			cout << "user proxy : " << proxy << endl;
		}
		// files to be downloaded
		for ( it = files.begin( ) ; it != files.end( ) ; it++ ){
			source = it->first ;
			curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
			p = source.find_last_of("/");
			if ( p != string::npos ){
				destination = source.substr( p+1 , source.size( ) ) ;
			}
                        // option dir
			if ( dir ){
				destination = *dir+ "/" + destination ;
			}
			if ( debug ){
				cout << "source : " << source << "\n";
				cout << "destination : " << destination<< "\n";
			}
			struct httpfile params={
				(char*)destination.c_str() ,
 				NULL
  			};
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &params);
			// downloading
			res = curl_easy_perform(curl);
			if (res<0){
				cerr << "error: failed to perform the downloading of the file" << endl;
				cerr << "source : " << source << "\n";
				cerr << "destination : " << destination<< "\n";
				throw WmsClientException(__FILE__,__LINE__,
				"transferFiles", DEFAULT_ERR_CODE,
				"File Transfer Error",
                        	 err.str() );
			}
		}
	}
}

void JobOutput::getOutput ( ){
	vector <pair<string , long> > files ;
	vector<string>::iterator it ;
	// checks that the jobids vector is not empty
        if (jobIds.empty()){
		throw WmsClientException(__FILE__,__LINE__,
			"cancel", DEFAULT_ERR_CODE,
			"Missing Information",
                        "unknown jobid(s)" );
        }
        // checks that the config-context is not null
        if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }
	// performs cancelling
        for (it = jobIds.begin() ; it != jobIds.end() ; it++){
		files = getOutputFileList(*it, cfgCxt );
                getFiles (files);
                listResult(files, *it);
        }
};

}}}} // ending namespaces

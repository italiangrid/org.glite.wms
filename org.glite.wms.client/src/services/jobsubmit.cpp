

#include "jobsubmit.h"
// streams
#include <sstream>
#include <iostream>
// CURL
#include <curl/curl.h>
// fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
// exceptions
#include "utilities/excman.h"
// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// Ad attributes and JDL methods
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/extractfiles.h"
#include "glite/wms/jdl/adconverter.h"


using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::wms::jdl;

namespace glite {
namespace wms{
namespace client {
namespace services {

/*
*	default constructor
*/
JobSubmit::JobSubmit( ){
	// init of the string attributes
	 logfile = NULL ;
	 chkpt  = NULL;
	 lmrs = NULL ;
	 to = NULL ;
	 ouput = NULL ;
	 input  = NULL;
	 config = NULL ;
	 resource = NULL ;
         vo = NULL ;
	// init of the valid attribute (long type)
	valid = 0 ;
	// init of the boolean attributes
	nomsg = false ;
	nogui = false ;
	nolisten = false ;
	noint  = false;
	version  = false;
        debug = false ;
         jdlFile = NULL ;
	// parameters
        wmpEndPoint = NULL ;
        proxyFile = NULL ;
        trustedCert = NULL ;
	// utilities objects
	wmcOpts = NULL ;
	wmcUtils = NULL ;
        // Ad
        ad = NULL;
        dag = NULL;
};

void JobSubmit::readOptions (int argc,char **argv){
	ostringstream err ;
        vector<string> wrongids;
	// init of option objects object
        wmcOpts = new Options(Options::JOBSUBMIT) ;
	wmcOpts->readOptions(argc, (const char**)argv);
        // utilities
        wmcUtils = new Utils (wmcOpts);
	// input & resource (no together)
	input = wmcOpts->getStringAttribute(Options::INPUT);
	resource = wmcOpts->getStringAttribute(Options::RESOURCE);
	if (input && resource){
		err << "the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::INPUT) << "\n";
		err << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
        // config & vo(no together)
        config= wmcOpts->getStringAttribute( Options::CONFIG ) ;
        vo = wmcOpts->getStringAttribute( Options::VO ) ;
	if (vo && config){
		err << "the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::VO) << "\n";
		err << wmcOpts->getAttributeUsage(Options::CONFIG) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// lmrs has to be used with input o resource
	lmrs = wmcOpts->getStringAttribute(Options::LMRS);
	if (lmrs && !( resource || input) ){
		err << "LRMS option cannot be specified without a resource:\n";
		err << "use " + wmcOpts->getAttributeUsage(Options::LMRS) << " with\n";
		err << wmcOpts->getAttributeUsage(Options::RESOURCE) << "\n";
		err << "or\n" + wmcOpts->getAttributeUsage(Options::INPUT) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	// "valid" & "to" (no together)
	valid = wmcOpts->getStringAttribute(Options::VALID);
	to = wmcOpts->getStringAttribute(Options::TO);
	if (valid && to){
		err << "the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::VALID) << "\n";
		err << wmcOpts->getAttributeUsage(Options::TO) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	if (valid){
		try{
			if (! Utils::isAfter(*valid, 2) ){

				throw WmsClientException(__FILE__,__LINE__,
					"readOptions", DEFAULT_ERR_CODE,
					"Invalid Time Value", "time is out of limit" );
			}
		} catch (WmsClientException &exc) {
			err << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::VALID) << ")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",err.str() );
		}
	}
	if (to){
		try{
			if (! Utils::isAfter(*to) ){

				throw WmsClientException(__FILE__,__LINE__,
					"readOptions", DEFAULT_ERR_CODE,
					"Invalid Time Value", "time is out of limit" );
			}
		} catch (WmsClientException &exc) {
			err << exc.what() << " (use: " << wmcOpts->getAttributeUsage(Options::TO) <<")\n";
			throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Wrong Time Value",err.str() );
		}
	}

        logfile = wmcOpts->getStringAttribute(Options::LOGFILE );
	chkpt=  wmcOpts->getStringAttribute( Options::CHKPT) ;
	ouput=  wmcOpts->getStringAttribute( Options::OUTPUT ) ;

        nomsg=  wmcOpts->getBoolAttribute (Options::NOMSG);
	nogui =  wmcOpts->getBoolAttribute (Options::NOGUI);
	nolisten =  wmcOpts->getBoolAttribute (Options::NOLISTEN);
	noint=  wmcOpts->getBoolAttribute (Options::NOINT) ;
	version =  wmcOpts->getBoolAttribute (Options::VERSION);

        // path to the JDL file
	jdlFile = wmcOpts->getPath2Jdl( );
        // configuration context
        cfgCxt = new ConfigContext("", wmpEndPoint, "");
	// user proxy
	proxyFile =  (char*)getProxyFile(cfgCxt);
 	// trusted Certs
	trustedCert =  (char*)getTrustedCert(cfgCxt);
}


  /*
  *	callback called during file uploading by curl
 *	in case if verbosity is on
*/
 size_t JobSubmit::readCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode;
   	retcode = fread(ptr, size, nmemb, (FILE*)stream);
	fprintf(stderr, "%d byte read\n", retcode);
	return retcode;
 }


/*
*  Uploads a set of files to the Destination URIs
*	@param paths InputSB files
*/
std::string JobSubmit::transferFiles (std::vector<std::pair<std::string,std::string> > paths)
{
	// curl struct
	CURL *curl = NULL;
	// curl result code
	CURLcode res;
	// file struct
	struct stat file_info;
	FILE * hd_src  = NULL;
	// vector iterator
	vector <pair<string , string> >::iterator it ;
	// source and destination
	string source = "" ;
	string destination = "" ;
	// local filepath
	string file = "" ;
	// result message
	string result = "";
	int hd = 0;
	long	httpcode = 0;
	long freequota=0;
	// error messages
	ostringstream err ;
	// checks user free quota
	if ( ! checkFreeQuota (paths, freequota) ){
		err << "not enough free quota on the server for InputSandbox file (freequota=" ;
		err << freequota << " bytes)\n";
		throw WmsClientException( __FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"User FreeQuota Error" , err.str());
	}
	// checks the user proxy pathname
	if (!proxyFile){
		throw WmsClientException(__FILE__,__LINE__,
			"transferFiles", DEFAULT_ERR_CODE,
			"Missing Proxy", "unable to determine the proxy file" );
	}
	// checks the trusted cert dir
	if (!trustedCert){
		throw WmsClientException(__FILE__,__LINE__,
			"transferFiles", DEFAULT_ERR_CODE,
			"Directory Not Found", "unable to determine the trusted certificate directory" );
	}
	// curl init
	curl_global_init(CURL_GLOBAL_ALL);
 	curl = curl_easy_init();
	 if(curl) {
	 	if ( debug ) {
			cout << "Input Sandbox file transferring ....\n" << endl;
			cout << "user proxy : " << proxyFile << endl;
			cout << "transferring of number " << paths.size() << " files" << endl;
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION,  JobSubmit::readCallback);
		}
		// result message
		result = "\nInputSandbox file(s)  :\n";
		// curl options: proxy
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
  		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,   "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLCERT,	proxyFile);
  		curl_easy_setopt(curl, CURLOPT_SSLKEY, proxyFile);
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, NULL);
		// curl option: trusted cert dir
                curl_easy_setopt(curl, CURLOPT_CAPATH, trustedCert);
		// curl options: no verify the ssl properties
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
   		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		// enable uploading
		curl_easy_setopt(curl, CURLOPT_PUT, 1);
		// name of the file to be transferred
		for ( it = paths.begin() ; it != paths.end() ; it++ ){
			// curl options: source
			source = it->first ;
			hd_src = fopen(source.c_str(), "rb");
			if (hd_src == NULL) {
					throw WmsClientException(__FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"File Not Found", "no such file : " + it->first  );
			}
			curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
			// curl options: file size
			hd = open(source.c_str(), O_RDONLY) ;
			fstat(hd, &file_info);
			close(hd) ;
			curl_easy_setopt(curl,CURLOPT_INFILESIZE, file_info.st_size);
			// curl options: destination
			destination = it->second;
			curl_easy_setopt(curl,CURLOPT_URL, destination.c_str());
			if ( debug ) {
				cout << "InputSandbox file : " << source << endl;
				cout << "destination URI : " << destination << endl;
				cout << "size : " << file_info.st_size << " byte"<< endl;
				cout << "\ntransferring ..... " ;
			}
			// transferring ......
			res = curl_easy_perform(curl);
			// result
			if ( res != 0 ){
				curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
				err << "couldn't transfer the InputSandbox file : " << source ;
				err << "to " << destination << "\nhttp error code: " << httpcode ;
				throw WmsClientException(__FILE__,__LINE__,
						"transferFiles",  DEFAULT_ERR_CODE,
						"File Transfer Error", err.str()  );
			} else {
				if ( debug ) {
						cout << "success" << endl ;
				}
			}
			// result string message
			result += "-> " + source + "\n";
			result += "to " + destination + "\n\n";
			fclose(hd_src);
		}
		// cleanup
		curl_easy_cleanup(curl);
	}
        return result;
}

 /*
*	handles the result of the register for a normal job
*/
void JobSubmit::normalJob( )
{
	// jobid and nodename
	string id = "";
	string jobid = "";
	string node = "";
	// final output message
	string outmsg = "";
	// file transferring message
	string result = "";
	// Destination URI
	string *dest_uri = NULL;
	// InputSB files
	vector <string> paths ;
	vector <pair<string, string> > to_bcopied ;
	// JOB-ID
        jobid = Utils::checkJobId (jobIds.jobid);
	id = jobid ;
	// output message
	outmsg = "****************************************************************************\n";
	outmsg += "your job has been successfully submitted by the end point:\n\n";
	outmsg += "\t" +  string(wmpEndPoint)+ "\n\n";
	outmsg += "with the jobid:\n\n\t" + jobid + "\n\n";
	//  node name (if it is set)
	if ( jobIds.nodeName ){
		node =*jobIds.nodeName;
		if (node.size() > 0 ){
			outmsg += "node: " + node + "\n\n";
		}
	}
	// verbose msg
	if (debug){
		cout << "assigned jobid : " << jobid  << endl;
		cout << "retrieving InputSanbox files ...." << endl;
	}
	// INPUTSANDBOX
	if (ad->hasAttribute(JDL::INPUTSB)){
		// http destination URI
		dest_uri = getInputSBDestURI(jobid);
		if (!dest_uri){
			fprintf (stderr, "\nerror: unable to retrieve the InputSandbox destination URI\n");
			fprintf (stderr, "jobid: %s", jobid.c_str() );
			exit(-1);
		}
		if (debug){
			cout << "destination URI = " << *dest_uri << endl;
		}
		// InputSandbox files
		if (!ad){
			fprintf (stderr, "fatal error: unable to read the Job Description\n");
			exit(-1);
		}
		// gets the InputSandbox files
		paths = ad->getStringValue  (JDL::INPUTSB) ;
		// gets the InputSandbox files to be transferred to destinationURIs
		toBcopied(JDL::INPUTSB, paths, to_bcopied, *dest_uri,"");
		// InputSandbox file transferring
		result = transferFiles (to_bcopied);
		// result output message
		if (result.size()>0){
			outmsg += result;
		}
	}
	outmsg += "****************************************************************************\n";
	if (debug){
		cout << "\nstarting the job : " << jobid << "\n";
	}
	// START
	jobStart(id, cfgCxt);
	if (debug){
		cout << "\n job start: success\n";
		cout << "the job has been successfully submitted\n";
	}
	// displays the output message
	fprintf (stdout, "\n%s", outmsg.c_str());
 /*
	// saves the result
	if (fout){
		if ( ex_utilities::saveToFile(*fout, outmsg) < 0 ){
			fprintf (stderr, "\nwarning: unable to write the result in %s\n", fout->c_str());
		} else{
			fprintf (stdout, "\nThis message has been saved in %s\n", fout->c_str() );
		}
	}
 */
}
  /*
*	handles the result of the register for a DAG job
*/
void JobSubmit::dagJob( )
{
	// jobid and node-name
	string jobid = "";
	string id = "";
	string node = "";
	// destURI protocol
	string protocol = "";
	// Destination URI
	string *dest_uri = NULL;
	// final output message
	string outmsg = "";
	// transferring files message
	string result = "";
	// var-msg
	string x1, x2 = "";

	// InputSB files
	vector <string> paths ;
	vector <pair<string, string> > to_bcopied ;
	// children
	vector <JobIdApi*> children;
	JobIdApi* child = NULL;
	// iterators
	vector <JobIdApi*>::iterator it;
	ostringstream oss;

 	// MAIN JOB ====================
        // jobid
        jobid = Utils::checkJobId (jobIds.jobid);
        id = jobid ;
        outmsg = "****************************************************************************\n";
        outmsg += "REGISTER OPERATION :\n\n\n";
/*
                if (jobtype == COLLECTION){
                        x1 = "job collection";
                        x2 = "\tjob n.1 :\n\t=======================================\n";
                } else{
                        x1 = "DAG job" ;
                        x2 = "root's job" ;
                }
*/
        outmsg += "your " + x1 + " has been successfully submitted by the end point:\n\n";
        outmsg += "\t" + string(wmpEndPoint)+ "\n\n";
        outmsg += x2 + "\t" + jobid + "\n\n";
        if (debug){
                cout << x1 << "\n";
                cout << "(" << x2  << ") jobid : " << jobid  << endl;
                cout << "retrieving InputSanbox files ...." << endl;
        }
        // MAIN JOB: node name (if it is set)
        if ( jobIds.nodeName ){
                node =*jobIds.nodeName;
                if (node.size() > 0 ){
                        outmsg += "node: " + node + "\n\n";
                } else {
                        outmsg += "node name is unknown\n\n";
                }
        }
        if (!dag){
                fprintf (stderr, "fatal error: unable to read the DAG Job Description\n");
                exit(-1);
        }
        // MAIN JOB: InputSandox files
        if (dag->hasAttribute(JDL::INPUTSB)) {
                // MAIN JOB: http destination URI
                dest_uri = getInputSBDestURI(jobid);
                if (!dest_uri){
                        fprintf (stderr, "\nerror: unable to retrieve the InputSandbox destination URI\n");
                        fprintf (stderr, "(main job) jobid: %s", jobid.c_str() );
                        exit(-1);
                }
                paths = dag->getInputSandbox();
                if (debug){
                        cout << "this  job has " << paths.size() << " InputSB files" << endl;
                        cout << "destination URI = " << *dest_uri << "\n\n";
                }
                // MAIN JOB: InputSB files to be transferred to the DestURI
                toBcopied(JDL::INPUTSB, paths, to_bcopied, *dest_uri, "");
        } else {
                if (debug){
                        cout << "this job doesn't have any InputSB files" << endl;
                }
        }
                // CHILDREN ====================
        children = jobIds.children ;
         // Second part of the result message (children)
        if ( ! children.empty() ){
/*
                        if (jobtype == COLLECTION){
                                oss << ++n;
                                x1 = "\tjob n." + oss.str() + ":\n" ;
                                x2 = "\t=======================================\n";
                        } else {
                                x1 = "\tchildren";
                                x2 = "\t=======================================\n\n";
                        }
*/
		outmsg = outmsg.append (x1);
		outmsg = outmsg.append (x2);
                for ( it = children.begin() ; it != children.end(); it++){
                        child = *it;
                        if (child){
                                // child: jobid
                                jobid = Utils::checkJobId(child->jobid);
                                outmsg += "\tjobid\t: " + jobid + "\n";
                                if (debug){
                                        cout << x1 << x2 << "\n";
                                        cout << "assigned jobid : " << jobid  << endl;
                                        cout << "retrieving InputSanbox files ...." << endl;
                                }
                                //child:  node name
                                if ( ! child->nodeName ){
                                        cerr << "error:\nunable to retrieve the node name for child (" << jobid << ")\n";
                                        exit(-1);
                                }
                                node = *(child->nodeName);
                                if ( node != "" ){
                                        outmsg += "\tnode\t: " + node + "\n";
                                }
                                dest_uri = getInputSBDestURI(jobid);
                                if (!dest_uri){
                                        fprintf (stderr, "\nerror: unable to retrieve the InputSandbox destination URI\n");
                                        fprintf (stderr, "jobid: %s", jobid.c_str() );
                                        fprintf (stderr, "node: %s", node.c_str() );
                                        exit(-1);
                                }
                                // Children: InputSandbox files
                                paths = dag->getNodeStringValue(node, JDL::INPUTSB);
                                if (debug){
                                        cout << "this job has " << paths.size() << " InputSB files" << endl;
                                        cout << "destination URI = " << *dest_uri << "\n\n";
                                }
                                toBcopied(JDL::INPUTSB, paths, to_bcopied, *dest_uri,"");
                                outmsg += "\t-----------------------------------------------------------------\n";
                        }
                }
        } else {
                outmsg += "\nno chlidren\n\n";
        }
        // InputSandbox file transferring
        result = transferFiles (to_bcopied);

	// result output message
	if (result.size()>0){
		outmsg += result;
	}
	outmsg += "*************************************************************\n";
	if (debug){
		cout << "\nstarting the job : " << jobid << "\n";
	}
	// START
	jobStart(id, cfgCxt);
	if (debug){
		cout << "\n job start: success\n";
		cout << "the job has been successfully submitted\n";
	}
	// Displays the output message
	fprintf (stdout, "\n%s", outmsg.c_str());
        /*
	// saves the result
	if (fout){
		if ( ex_utilities::saveToFile(*fout, outmsg) < 0 ){
			fprintf (stderr, "\nwarning: unable to write the result in %s\n", fout->c_str());
		} else{
			fprintf (stdout, "\nThis message has been saved in %s\n", fout->c_str() );
		}
	}
        */
}
/*
*	performs the submission operations
*/
void JobSubmit::submission ( ){
	bool isdag = false;
	// ClassAd
	ad = new Ad( );

        if (! jdlFile){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Missing Information",
                        "uknown JDL file pathame"   );
        }
	ad->fromFile (*jdlFile);
	// DAG
        if ( ad->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
		isdag = true;
                dag = new  ExpDagAd ( ad->toString() );
                if ( debug ) {
                        cout << "\nyou have submitted a DAG job " << endl ;
                }
                // expands the DAG loading all JDL files
                dag->expand( );


                // .............

                jdlString = new string (ad->toString() );
        }  else{
        	jdlString = new string (ad->toString() );
        }

	if ( ! delegID){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to DelegationID String"   );
        }
	if (!cfgCxt){
		throw WmsClientException(__FILE__,__LINE__,
			"submission",  DEFAULT_ERR_CODE,
			"Null Pointer Error", "null pointer to ConfigContext object"   );
        }
	// Register
        jobIds = jobRegister (*jdlString, *delegID, cfgCxt);
	// handles the results
        if (isdag){
		dagJob( );
        } else{
		normalJob( );
        }

};

/*====================================
	private methods
==================================== */

/*
*	contacts the endpoint configurated in the context
*	in order to retrieve the http(s) destionationURI of the job
*	identified by the jobid
*/
std::string* JobSubmit::getInputSBDestURI(const std::string &jobid)
{
	string *dest_uri = NULL;
	vector <string> dest ;
	vector <string>::iterator it ;
	if (cfgCxt){
		dest = getSandboxDestURI (jobid, (ConfigContext *)cfgCxt);
		for (it = dest.begin() ; it != dest.end() ; it++){
			dest_uri = new string( *it );
			if ( dest_uri->substr (0, 4) == "http" ){
				break;
			} else {
				dest_uri = NULL;
			}
		}
		//input DestURI
		if (dest_uri){
			*dest_uri += "/input" ;
		}
	}
	return dest_uri;
}
/*
*	checks if the user free quota (on the server endpoint) could support (in size) the transferring of a set of files
*/
bool JobSubmit::checkFreeQuota ( std::vector<std::pair<std::string,std::string> > files, long &limit )
 {
	int hd = 0;
	char *path = "";
	struct stat file_info;
	FILE* fptr = NULL;
	vector<pair<string,string> >::iterator it ;
	bool not_exceed = true;
	// results of getFrewQuota
	pair<long, long> free_quota ;
	long size = 0;
	if (cfgCxt) {
		// gets the user free quota from the endpoint
		free_quota = getFreeQuota(cfgCxt);
		// soft limit
		limit = free_quota.first;
		// if the previous function gets back a negative number
		// the user free quota is not set on the server and
		// no check is performed (this functions returns not exceed=true)
		if ( limit  > 0 ) {
			// number of bytes to be transferred
			for ( it = files.begin() ;  it != files.end() ; it++ ) {
				// pathname
				path =(char*) checkFileExistence( (it->first).c_str());
				if ( !path) {
					throw WmsClientException(__FILE__,__LINE__,
						"checkFileExistence",  DEFAULT_ERR_CODE,
						"File Not Found", "no such file : " + it->first  );
				}
				// size of
				hd = open(path, O_RDONLY) ;
				fstat(hd, &file_info);
				close(hd) ;
				// adds up the size of the size
				size += file_info.st_size ;
				if ( size > limit) {
					not_exceed = false;
					fclose (fptr);
					break;
				}
				fclose (fptr);
			}
			if ( debug ) {
				cout << "\nuser free quota = " << limit;
				if ( not_exceed ){
					cout << " ok\n";
					cout << "\nfree quota is " << limit << "\nInputSandbox file size is " << size << " byte." << endl;
				} else{
					cout << "failed: over-quota (not enough user free quota to transfer the file(s) )\n";
				}
			}
		}
	}
	return not_exceed ;
 }
/*
std::vector<std::pair<string,string> > getInputSandboxFiles(Ad *ad){

	if (ad){
		 if (ad->hasAttribute(requestad::JDL::INPUTSB)){
			// http destination URI
			dest_uri = ex_utilities::getInputSBDestURI(jobid, cfs);
			if (!dest_uri){
				fprintf (stderr, "\nerror: unable to retrieve the InputSandbox destination URI\n");
				fprintf (stderr, "jobid: %s", jobid.c_str() );
				exit(-1);
			}
		if (debug){
			cout << "destination URI = " << *dest_uri << endl;
		}
		// InputSandbox files
		if (!ad){
			fprintf (stderr, "fatal error: unable to read the Job Description\n");
			exit(-1);
		}
		// gets the InputSandbox files
		paths = ad->getStringValue  (requestad::JDL::INPUTSB) ;
  	}
 }
 */
}}}} // ending namespaces

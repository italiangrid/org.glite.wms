/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpauthorizer.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

// added to build on IA64
#include <pwd.h>
#include <sys/types.h>

#include <openssl/pem.h>

#include "wmpauthorizer.h"
#include "wmpvomsauthz.h"

#ifndef GLITE_GACL_ADMIN

// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

#include "glite/wms/jdl/RequestAdExceptions.h"

// Utilities
#include "utilities/wmputils.h"

//Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "utilities/logging.h"


extern "C" {
	// LCMAPS C libraries headers
	#include "glite/security/lcmaps_without_gsi/lcmaps.h"
	#include "glite/security/lcmaps_without_gsi/lcmaps_return_poolindex_without_gsi.h"
}

#include <dlfcn.h>
#include "wmpgaclmanager.h"

#endif  // GLITE_GACL_ADMIN



#ifndef ALLOW_EMPTY_CREDENTIALS
#define ALLOW_EMPTY_CREDENTIALS
#endif

//Boost
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>


namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

using namespace std;

#ifndef GLITE_GACL_ADMIN

using namespace glite::wmsutils::exception; // Exception
using namespace glite::wms::wmproxy::utilities; //Exception

namespace logger       = glite::wms::common::logger;
namespace wmputilities = glite::wms::wmproxy::utilities;


/// Job Directories
const char* WMPAuthorizer::INPUT_SB_DIRECTORY = "input";
const char* WMPAuthorizer::OUTPUT_SB_DIRECTORY = "output";
const char* WMPAuthorizer::PEEK_DIRECTORY = "peek";

const char* WMPAuthorizer::DOCUMENT_ROOT = "DOCUMENT_ROOT";
const string WMPAuthorizer::VOMS_GACL_FILE = "glite_wms_wmproxy.gacl";
const char* WMPAuthorizer::VOMS_GACL_VAR = "GRST_CRED_2";

const string LCMAPS_LOG_FILE = "lcmaps.log";

// FQAN strings
const std::string FQAN_FIELDS[ ]  = { "vo", "group", "group", "role", "capability"};
const std::string FQAN_FIELD_SEPARATOR = "";
const std::string FQAN_NULL = "null";


WMPAuthorizer::WMPAuthorizer(char * lcmaps_logfile)
{
	edglog_fn("WMPAuthorizer::WMPAuthorizer");
	
	this->mapdone = false;
	this->certfqan = "";
	if (lcmaps_logfile) {
		this->lcmaps_logfile = lcmaps_logfile;
		edglog(debug)<<"LCMAPS log file: "<<string(lcmaps_logfile)<<endl;
	} else {
		this->lcmaps_logfile = (char*) malloc(1024);
		char * location = getenv("GLITE_WMS_LOCATION_VAR");
		if (!location) {
			location = getenv("GLITE_LOCATION_VAR");
		}
		string slocation = "";
		if (location && wmputilities::fileExists(string(location) + "/log/")) {
			slocation = string(location) + "/log/" + LCMAPS_LOG_FILE;
			sprintf(this->lcmaps_logfile, "%s", slocation.c_str());
		} else {
			slocation = "/tmp/" + LCMAPS_LOG_FILE;
			sprintf(this->lcmaps_logfile, "%s", slocation.c_str());
		}
		edglog(debug)<<"LCMAPS log file: "<<slocation<<endl;
	}
	
}

WMPAuthorizer::~WMPAuthorizer() throw()
{
	if (this->lcmaps_logfile ) {
		free(this->lcmaps_logfile);
	}
}
	
string
WMPAuthorizer::getUserName()
{
	GLITE_STACK_TRY("getUserName()");
	if (!mapdone) {
		mapUser(this->certfqan);
	}
	return this->username;
	GLITE_STACK_CATCH();
}

uid_t
WMPAuthorizer::getUserId()
{
	GLITE_STACK_TRY("getUserId()");
	if (!mapdone) {
		mapUser(this->certfqan);
	}
	return this->userid;
	GLITE_STACK_CATCH();
}

void 
WMPAuthorizer::authorize(const string &certfqan, const string & jobid)
{
	GLITE_STACK_TRY("authorize()");
	edglog_fn("WMPAuthorizer::authorize");
	
	if (jobid != "") {
		// Checking job owner
		string userproxypath = getJobDelegatedProxyPath(jobid);
		
		//TBD change it to do check depending on the server operation requested:
		//i.e. jobCancel check for write, getOutputFileList check for list/read
		string userdn = string(getUserDN());
		string gaclfile = getJobDirectoryPath(jobid) + "/"
			+ GaclManager::WMPGACL_DEFAULT_FILE;
		edglog(debug)<<"Job gacl file: "<<gaclfile<<endl;
		try {
			authorizer::GaclManager gaclmanager(gaclfile);
			if (!gaclmanager.checkAllowPermission(GaclManager::WMPGACL_PERSON_TYPE,
					userdn, GaclManager::WMPGACL_WRITE)) {
				edglog(info)<<"User not authorized to perform this operation"
					<<endl;
				throw AuthorizationException(__FILE__, __LINE__,
			    	"authorize()", wmputilities::WMS_AUTHZ_ERROR,
			    	"User not authorized to perform this operation");
			}
		} catch (Exception &ex) {
			if (ex.getCode() == WMS_GACL_ITEM_NOT_FOUND) {
				edglog(info)<<"Operation permitted only to job owner or "
					"authorized user"<<endl;
				throw AuthorizationException(__FILE__, __LINE__,
			    	"authorize()", wmputilities::WMS_AUTHZ_ERROR, 
			    	"Operation permitted only to job owner or authorized user");
			}
			throw ex;
		}
	}
	
	// VOMS Authorizing
	edglog(debug)<<"Delegated Proxy FQAN: "<<certfqan<<endl;
	edglog(debug)<<"Request's Proxy FQAN: "<<wmputilities::getEnvFQAN()<<endl;
	if (certfqan != "") {
		this->certfqan = certfqan;
		if (!compareFQAN(certfqan, wmputilities::getEnvFQAN())) {
			edglog(info)<<"VOMS FQAN Authorization: user not authorized"
				<<endl;
			throw AuthorizationException(__FILE__, __LINE__,
		    	"authorize()", wmputilities::WMS_AUTHZ_ERROR, 
		    	"VOMS FQAN Authorization: user not authorized");
		}
		// Gacl Authorizing
 		checkGaclUserAuthZ();
		
	} else { // Not a VOMS Proxy
		/*if (!compareDN(dn, wmputilities::getEnvDN())) {
			throw AuthorizationException(__FILE__, __LINE__,
		    	"authorize()", wmputilities::WMS_AUTHZ_ERROR, 
		    	"VOMS FQAN Authorization: user not authorized");
		}*/
		// Gacl Authorizing
 		checkGaclUserAuthZ();
	}
	
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::mapUser(const std::string &certfqan)
{
	GLITE_STACK_TRY("mapUser()");
	edglog_fn("WMPAuthorizer::mapUser");
	
	this->mapdone = false;
	
 	int retval;
 	struct passwd * user_info = NULL;
	char * user_dn = wmputilities::getUserDN();
	
	edglog(debug)<<"certfqan: "<<certfqan<<endl;
	setenv("LCMAPS_POLICY_NAME", "standard:voms", 1);

  	if (certfqan != "") {
		// Initialising structure
		if (this->lcmaps_logfile != "") {
			setenv("LCMAPS_LOG_FILE", this->lcmaps_logfile, 0);
		}
		lcmaps_account_info_t plcmaps_account;
	  	retval = lcmaps_account_info_init(&plcmaps_account);
	  	if (retval) {
	    	edglog(error)<<"LCMAPS info initialization failure"<<endl;
	    	throw AuthorizationException(__FILE__, __LINE__,
	    		"lcmaps_account_info_init()", wmputilities::WMS_USERMAP_ERROR,
	    		"LCMAPS info initialization failure");
	  	}
	  	
	  	// Send user mapping request to LCMAPS 
	  	//char * user_dn = wmputilities::getUserDN();
	  	int fqan_num = 1; // N.B. Considering only one FQAN inside the list
	  	char * fqan_list[1]; // N.B. Considering only one FQAN inside the list
		fqan_list[0] = const_cast<char*>(certfqan.c_str());
	  	edglog(debug)<<"Inserted fqan: "<<string(fqan_list[0])<<endl;
	  	
	  	retval = lcmaps_return_poolindex_without_gsi((char *)
	  		wmputilities::convertDNEMailAddress(user_dn).c_str(),
	  		fqan_list, fqan_num, &plcmaps_account);
	  	if (retval) {
        	retval = lcmaps_return_poolindex_without_gsi(user_dn, 
        		fqan_list, fqan_num, &plcmaps_account);
            if (retval) {
	    		edglog(info)<<"LCMAPS failed authorization: User "<<user_dn 
	    			<<" is not authorized"<<endl;
	    		throw AuthorizationException(__FILE__, __LINE__,
	        		"lcmaps_return_poolindex_without_gsi()",
	        		wmputilities::WMS_NOT_AUTHORIZED_USER,
	        		("LCMAPS failed to map user credential"));
	  		}
        }

	  	// Getting username from uid
	  	this->userid = plcmaps_account.uid;
		user_info = getpwuid(this->userid);
	  	if (user_info == NULL) {
	    	edglog(info)<<"LCMAPS could not find the username related to uid: "
	    		<<this->userid<<endl; 
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"getpwuidn()",
	        	wmputilities::WMS_USERMAP_ERROR,
	        	"LCMAPS could not find the username related to uid");
	  	}
	  	
	  	// Checking for mapped user group. The group of the assigned local user
	  	// MUST be different from the group of user running server
	  	if (user_info->pw_gid == getgid()) {
	  		edglog(info)<<"Mapping not allowed, mapped local user group equal "
	  			"to group of user running server"<<endl; 
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"mapUser()", wmputilities::WMS_USERMAP_ERROR,
	        	"Mapping not allowed, mapped local user group equal to group"
	        	" of user running server\n(please contact server administrator)");
	  	}
	  	
	  	// Setting value for username private member
	  	this->username = string(user_info->pw_name);
	  
	  	// Cleaning structure
	  	retval = lcmaps_account_info_clean(&plcmaps_account);
	  	if (retval) {
	    	edglog(error)<<"LCMAPS info clean failure"<<endl;
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"lcmaps_account_info_clean()", wmputilities::WMS_USERMAP_ERROR,
	        	"LCMAPS info clean failure");
	  	}
	  	
	} else { // certfqan == "" -> authorize using DN
	  	int npols = 0; // this makes LCMAPS check all policies
	  	char * request = NULL;
	  	char ** namep = (char**) malloc(sizeof(char*));
	  	char ** policynames = NULL; // this makes LCMAPS check all policies
	  	gss_cred_id_t user_cred_handle = GSS_C_NO_CREDENTIAL;
	  	
		edglog(info)<<"LCMAPS log file: " <<this->lcmaps_logfile<<user_dn;
		FILE * logfile = fopen(this->lcmaps_logfile, "a");
	  	retval = lcmaps_init(logfile);
	  	if (retval) {
	    	edglog(error)<<"LCMAPS initialization failure"<<endl;
	    	throw AuthorizationException(__FILE__, __LINE__,
	    		"lcmaps_init()", wmputilities::WMS_USERMAP_ERROR,
	    		"LCMAPS initialization failure");
	  	}
		
	  	// Send user mapping request to LCMAPS 
	  	//retval = lcmaps_run_and_return_username(user_dn, user_cred_handle, request, 
	  	//	namep, npols, policynames);
	  	retval = lcmaps_run_and_return_username((char *)
	  		wmputilities::convertDNEMailAddress(user_dn).c_str(),
	  		user_cred_handle, request, namep, npols, policynames);
	 
	  	if (retval) {
        	retval = lcmaps_run_and_return_username(user_dn,
        		user_cred_handle, request, namep, npols, policynames);
            if (retval) {
	    		edglog(info)<<"LCMAPS failed authorization: User "<<user_dn
	    			<<" is not authorized"<<endl;
	    		throw AuthorizationException(__FILE__, __LINE__,
	        		"lcmaps_run_and_return_username()",
	        		wmputilities::WMS_NOT_AUTHORIZED_USER,
	        		("LCMAPS failed to map user credential"));
	  		}
        }
	  
	  	// Setting value for username private member
	  	this->username = string(*namep);
	  
	  	// Get uid from username
	  	user_info = getpwnam(*namep);
	  	if ( user_info == NULL ) {
	    	edglog(info)<<"LCMAPS could not find the uid related to username: "
	    		<<*namep<<endl; 
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"getpwnam()", wmputilities::WMS_USERMAP_ERROR,
	        	"LCMAPS could not find the uid related to username");
	  	}
	  	
	  	// Checking for mapped user group. The group of the assigned local user
	  	// MUST be different from the group of user running server
	  	if (user_info->pw_gid == getgid()) {
	  		edglog(info)<<"Mapping not allowed, mapped local user group equal "
	  			"to group of user running server"<<endl; 
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"mapUser()", wmputilities::WMS_USERMAP_ERROR,
	        	"Mapping not allowed, mapped local user group equal to group"
	        	" of user running server\n(please contact server administrator)");
	  	}
	  
	  	// Terminate the LCMAPS
	  	retval = lcmaps_term();
	  	if (retval) {
	    	edglog(error)<<"LCMAPS termination failure"<<endl;
	    	throw AuthorizationException(__FILE__, __LINE__,
	        	"lcmaps_term()", wmputilities::WMS_USERMAP_ERROR,
	        	"LCMAPS termination failure.");
	  	}
	  	
	  	fclose(logfile);

	  	// Setting value for userid private member
	  	this->userid = user_info->pw_uid;
	}

  	this->mapdone = true;
  	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::checkGaclUserAuthZ()
{
	GLITE_STACK_TRY("checkGaclUserAuthZ()");
	edglog_fn("WMPAuthorizer::checkGaclUserAuthZ");
	
	char *grst_cred = NULL;
	string fqan = "";
	string errmsg = "";
	bool exec = true;
	bool execDN = true;
	int pos = 0;
	edglog(debug)<<"Checking for VOMS credential..."<<endl;
	grst_cred = getenv ( VOMS_GACL_VAR );
	if ( grst_cred ){
		edglog(info)<<"Checking VOMS proxy..."<<endl;
		fqan = fqan.assign(grst_cred);
		pos = fqan.find("/") ;
		if (fqan.find("VOMS") == 0 && pos > 0) {
			fqan = fqan.erase(0, pos);
		}
	} else {
		edglog(warning)<<"Unknown voms fqan: "<<VOMS_GACL_VAR
			<<" environment variable not set"<<endl;
		fqan = "";
	}
	edglog(info)<<"fqan="<<fqan<<endl;
	string dn = string(wmputilities::getUserDN());
	edglog(info)<<"dn="<<dn<<endl;
	// Gacl-Mgr
	try {
		string gaclfile;
		if (getenv("GLITE_WMS_LOCATION")) {
			gaclfile = string(getenv("GLITE_WMS_LOCATION")) + "/etc/"
				+ WMPAuthorizer::VOMS_GACL_FILE;
		} else {
			if (getenv("GLITE_LOCATION")) {
				gaclfile = string(getenv("GLITE_LOCATION")) + "/etc/"
					+ WMPAuthorizer::VOMS_GACL_FILE;
			} else {
				gaclfile = "/opt/glite/etc/" + WMPAuthorizer::VOMS_GACL_FILE;
			}
		}
		GaclManager gacl(gaclfile);
		// checks exec permssion
		if (fqan != "") {
			// FQAN authorization
			exec = gacl.checkAllowPermission(GaclManager::WMPGACL_VOMS_TYPE,
				fqan, GaclManager::WMPGACL_EXEC);
			if (gacl.hasEntry(authorizer::GaclManager::WMPGACL_PERSON_TYPE, dn))  {
				execDN = gacl.checkAllowPermission(GaclManager::WMPGACL_PERSON_TYPE,
                                    dn,     GaclManager::WMPGACL_EXEC);
			 } else execDN = true ;
		} else {
			exec = true;
			// DN authorization
			execDN = gacl.checkAllowPermission(GaclManager::WMPGACL_PERSON_TYPE,
				dn,	GaclManager::WMPGACL_EXEC );
		}
		exec = exec && execDN;
	} catch (GaclException &exc){
			errmsg = "User not authorized:\n";
			errmsg += exc.what();
			edglog(critical)<<errmsg<<endl;
		throw GaclException(__FILE__, __LINE__,
			"checkGaclUserAuthZ()",
			wmputilities::WMS_GACL_FILE,
			errmsg);
	}
	// checks exec permission
	if (!exec) {
		edglog(info)<<"Authorization error: user not authorized"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
			"checkGaclUserAuthZ()",
			wmputilities::WMS_AUTHZ_ERROR, "Authorization error: "
			"user not authorized");
	}
	
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::setJobGacl(vector<string> &jobids)
{
	GLITE_STACK_TRY("setJobGacl()");
	edglog_fn("WMPAuthorizer::setJobGacl vector");
	
	unsigned int size = jobids.size();
	if (size != 0) {
		string user_dn = wmputilities::getUserDN();
		string errmsg = "";
		
		// Creates a gacl file in the job directory
		authorizer::WMPgaclPerm permission =
			authorizer::GaclManager::WMPGACL_READ |
			authorizer::GaclManager::WMPGACL_LIST |
			authorizer::GaclManager::WMPGACL_WRITE |
			authorizer::GaclManager::WMPGACL_READ;
	 	
		// main user job directory
		string filename = wmputilities::getJobDirectoryPath(jobids[0]) + "/" 
			+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE;
		
		try {
			authorizer::GaclManager gacl(filename, true);
			// adds the new user credential entry
			gacl.addEntry(authorizer::GaclManager::WMPGACL_PERSON_TYPE, user_dn);
			// allow permission
			gacl.allowPermission(authorizer::GaclManager::WMPGACL_PERSON_TYPE,
					user_dn, permission);
			gacl.saveGacl();
		} catch (GaclException &exc) {
			errmsg = "internal server error: unable to set the gacl user properties";
			errmsg += "\n(please contact server administrator)\n";
			errmsg += "please report the following message:\n" ;
			errmsg += exc.what();
			edglog(critical)<<errmsg<<endl;
			throw GaclException(__FILE__, __LINE__, "setJobGacl()",
				wmputilities::WMS_GACL_FILE, errmsg);
		}
		ifstream infile(filename.c_str());
		if (!infile.good()) {
			edglog(severe)<<filename<<": !filename.good()"<<endl;
			throw FileSystemException(__FILE__, __LINE__,
				"setJobGacl()", WMS_IS_FAILURE, "Unable to open gacl "
				"input file\n(please contact server administrator)");
		}
		string gacltext = "";
		string s;
		while (getline(infile, s, '\n')) {
			gacltext += s + "\n";
		}
		infile.close();
		
		fstream outfile;
		for (unsigned int i = 1; i < size; i++) {
			filename = wmputilities::getJobDirectoryPath(jobids[i]) + "/"
				+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE;
			outfile.open(filename.c_str(), ios::out);
			if (!outfile.good()) {
				edglog(severe)<<outfile<<": !outfile.good()"<<endl;
				throw FileSystemException(__FILE__, __LINE__,
					"setJobGacl()", WMS_IS_FAILURE, "Unable to open gacl "
					"output file\n(please contact server administrator)");
			}
			outfile<<gacltext;
			outfile.close();
		}
	}
	
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::setJobGacl(const string &jobid)
{
	GLITE_STACK_TRY("setJobGacl()");
	edglog_fn("WMPAuthorizer::setJobGacl string");
	
	string user_dn = wmputilities::getUserDN();
	string errmsg = "";
	
	// Creates a gacl file in the job directory
	authorizer::WMPgaclPerm permission =
		authorizer::GaclManager::WMPGACL_READ |
		authorizer::GaclManager::WMPGACL_LIST |
		authorizer::GaclManager::WMPGACL_WRITE |
		authorizer::GaclManager::WMPGACL_READ;
 	
	// main user job directory
	string filename = wmputilities::getJobDirectoryPath(jobid) + "/"
		+ authorizer::GaclManager::WMPGACL_DEFAULT_FILE;
	
	try {
		authorizer::GaclManager gacl(filename, true);
		// adds the new user credential entry
		gacl.addEntry(authorizer::GaclManager::WMPGACL_PERSON_TYPE, user_dn);
		// allow permission
		gacl.allowPermission( authorizer::GaclManager::WMPGACL_PERSON_TYPE,
				user_dn, permission);
		gacl.saveGacl( );
	} catch (GaclException &exc) {
		errmsg = "internal server error: unable to set the gacl user properties  ";
		errmsg += " (please contact server administrator)\n";
		errmsg += "please report the following message:\n" ;
		errmsg += exc.what ( );
		edglog(critical)<<errmsg<<endl;
		throw GaclException(__FILE__, __LINE__, "setJobGacl()",
			wmputilities::WMS_GACL_FILE, errmsg);
	}
		
	GLITE_STACK_CATCH();
}

bool 
WMPAuthorizer::checkJobDrain()
{
	GLITE_STACK_TRY("checkJobDrain");
	edglog_fn("WMPAuthorizer::checkJobDrain");

	bool exec = true;
	string drain_file = "";
	string errmsg = "";
	char* doc_root = getenv (DOCUMENT_ROOT);

	if (doc_root){
		// gacl file: path location
		drain_file = drain_file.assign(doc_root).append("/")
			.append(authorizer::GaclManager::WMPGACL_DEFAULT_DRAIN_FILE);
		edglog(debug) <<"checkJobDrain> drain_file = "<<drain_file<<endl;
		// the drain is ony checked if the gacl file exists (if it doesn't no exception is thrown)
		if (utilities::fileExists(drain_file)){
			authorizer::GaclManager gacl(drain_file) ;
			if (gacl.hasEntry(authorizer::GaclManager::WMPGACL_ANYUSER_TYPE)){
				exec = gacl.checkAllowPermission(
					authorizer::GaclManager::WMPGACL_ANYUSER_TYPE,
					authorizer::GaclManager::WMPGACL_ANYUSER_CRED,
					authorizer::GaclManager::WMPGACL_EXEC);
			}
		}
	} else {
		string msg = "Internal server error: information on the document root "
			"location is not available (please contact server administrator)";
		edglog(critical)<<msg<<endl;
		throw GaclException (__FILE__, __LINE__, "checkJobDrain()" ,
			wmputilities::WMS_GACL_ERROR, msg);
	}
	return (!exec);
	
	GLITE_STACK_CATCH();
}
#endif //GLITE_GACL_ADMIN

bool
WMPAuthorizer::isNull(string field)
{
	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_TRY("isNull");
	#endif

	bool is_null = false ;
	int p1 = field.size() - 5 ;
	int p2 = field.find ("=NULL");

	if ( p1 > 0 &&  p1==p2 ){
		is_null= true;
	}
	//edglog(debug)<<"is_null="<<is_null<<endl;
	return is_null;

	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_CATCH();
	#endif
}

/**
 *      /VO [ /group [ /subgroup (s) ]  ]  [ /Role = Role_Value ] [ /Capability = Capability_Value]
 */
std::vector<std::pair<std::string,std::string> >
WMPAuthorizer::parseFQAN(const std::string &fqan)
{
	#ifndef GLITE_GACL_ADMIN
        GLITE_STACK_TRY("parseFQAN");
        #endif

        vector<string> tokens;
        std::vector<std::pair<std::string,std::string> > vect;
        string label = "";
        string value = "";
        int nt = 0;
        boost::char_separator<char> separator("/");
        boost::tokenizer<boost::char_separator<char> >
        tok(fqan, separator);
        boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin();
        boost::tokenizer<boost::char_separator<char> >::iterator const end = tok.end();
        for (  ; it != end; it++) {
                tokens.push_back(string(*it));
        }
        // number of found tokens
        nt = tokens.size();
        if (nt > 0) {
                // Checks the VO field
                split(tokens[0], label, value);
                if ((label.size() > 0 || value.size() >0) &&
                         ((label.compare(FQAN_FIELDS[FQAN_ROLE])==0) ||
                         (label.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0)) ){
                                throw AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  WMS_PROXY_ERROR,
                        "malformed fqan (VO field is missing): " +  fqan);
                } else {
                        vect.push_back(make_pair(FQAN_FIELDS[FQAN_VO], tokens[0]));
                        tokens.erase(tokens.begin());
                }
                // Checks group and subgroups fields (if present)
                while (tokens.empty()==false) {
                        split(tokens[0], label, value);
                        if (label.size()==0) {
                                vect.push_back(make_pair(FQAN_FIELDS[FQAN_GROUP], tokens[0]));
                                tokens.erase(tokens.begin());
                        } else {
                                break;
                        }
                }
                // Checks Role & Capability
                if (tokens.empty() == false) {
                        // either Role or Capability is expected
                        split(tokens[0], label, value);
                        // Checks whether the fqan contains the  ROLE field
                        if (label.compare(FQAN_FIELDS[FQAN_ROLE])==0) {
                                if (value.size()==0) {
                                        throw AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  WMS_PROXY_ERROR,
                                         "malformed FQAN field /" + tokens[0] );
                                }
                                // Role is present (and the value is not "null")
                                if (value.compare(FQAN_NULL) != 0){
                                        vect.push_back(make_pair(label, value));
                                }
                                tokens.erase(tokens.begin());
                                if (tokens.empty()==false) {
                                        // This token has to contain only the Capability field
                                        split(tokens[0], label, value);
                                } else {
                                        label = "";
                                        value = "";
                                }
                        }
                        // No other tokes must be present
                        if (tokens.size() > 1) {
                                nt = tokens.size();
                                ostringstream err;
                                err << "malformed FQAN field; one or more field are invalid :\n";                                               for (int i = 0; i < nt ; i++ ){
                                        err << "/" << tokens[i] << "\n";
                                }
                                throw AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  WMS_PROXY_ERROR,
                                        err.str() );
                        }

                        if (label.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0) {
                                if (value.size()==0) {
                                throw AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  WMS_PROXY_ERROR,
                                        "malformed FQAN field (/" + tokens[0] );
                                }
                                // Capability is present (and the value is not "null")
                                if (value.compare(FQAN_NULL) != 0){
                                        vect.push_back(make_pair(label, value));
                                }
                                tokens.erase(tokens.begin());

                        } else {
                                if (label.size()>0 || value.size()>0) {
                                        throw AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  WMS_PROXY_ERROR,
                                        "malformed FQAN field; invalid Capability field: /" + tokens[0] );

                                }
                        }
                }
        } else {
                throw AuthorizationException(__FILE__, __LINE__,
                        "parseFQAN(string)",  WMS_PROXY_ERROR,
                                "invalid fqan: " + fqan  );;
        }
        return vect;
	#ifndef GLITE_GACL_ADMIN
        GLITE_STACK_CATCH();
        #endif


}

bool
WMPAuthorizer::compareDN(char * dn1, char * dn2)
{
	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_TRY("compareDN");
	#endif

	int ret;
	char * aa = NULL;
   	char * bb = NULL;
   	char * p = NULL;

   	aa = strdup(dn1);
   	while ((p = strstr(aa, "/emailAddress=")) != NULL) {
   		memmove(&p[6], &p[13], strlen(&p[13]) + 1);
      	p[1] = 'E';
    }

   	bb = strdup(dn2);
   	while ((p = strstr(bb, "/emailAddress=")) != NULL) {
    	memmove(&p[6], &p[13], strlen(&p[13]) + 1);
        p[1] = 'E';
    }

	ret = strcmp(aa, bb);

   	free(aa);
   	free(bb);

   	return ret;
   	#ifndef GLITE_GACL_ADMIN
   	GLITE_STACK_CATCH();
	#endif

}
bool
WMPAuthorizer::compareFQAN (const string &ref, const string &in )
{
	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_TRY("compareFQAN");
	edglog_fn("WMPAuthorizer::compareFQAN");
	#endif
        bool match = true;

        vector<pair<string,string> > vect_ref, vect_in;
	string lab_ref = "";
	string lab_in = "";
	string val_ref = "";
	string val_in = "";

        // the vectors contain pairs like this <label,value> (label may be an empty string)
        vect_ref = parseFQAN(ref );
	if ( vect_ref.empty()){
                throw AuthorizationException(__FILE__, __LINE__,
                        "compareFQAN(string, string)", wmputilities::WMS_AUTHZ_ERROR,
                        "no valid fields in the FQAN string: [" + ref + "] (please contact the server administrator");

	}
        // vin=<input-vect>
        vect_in = parseFQAN(in);

	if (vect_in.empty()) {
                throw AuthorizationException(__FILE__, __LINE__,
                        "compareFQAN(string, string)", wmputilities::WMS_AUTHZ_ERROR,
                        "no valid fields in the user FQAN string: [" + in+ "]");
	}
	// Compare VO's==================
	val_ref = vect_ref[0].second;
	val_in = vect_in[0].second;
	if (val_ref.compare(val_in) != 0){
		match = false;
	}
	vect_ref.erase(vect_ref.begin());
	vect_in.erase(vect_in.begin());
	//Checks Group and SubGroup(s)
	while (vect_ref.empty()==false && match) {
		if (vect_in.empty()) {
			// ref-FQQAN contains other fields
			// in-FQAN does not
			match = false;
			break;
		} else {
			lab_ref = vect_ref[0].first;
			if (lab_ref.compare(FQAN_FIELDS[FQAN_GROUP])==0 ) {
				lab_in = vect_in[0].first;
				//cout << "####compareFQANs> A) lab_in = " << lab_in << "\n";
				// ref-FQAN contains a group(or subgroup) field
				if (lab_in.compare(FQAN_FIELDS[FQAN_GROUP])==0){

					val_ref = vect_ref[0].second;
					val_in = vect_in[0].second;
					if (val_ref.compare(val_in)==0) {
						//cout << "####compareFQANs> match OK\n";
						// match=OK
						vect_ref.erase(vect_ref.begin());
						vect_in.erase(vect_in.begin());
					} else {
						// different groups !
						match = false;
						break;
					}
				} else {
					// in-FQAN does not contain a group field in the same position
					match = false;
					break;
				}
			//	cout << "####compareFQANs> GROUPS (0) - match = " << match << "\n";
			} else {
				// ref-FQAN has one or two fields (Role or/and Capability)
				// in-FQAN : removal of the Other groups(subgroups) which must be ignored
				lab_in = vect_in[0].first;
				//cout << "####compareFQANs> B) lab_in = " << lab_in << "\n";
				if (lab_in.compare(FQAN_FIELDS[FQAN_GROUP])==0) {
					// ref-FQAN has one or two fields (Role or/and Capability)
					// in-FQAN has other group/subgroup field
					match = false;
				}
				break;
			}
		}
	}
	//Checks Role
	if (vect_ref.empty()==false && match) {
		if (vect_in.empty()) {
			// ref-FQQAN contains other fields
			// in-FQAN does not
			match = false;
		} else {
			lab_ref = vect_ref[0].first;
			if (lab_ref.compare(FQAN_FIELDS[FQAN_ROLE])==0 ) {
				lab_in = vect_in[0].first;
				// ref-FQAN contains a Role field
				if (lab_in.compare(FQAN_FIELDS[FQAN_ROLE])==0){
					val_ref = vect_ref[0].second;
					val_in = vect_in[0].second;
					if (val_ref.compare(val_in)==0) {
						// removal of the fields just checked
						vect_ref.erase(vect_ref.begin());
						vect_in.erase(vect_in.begin());
					} else {
						// different roles !
						match = false;
					}
				} else {
					// ref-FQAN contains Role field
					// in-FQAN does not
					match = false;
				}
			}
		}
	}
	//Checks Capability
	if (vect_ref.empty()==false && match) {
		lab_ref = vect_ref[0].first;
		if (lab_ref.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0 ) {
			lab_in = vect_in[0].first;
			// ref-FQAN contains a Role field
			if (lab_in.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0){
				val_ref = vect_ref[0].second;
				val_in = vect_in[0].second;
				if (val_ref.compare(val_in)==0) {
					// removal of the fields just checked
					vect_ref.erase(vect_ref.begin());
					vect_in.erase(vect_in.begin());
				} else {
					// different capabilities !
					match = false;

				}
			} else {
				// ref-FQAN contains Capablity field
				// in-FQAN does not
				match = false;
			}

		}
	}
        return match ;
	#ifndef GLITE_GACL_ADMIN
   	GLITE_STACK_CATCH();
	#endif
}




// private method: converts ASN1_UTCTIME to time_t
time_t 
ASN1_UTCTIME_get(const ASN1_UTCTIME *s)
{
	struct tm tm;
    int offset;

	//struct tm *lt = localtime(&t);
    memset(&tm,'\0',sizeof tm);
#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
    tm.tm_year=g2(s->data);
    if (tm.tm_year < 50) {
    	tm.tm_year+=100;
    }
    tm.tm_mon=g2(s->data+2)-1;
    tm.tm_mday=g2(s->data+4);
    tm.tm_hour=g2(s->data+6);
    tm.tm_min=g2(s->data+8);
    tm.tm_sec=g2(s->data+10);
    if (s->data[12] == 'Z') {
    	offset=0;
    } else {
	    offset=g2(s->data+13)*60+g2(s->data+15);
    	if (s->data[12] == '-') {
            offset= -offset;
    	}
    }
#undef g2

    return timegm(&tm)-offset*60;
}

#ifndef GLITE_GACL_ADMIN

const long 
WMPAuthorizer::getProxyTimeLeft(const string &pxfile)
{
	GLITE_STACK_TRY("getProxyTimeLeft");
	edglog_fn("WMPAuthorizer::getProxyTimeLeft");
	
	time_t timeleft = 0;
	X509 *x = NULL;
	BIO *in = NULL;
	in = BIO_new(BIO_s_file());
	if (in) {
		BIO_set_close(in, BIO_CLOSE);
		if (BIO_read_filename(in, pxfile.c_str() ) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if (!x) {
				BIO_free(in);
        		edglog(severe)<<"Error in PEM_read_bio_X509: Proxy file "
        			"doesn't exist or has bad permissions"<<endl;
        		throw AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::getProxyTimeLeft", wmputilities::WMS_AUTHZ_ERROR,
			    	"Proxy file doesn't exist or has bad permissions");
      		}
			timeleft = (ASN1_UTCTIME_get(X509_get_notAfter(x)) - time(NULL))
				/ 60;
			free(x);
		} else {
			BIO_free(in);
			edglog(error)<<"Unable to get the proxy time left"<<endl;
			throw ProxyOperationException(__FILE__, __LINE__,
				"BIO_read_filename", wmputilities::WMS_PROXY_ERROR,
				"Unable to get the proxy time left");
		}
		BIO_free(in);
	} else {
		edglog(error)<<"Unable to get the proxy time left (BIO SSL error)"<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"BIO_new", wmputilities::WMS_PROXY_ERROR,
			"Unable to get the proxy time left (BIO SSL error)");

	}
	return timeleft;
	
	GLITE_STACK_CATCH();
}


const long 
WMPAuthorizer::getNotBefore(const string &pxfile)
{
	GLITE_STACK_TRY("getNotBefore()");
	edglog_fn("WMPAuthorizer::getNotBefore");
	
	long sec = 0;
	X509 *x = NULL;
	BIO *in = NULL;
	in = BIO_new(BIO_s_file());
	if (in) {
		BIO_set_close(in, BIO_CLOSE);
		if (BIO_read_filename(in, pxfile.c_str()) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if (!x) {
				BIO_free(in);
        		edglog(severe)<<"Error in PEM_read_bio_X509: Proxy file "
        			"doesn't exist or has bad permissions"<<endl;
        		throw AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::getProxyTimeLeft", wmputilities::WMS_AUTHZ_ERROR,
			    	"Proxy file doesn't exist or has bad permissions");
      		}
			sec = ASN1_UTCTIME_get(X509_get_notBefore(x));
			free(x);
		} else {
			BIO_free(in);
			edglog(error)<<"Unable to get Not Before date from Proxy"<<endl;
			throw ProxyOperationException(__FILE__, __LINE__,
				"getNotBefore()", wmputilities::WMS_PROXY_ERROR,
				"Unable to get Not Before date from Proxy");
		}
		BIO_free(in);
	} else {
		edglog(error)<<"Unable to get Not Before date from Proxy (BIO SSL error)"
			<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"getNotBefore()", wmputilities::WMS_PROXY_ERROR,
			"Unable to get Not Before date from Proxy (BIO SSL error)");
	}
	return sec;
	
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::checkProxy(const string &proxy)
{
	GLITE_STACK_TRY("checkProxy()");
	edglog_fn("WMPAuthorizer::checkProxy");

	edglog(debug)<<"time(NULL): "<<
		boost::lexical_cast<std::string>(time(NULL))<<endl;
	edglog(debug)<<"Not Before: "<<
		boost::lexical_cast<std::string>(getNotBefore(proxy))<<endl;
		
	if (time(NULL) < getNotBefore(proxy)) {
		edglog(info)<<"Proxy validity starting time in the future"<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"checkProxy()", wmputilities::WMS_PROXY_ERROR,
			"Proxy validity starting time in the future"
			"\nPlease check client date/time");
	}
	
	long timeleft = getProxyTimeLeft(proxy);
	edglog(debug)<<"Time Left: "<<timeleft<<endl;

	if (timeleft <= 1) {
		edglog(info)<<"The delegated Proxy has expired"<<endl;
		throw ProxyOperationException(__FILE__, __LINE__,
			"checkProxy()", wmputilities::WMS_PROXY_EXPIRED,
			"The delegated Proxy has expired");
	}
	
	GLITE_STACK_CATCH();

}

void
WMPAuthorizer::checkProxyExistence(const string &userproxypath, const string &jobid)
{
	GLITE_STACK_TRY("checkProxyExistence()");
	edglog_fn("WMPAuthorizer::checkProxyExistence");
	
	string userproxypathbak = getJobDelegatedProxyPathBak(jobid);
	if (!wmputilities::fileExists(userproxypath)) {
		if (!wmputilities::fileExists(userproxypathbak)) {
			edglog(error)<<"The job has not been registered from this Workload "
				"Manager Proxy server (Proxy file not found)"<<endl;
			throw JobOperationException(__FILE__, __LINE__,
				"checkProxyExistence()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"The job has not been registered from this Workload Manager Proxy "
				"server (" + wmputilities::getEndpoint() + ") or it has been purged");
		} else {
			unlink(userproxypath.c_str());
			wmputilities::fileCopy(userproxypathbak, userproxypath);
		}
	} else {
		wmputilities::fileCopy(userproxypath, userproxypathbak);
	}
	
	GLITE_STACK_CATCH();
}
#endif


} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

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
	  	char * user_dn = wmputilities::getUserDN();
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
				execDN = !gacl.checkDenyPermission(GaclManager::WMPGACL_PERSON_TYPE,
					dn,	GaclManager::WMPGACL_EXEC);
			} else {
				// DN authorization
				execDN = gacl.checkAllowPermission(GaclManager::WMPGACL_PERSON_TYPE,
					dn,	GaclManager::WMPGACL_EXEC );
			}
			exec = exec && execDN;
		} catch (GaclException &exc){
				errmsg = "Internal server error: missing information on authorization";
				errmsg += "\n(please contact server administrator)\n";
				errmsg += "please report the following message:\n";
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

	} else {
		edglog(warning)<< "unknown voms fqan: " << VOMS_GACL_VAR << " environment variable not set" << endl;
	}
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::setJobGacl(string jobid )
{
	GLITE_STACK_TRY("setJobGacl()");
	edglog_fn("WMPAuthorizer::setJobGacl");
	
	vector<string> gacl_files;
	vector<string>::iterator it ;
	string errmsg = "";
	// Creates a gacl file in the job directory
	authorizer::WMPgaclPerm permission = authorizer::GaclManager::WMPGACL_READ |
									authorizer::GaclManager::WMPGACL_LIST |
									authorizer::GaclManager::WMPGACL_WRITE |
									authorizer::GaclManager::WMPGACL_READ;
 	string user_dn = wmputilities::getUserDN() ;
	edglog(debug)<<"userDN: "<<user_dn<<endl;

	// main user job directory
	string job_dir = wmputilities::getJobDirectoryPath(jobid) ;
	// paths of the gacl files
	gacl_files.push_back(job_dir + "/" + authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	gacl_files.push_back(job_dir + "/" + INPUT_SB_DIRECTORY + "/" + authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	gacl_files.push_back(job_dir + "/" + OUTPUT_SB_DIRECTORY + "/" + authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	gacl_files.push_back(job_dir + "/" + PEEK_DIRECTORY + "/" + authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
	// creates the gacl files
	for (it = gacl_files.begin(); it != gacl_files.end(); it++) {
		try {
			authorizer::GaclManager gacl(*it, true);
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
			throw GaclException(__FILE__, __LINE__,
				"setJobGacl()",
				wmputilities::WMS_GACL_FILE,
				errmsg);
		}
	}
	GLITE_STACK_CATCH();
}

bool 
WMPAuthorizer::checkJobDrain()
{
	GLITE_STACK_TRY("checkJobDrain");
	edglog_fn("WMPAuthorizer::checkJobDrain");
	
	bool exec = true;
	string gacl_file = "";
	string errmsg = "";
	char* doc_root = getenv (DOCUMENT_ROOT);

	if (doc_root){
		// gacl file: path location
		gacl_file = gacl_file.assign(doc_root).append("/")
			.append(authorizer::GaclManager::WMPGACL_DEFAULT_FILE);
		edglog(debug) <<"checkJobDrain> gacl_file = "<<gacl_file<<endl;
		// the drain is ony checked if the gacl file exists (if it doesn't no exception is thrown)
		if (utilities::fileExists(gacl_file)){
			authorizer::GaclManager gacl(gacl_file) ;
			exec = gacl.checkAllowPermission(
				authorizer::GaclManager::WMPGACL_ANYUSER_TYPE,
				authorizer::GaclManager::WMPGACL_ANYUSER_CRED,
				authorizer::GaclManager::WMPGACL_EXEC);
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

vector<string>
WMPAuthorizer::parseFQAN(const string &fqan)
{
	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_TRY("parseFQAN");
	#endif
	
	vector<string> returnvector;
	string field ;
	boost::char_separator<char> separator("/");
	boost::tokenizer<boost::char_separator<char> >
    	tok(fqan, separator);
   	 for (boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
		token != tok.end(); token++) {
    		returnvector.push_back(*token);
	}
	return returnvector;

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
WMPAuthorizer::compareFQAN(const string &ref, const string &in )
{
	#ifndef GLITE_GACL_ADMIN
	GLITE_STACK_TRY("compareFQAN");
	edglog_fn("WMPAuthorizer::compareFQAN");
	#endif
	bool match = true;
	vector<string> v1, v2;
	vector<string>::iterator it1, it2, it3 ;

	// v1=<referring-vect>
	v1= authorizer::WMPAuthorizer::parseFQAN( ref );
	// v2=<input-vect>
	v2= authorizer::WMPAuthorizer::parseFQAN( in );

	for ( it1 = v1.begin() ; it1 != v2.end( ) ; it1++ ) {
		// checks if  the <input-vect> is empty
		if ( v2.empty( ) ) {
			// checks if the remaining elements of the <ref-vect> are NULL
			for ( it3 = it1; it3 != v1.end() ; it3++ ) {
				if ( ! isNull ( *it3 ) ) {
					match = false;
					break;
				}
			} // for (it3)
			break;
		} else {
			// compares the fqan fields
			it2 = v2.begin( );
			//edglog(debug)<<"ref=["<<*it1<<"]-in["<<*it2<<"]"<<endl;
			if ( *it1 != *it2 &&  ! isNull(*it1) ) {
				match = false;
				break;
			}
			v2.erase( it2 );
		} // if (v2 empty)
	} // for(it1)
	//edglog(debug)<<"match="<<match<<endl;
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

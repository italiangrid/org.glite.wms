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
limitations under the License. */

#include <iostream>
#include <vector>
#include <string>
#include <pwd.h> // to build on IA64
#include <sys/types.h>
#include <dlfcn.h>

#include <openssl/pem.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/tuple/tuple.hpp"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMPConfiguration.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/logger/edglog.h"
#include "utilities/logging.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "wmpgaclmanager.h"
#include "wmpvomsauthn.h"
#include "wmpdelegation.h"
#include "utilities/wmputils.h"
#include "wmpauthorizer.h"
#include "argus_authz.h"

extern "C" {
#include "lcmaps/lcmaps.h"
#include "lcmaps/lcmaps_return_poolindex_without_gsi.h"
}

namespace wmputilities = glite::wms::wmproxy::utilities;
namespace logger = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;

using namespace std;
using namespace glite::wmsutils::exception;

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

namespace {

#define DOCUMENT_ROOT "DOCUMENT_ROOT"
std::string const GACL_FILE("glite_wms_wmproxy.gacl");

// FQAN strings
const std::string FQAN_FIELDS[ ]  = { "vo", "group", "group", "role", "capability"};
const std::string FQAN_FIELD_SEPARATOR = "";
const std::string FQAN_NULL = "null";

/**
 *      /VO [ /group [ /subgroup (s) ]  ]  [ /Role = Role_Value ] [ /Capability = Capability_Value]
 */
std::vector<std::pair<std::string,std::string> >
parseFQAN(const std::string &fqan)
{
        GLITE_STACK_TRY("parseFQAN");

        std::vector<std::string> tokens;
        std::vector<std::pair<std::string,std::string> > vect;
        std::string label = "";
        std::string value = "";
        int nt = 0;
        boost::char_separator<char> separator("/");
        boost::tokenizer<boost::char_separator<char> >
        tok(fqan, separator);
        boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin();
        boost::tokenizer<boost::char_separator<char> >::iterator const end = tok.end();
        for (; it != end; it++) {
			tokens.push_back(string(*it));
        }
        // number of found tokens
        nt = tokens.size();
        if (nt > 0) {
                // Checks the VO field
                wmputilities::split(tokens[0], label, value);
                if ((label.size() > 0 || value.size() >0) &&
                         ((label.compare(FQAN_FIELDS[FQAN_ROLE])==0) ||
                         (label.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0)) ){
                                throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                       		 "malformed fqan (VO field is missing): " +  fqan);
                } else {
                        vect.push_back(make_pair(FQAN_FIELDS[FQAN_VO], tokens[0]));
                        tokens.erase(tokens.begin());
                }
                // Checks group and subgroups fields (if present)
                while (tokens.empty()==false) {
                        wmputilities::split(tokens[0], label, value);
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
                        wmputilities::split(tokens[0], label, value);
                        // Checks whether the fqan contains the  ROLE field
                        if (label.compare(FQAN_FIELDS[FQAN_ROLE])==0) {
                                if (value.size()==0) {
					throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                                        	 "parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                                        	 "malformed FQAN field /" + tokens[0] );
                                }
                                // Role is present (and the value is not "null")
                                if (value.compare(FQAN_NULL) != 0){
                                        vect.push_back(make_pair(label, value));
                                }
                                tokens.erase(tokens.begin());
                                if (tokens.empty()==false) {
                                        // This token has to contain only the Capability field
                                        wmputilities::split(tokens[0], label, value);
                                } else {
                                        label = "";
                                        value = "";
                                }
                        }
                        // No other tokes must be present
                        if (tokens.size() > 1) {
                                nt = tokens.size();
                                ostringstream err;
                                err << "malformed FQAN field; one or more field are invalid :\n";
                                for (int i = 0; i < nt; i++) {
                                        err << "/" << tokens[i] << "\n";
                                }
                                throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                                        err.str() );
                        }

                        if (label.compare(FQAN_FIELDS[FQAN_CAPABILITY])==0) {
                                if (value.size()==0) {
                               		 throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                                         	"parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                                        	"malformed FQAN field (/" + tokens[0] );
                                }
                                // Capability is present (and the value is not "null")
                                if (value.compare(FQAN_NULL) != 0){
                                        vect.push_back(make_pair(label, value));
                                }
                                tokens.erase(tokens.begin());

                        } else {
                                if (label.size()>0 || value.size()>0) {
                                        throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                                         "parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                                        "malformed FQAN field; invalid Capability field: /" + tokens[0] );
                                }
                        }
                }
        } else {
                throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                        "parseFQAN(string)",  wmputilities::WMS_PROXY_ERROR,
                                "invalid fqan: " + fqan  );
        }
        return vect;
        GLITE_STACK_CATCH();
}

bool
compareFQAN (const string &ref, const string &in )
{
	GLITE_STACK_TRY("compareFQAN");
	edglog_fn("WMPAuthorizer::compareFQAN");

	// Checking for empty FQAN
	if ((ref == "") && (in == "")) {
		return true;
	}
	if ((ref == "") || (in == "")) {
		return false;
	}

	bool match = true;

	std::vector<std::pair<std::string, std::string> > vect_ref, vect_in;
	string lab_ref = "";
	string lab_in = "";
	string val_ref = "";
	string val_in = "";

	// the vectors contain pairs like this <label,value> (label may be an empty string)
	vect_ref = parseFQAN(ref );
	if ( vect_ref.empty()){
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"compareFQAN(string, string)", wmputilities::WMS_AUTHORIZATION_ERROR,
			"no valid fields in the FQAN string: [" + ref + "] (please contact the server administrator");
	}
	// vin=<input-vect>
	vect_in = parseFQAN(in);
	if (vect_in.empty()) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"compareFQAN(string, string)", wmputilities::WMS_AUTHORIZATION_ERROR,
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
	while (vect_ref.empty()==false && match)  {
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
   	GLITE_STACK_CATCH();
}

long 
getNotBefore(const string &pxfile)
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
        		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::getProxyTimeLeft", wmputilities::WMS_AUTHORIZATION_ERROR,
			    	"Proxy file doesn't exist or has bad permissions");
		}
			sec = ASN1_UTCTIME_get(X509_get_notBefore(x));
			free(x);
		} else {
			BIO_free(in);
			edglog(error)<<"Unable to get Not Before date from Proxy"<<endl;
			throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
				"getNotBefore()", wmputilities::WMS_PROXY_ERROR,
				"Unable to get Not Before date from Proxy");
		}
		BIO_free(in);
	} else {
		edglog(error)<<"Unable to get Not Before date from Proxy (BIO SSL error)"
			<<endl;
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"getNotBefore()", wmputilities::WMS_PROXY_ERROR,
			"Unable to get Not Before date from Proxy (BIO SSL error)");
	}
	return sec;
	
	GLITE_STACK_CATCH();
}

bool
compareFQANAuthN(const string &ref, const string &in )
{
        GLITE_STACK_TRY("compareFQAN");
        edglog_fn("WMPAuthorizer::compareFQANAuthN");

        // Checking for empty FQAN
        if ((ref == "") && (in == "")) {
                return true;
        }
        if ((ref == "") || (in == "")) {
                return false;
        }

        bool match = true;

        std::vector<std::pair<std::string,std::string> > vect_ref, vect_in;
        string lab_ref = "";
        string lab_in = "";
        string val_ref = "";
        string val_in = "";

        // the vectors contain pairs like this <label,value> (label may be an empty string)
        vect_ref = parseFQAN(ref );
        if ( vect_ref.empty()){
                throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                        "compareFQAN(string, string)", wmputilities::WMS_AUTHORIZATION_ERROR,
                        "no valid fields in the FQAN string: [" + ref + "] (please contact the server administrator");
        }
        // vin=<input-vect>
        vect_in = parseFQAN(in);
        if (vect_in.empty()) {
                throw wmputilities::AuthorizationException(__FILE__, __LINE__,
                        "compareFQAN(string, string)", wmputilities::WMS_AUTHORIZATION_ERROR,
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
        while ((vect_ref.empty()==false && match) || (vect_in.empty()==false && match)) {
                if (vect_ref.empty()) {
                        // ref-FQQAN contains no other fields
                        // in-FQAN does
                        match = false;
                        break;
                }
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
                        //      cout << "####compareFQANs> GROUPS (0) - match = " << match << "\n";
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
        if ((vect_ref.empty()==false && match)  || (vect_in.empty()==false && match)) {
                if (vect_ref.empty()) {
                        // ref-FQQAN contains no other fields
                        // in-FQAN does
                        match = false;
                }
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
        if ((vect_ref.empty()==false && match) || (vect_in.empty()==false && match)) {
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
        GLITE_STACK_CATCH();
}

} // anonymous namespace

void
do_authZ_jobid(std::string const& action, std::string const& job_id)
{
        // retrieve delegated proxy in job directory
        std::string delegatedproxy = wmputilities::getJobDelegatedProxyPath(job_id);
        checkProxyValidity(delegatedproxy);
        WMPAuthorizer auth(action, delegatedproxy);
        VOMSAuthN vomsproxy(delegatedproxy);
        if (vomsproxy.hasVOMSExtension()) {
                auth.authorize(vomsproxy.getDefaultFQAN()); //throws
        } else {
                auth.authorize(); // throws
        }
}

void
do_authZ(std::string const& action, std::string const& delegation_id)
{
        std::string delegatedproxy = glite::wms::wmproxy::server::getDelegatedProxyPath(delegation_id);
        checkProxyValidity(delegatedproxy);
        WMPAuthorizer auth(action, delegatedproxy);
        VOMSAuthN vomsproxy(delegatedproxy);
        if (vomsproxy.hasVOMSExtension()) {
                auth.authorize(vomsproxy.getDefaultFQAN()); //throws
        } else {
                auth.authorize(); // throws
        }
}

std::string
do_authZ(std::string const& action)
{
        WMPAuthorizer auth(action);
        auth.authorize(); // throws
        return auth.getUserName();
}

void
setGridsiteJobGacl(std::vector<std::string> &jobids)
{
	GLITE_STACK_TRY("setGridsiteJobGacl()");
	edglog_fn("WMPAuthorizer::setGridsiteJobGacl vector");
	
	if (jobids.size()) {
		string user_dn = wmputilities::getDN_SSL(); // taken from ssl
		string errmsg = "";
		
		// Creates a gacl file in the job directory
		WMPgaclPerm permission =
			GaclManager::WMPGACL_LIST |
			GaclManager::WMPGACL_WRITE |
			GaclManager::WMPGACL_READ;
	 	
		// main user job directory
		string filename = wmputilities::getJobDirectoryPath(jobids[0]) + "/" 
			+ GaclManager::WMPGACL_DEFAULT_FILE;
		
		try {
			GaclManager gacl(filename, true);
			// adds the new user credential entry
			gacl.addEntry(GaclManager::WMPGACL_PERSON_TYPE, user_dn.c_str());
			// allow permission
			gacl.allowPermission(GaclManager::WMPGACL_PERSON_TYPE,
					user_dn.c_str(), permission);
			gacl.saveGacl();
		} catch (wmputilities::GaclException &exc) {
			errmsg = "internal server error: unable to set the gacl user properties";
			errmsg += "\n(please contact server administrator)\n";
			errmsg += "please report the following message:\n" ;
			errmsg += exc.what();
			throw wmputilities::GaclException(__FILE__, __LINE__, "setGridsiteJobGacl()",
				wmputilities::WMS_GACL_FILE, errmsg);
		}
		ifstream infile(filename.c_str());
		if (!infile.good()) {
			throw wmputilities::FileSystemException(__FILE__, __LINE__,
				"setGridsiteJobGacl()", wmputilities::WMS_IS_FAILURE, "Unable to open gacl "
				"input file\n(please contact server administrator)");
		}
		string gacltext = "";
		string s;
		while (getline(infile, s, '\n')) {
			gacltext += s + "\n";
		}
		infile.close();
		
		fstream outfile;
		
		std::vector<std::string>::iterator iter = jobids.begin();
		std::vector<std::string>::iterator const end = jobids.end();
		for (; iter != end; ++iter) {
			filename = wmputilities::getJobDirectoryPath(*iter) + "/"
				+ GaclManager::WMPGACL_DEFAULT_FILE;
			outfile.open(filename.c_str(), ios::out);
			if (!outfile.good()) {
				throw wmputilities::FileSystemException(__FILE__, __LINE__,
					"setGridsiteJobGacl()", wmputilities::WMS_IS_FAILURE, "Unable to open gacl "
					"output file\n(please contact server administrator)");
			}
			outfile<<gacltext;
			outfile.close();
		}
	}
	
	GLITE_STACK_CATCH();
}

void
setGridsiteJobGacl(const string &jobid)
{
	GLITE_STACK_TRY("setGridsiteJobGacl()");
	edglog_fn("WMPAuthorizer::setGridsiteJobGacl string");
	
	string user_dn = wmputilities::getDN_SSL(); // taken from ssl
	
	// Creates a gacl file in the job directory
	WMPgaclPerm permission =
		GaclManager::WMPGACL_LIST |
		GaclManager::WMPGACL_WRITE |
		GaclManager::WMPGACL_READ;
 	
	// main user job directory
	string filename = wmputilities::getJobDirectoryPath(jobid) + "/"
		+ GaclManager::WMPGACL_DEFAULT_FILE;
	
	try {
		GaclManager gacl(filename, true);
		// adds the new user credential entry
		gacl.addEntry(GaclManager::WMPGACL_PERSON_TYPE, user_dn.c_str());
		// allow permission
		gacl.allowPermission(GaclManager::WMPGACL_PERSON_TYPE,
				user_dn.c_str(), permission);
		gacl.saveGacl( );
	} catch (wmputilities::GaclException &exc) {
		string errmsg = "internal server error: unable to set the gacl user properties";
		errmsg += " (please contact server administrator)\n";
		errmsg += "please report the following message:\n" ;
		errmsg += exc.what ( );
		throw wmputilities::GaclException(__FILE__, __LINE__, "setGridsiteJobGacl()",
			wmputilities::WMS_GACL_FILE, errmsg);
	}
		
	GLITE_STACK_CATCH();
}

bool 
checkGridsiteJobDrain()
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
			.append(GaclManager::WMPGACL_DEFAULT_DRAIN_FILE);
		edglog(debug) <<"checking drain_file: "<<drain_file<<endl;
		// the drain is ony checked if the gacl file exists (if it doesn't no exception is thrown)
		if (utilities::fileExists(drain_file)){
			GaclManager gacl(drain_file) ;
			if (gacl.hasEntry(GaclManager::WMPGACL_ANYUSER_TYPE)){
				exec = gacl.checkAllowPermission(
					GaclManager::WMPGACL_ANYUSER_TYPE,
					GaclManager::WMPGACL_ANYUSER_CRED,
					GaclManager::WMPGACL_EXEC);
			}
		}
	} else {
		string msg =
		"Internal server error: information on the document root location is not available (please contact server administrator)";
		throw wmputilities::GaclException (__FILE__, __LINE__, "checkJobDrain()" ,
			wmputilities::WMS_GACL_ERROR, msg);
	}
	return (!exec);
	
	GLITE_STACK_CATCH();
}

void
checkProxyValidity(const string &proxy)
{
	GLITE_STACK_TRY("checkProxyValidity()");
	edglog_fn("WMPAuthorizer::checkProxyValidity");

	edglog(debug)<<"Proxy path: "<<proxy<<endl;
	
	static int const PROXY_TIME_MISALIGNMENT_TOLERANCE = 5;
	time_t now = time(NULL);
	time_t proxytime = getNotBefore(proxy);
	double timediff = proxytime - now;
	edglog(debug)<<"Delegated Proxy Time difference (proxy - now): "<< boost::lexical_cast<std::string>(timediff)<<endl;
	if (timediff > PROXY_TIME_MISALIGNMENT_TOLERANCE) {
		edglog(error)<<"Proxy validity starting time in the future ("<< timediff << " secs)"  <<endl;
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"checkProxyValidity()", wmputilities::WMS_PROXY_ERROR,
			"Proxy validity starting time in the future"
			"\nPlease check client date/time");
	} else {
		if (timediff > 0) {
			edglog(debug)<<"Tolerable Proxy validity starting time in the future ("
				<<timediff<<" secs)"<<endl;
		}
	}
	long timeleft = getProxyTimeLeft(proxy);
	edglog(debug)<<"Proxy Time Left (should be positive number): "<<timeleft<<endl;
	if (timeleft <= 1) {
		edglog(error)<<"The delegated Proxy has expired!"<<endl;
		throw wmputilities::ProxyOperationException(__FILE__, __LINE__,
			"checkProxyValidity()", wmputilities::WMS_PROXY_EXPIRED,
			"The delegated Proxy has expired");
	}
	GLITE_STACK_CATCH();
}

void
checkProxyExistence(const string &userproxypath, const string &jobid)
{
	GLITE_STACK_TRY("checkProxyExistence()");
	edglog_fn("WMPAuthorizer::checkProxyExistence");
	
	string userproxypathbak = wmputilities::getJobDelegatedProxyPathBak(jobid);
	if (!wmputilities::fileExists(userproxypath)) {
		if (!wmputilities::fileExists(userproxypathbak)) {
			edglog(error)<<"Unable to find a Proxy file in the job directory for job:\n"
				<<jobid<<endl;
			throw wmputilities::JobOperationException(__FILE__, __LINE__,
				"checkProxyExistence()", wmputilities::WMS_OPERATION_NOT_ALLOWED,
				"Unable to find a Proxy file in the job directory");
		} else {
			unlink(userproxypath.c_str());
			wmputilities::fileCopy(userproxypathbak, userproxypath);
		}
	} else {
               char* c_x509_proxy = 0;
                 
               // Checking if the proxy is still registered to proxyrenewal
               int const err_code( glite_renewal_GetProxy(jobid.c_str(), &c_x509_proxy) );
                 
               if (err_code == 0) {
                    free(c_x509_proxy);
                  // If the proxy is still registered i can override the back up
                  wmputilities::fileCopy(userproxypath, userproxypathbak);
               } else {
                  // If the proxy is not registered i ovverride the user.proxy with its back up
                  unlink(userproxypath.c_str());
                  wmputilities::fileCopy(userproxypathbak, userproxypath);
               }

	}
	
	GLITE_STACK_CATCH();
}

WMPAuthorizer::WMPAuthorizer(std::string const& action)
: userid_(0), usergroup_(0), mapdone_(false), action_(action)
{
	// only the user DN can be taken (from grisite delegation as env var)
        userdn_ = wmputilities::getDN_SSL();
        fqans_ = wmputilities::getGridsiteFQANs();
}

WMPAuthorizer::WMPAuthorizer(std::string const& action, std::string const& userproxypath)
: userid_(0), usergroup_(0), mapdone_(false), action_(action)
{
	// we have the delegated proxy, we can directly use the VOMS APIs
        VOMSAuthN authn(userproxypath);
        userdn_ = authn.getDN();
        fqans_ = authn.getFQANs();
}

WMPAuthorizer::~WMPAuthorizer() {}

string
WMPAuthorizer::getUserName()
{
	GLITE_STACK_TRY("getUserName()");
	if (!mapdone_) {
		map_user_lcmaps();
	}
	return username_;
	GLITE_STACK_CATCH();
}

uid_t
WMPAuthorizer::getUserId()
{
	GLITE_STACK_TRY("getUserId()");
	if (!mapdone_) {
		map_user_lcmaps();
	}
	return userid_;
	GLITE_STACK_CATCH();
}

uid_t
WMPAuthorizer::getUserGroup()
{
	GLITE_STACK_TRY("getUserGroup()");
	if (!mapdone_) {
		map_user_lcmaps();
	}
	return usergroup_;
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::authorize(const string &certfqan, const string & jobid)
{
	GLITE_STACK_TRY("authorize()");
	edglog_fn("WMPAuthorizer::authorize");

	if (configuration::Configuration::instance()->wp()->argus_authz()) {
		std::vector<std::string> endpoints =
			configuration::Configuration::instance()->wp()->argus_pepd_endpoints();
		if (!endpoints.empty()) {
			boost::tuple<bool, xacml_decision_t, uid_t, gid_t> ans;
			ans = argus_authZ(
			        endpoints,
			        fqans_,
			        wmputilities::getEndpoint(),
			        action_,
			        wmputilities::getDN_SSL());
			if (ans.get<1>() == XACML_DECISION_PERMIT) {
				// set the mapping
				userid_ = ans.get<2>();
				usergroup_ = ans.get<3>();
				mapdone_ = true;
				return;
			} else {
				throw wmputilities::AuthorizationException(__FILE__, __LINE__,
				"authorize()", wmputilities::WMS_AUTHORIZATION_ERROR,
				"Argus denied authorization on " + action_ + " by " +wmputilities::getDN_SSL());
			}
		}
	}

	edglog(debug) << "Gridsite authZ" << endl;
	checkGaclUserAuthZ(fqans_.front(), userdn_);

	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::map_user_lcmaps()
{
	GLITE_STACK_TRY("map_user_lcmaps()");
	edglog_fn("WMPAuthorizer::map_user_lcmaps");
	mapdone_ = false;
	int retval;
	struct passwd* user_info = 0;

	setenv("LCMAPS_POLICY_NAME", "standard:voms", 1);

        lcmaps_init_and_logfile(
		"/var/log/glite/lcmaps.log",
		0 /* no FILE* provided */,
		(unsigned short)1 /* user logging */);
	lcmaps_account_info_t plcmaps_account;
	retval = lcmaps_account_info_init(&plcmaps_account);
	if (retval) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
		"lcmaps_account_info_init()", wmputilities::WMS_USERMAP_ERROR,
		"LCMAPS info initialization failure");
	}

	// Send user mapping request to LCMAPS
	int mapcounter = 0; // single mapping result
	int fqan_num = 1; // N.B. Considering only one FQAN inside the list
	char * fqan_list[1]; // N.B. Considering only one FQAN inside the list
	fqan_list[0] = const_cast<char*>(fqans_.front().c_str());
	string user_dn = wmputilities::getDN_SSL();
	retval = lcmaps_return_account_without_gsi(const_cast<char*>(user_dn.c_str()),
		fqan_list, fqan_num, mapcounter, &plcmaps_account);

	if (retval) {
		retval = lcmaps_return_account_without_gsi(const_cast<char*>(user_dn.c_str()),
			fqan_list, fqan_num, mapcounter, &plcmaps_account);
		if (retval) {
			edglog(error)<<"LCMAPS failed authorization: User "<<user_dn <<" is not authorized"<<endl;
			throw wmputilities::AuthorizationException(__FILE__, __LINE__,
				"lcmaps_return_poolindex_without_gsi()",
				wmputilities::WMS_AUTHORIZATION_ERROR,
				"LCMAPS failed to map user credential");
		}
	}

	userid_ = plcmaps_account.uid;
	user_info = getpwuid(userid_);
	if (user_info == NULL) {
		edglog(error)<<"LCMAPS: Unkwonwn uid " << userid_ << endl;
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"getpwuidn()",wmputilities::WMS_USERMAP_ERROR,
			"LCMAPS could not find the username related to uid");
	}
	// Checking for mapped user group. The group of the assigned local user
	// MUST be different from the group of user running server
	if (user_info->pw_gid == getgid()) {
		edglog(error)<<"Mapping not allowed, mapped local user group equal "
			"to group of user running server"<<endl;
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"map_user_lcmaps()", wmputilities::WMS_USERMAP_ERROR,
			"Mapping not allowed, mapped local user group equal to group"
			" of user running server\n(please contact server administrator)");
	}
	// Setting value for username private member
	username_ = string(user_info->pw_name);
	// Setting value for usergroup private member
	usergroup_ = user_info->pw_gid;
	// Cleaning structure
	retval = lcmaps_account_info_clean(&plcmaps_account);
	if (retval) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"lcmaps_account_info_clean()", wmputilities::WMS_USERMAP_ERROR,
			"LCMAPS info clean failure");
	}
	mapdone_ = true;
	GLITE_STACK_CATCH();
}

void
WMPAuthorizer::checkGaclUserAuthZ(string const& fqan, string const& dn)
try {
	GLITE_STACK_TRY("checkGaclUserAuthZ()");
	edglog_fn("WMPAuthorizer::checkGaclUserAuthZ");

	bool exec = false;
	bool execDN = false;
    	bool execAU = false;
    	bool exist = false;
   	bool existDN = false;
    	bool existAU = false;

	string dn = wmputilities::getDN_SSL(); // taken from ssl

	// Gacl-Mgr
	string gaclfile;
	if (getenv("GLITE_WMS_CONFIG_DIR")) {
		gaclfile = string(getenv("GLITE_WMS_CONFIG_DIR"))
			+ '/' + GACL_FILE;
	} else if (getenv("GLITE_LOCATION")) {
		gaclfile = string(getenv("GLITE_LOCATION")) + "/etc/"
			+ GACL_FILE;
	} else if (getenv("WMS_LOCATION_ETC")) {
		gaclfile = string(getenv("WMS_LOCATION_ETC")) + '/'
			+ GACL_FILE;
	} else if (getenv("WMS_LOCATION")) {
		gaclfile = string(getenv("WMS_LOCATION")) + "/etc/"
			+ GACL_FILE;
	} else {
		gaclfile = "/etc/glite-wms/" + GACL_FILE;
	}
	GaclManager gacl(gaclfile);

        edglog(debug)<<"Checking gacl file entries..."<<endl;

        // checking credential types present in gacl file
        exist = gacl.checkCredentialEntries(GaclManager::WMPGACL_VOMS_CRED);
        existDN = gacl.checkCredentialEntries(GaclManager::WMPGACL_PERSON_CRED);
        existAU = gacl.checkCredentialEntries(GaclManager::WMPGACL_ANYUSER_CRED);

        if (exist) {
                edglog(debug)<<"VOMS credential type present"<<endl;
        }
        if (existDN){
                edglog(debug)<<"person credential type present"<<endl;
        }
        if (existAU){
                edglog(debug)<<"any-user credential type present"<<endl;
        }

        // checks exec permission
        if (!fqan.empty()) { // user proxy has FQAN
                // ANY USER authorization
                if (existAU) {
                        execAU = gacl.checkAllowPermission(
                                GaclManager::WMPGACL_ANYUSER_TYPE,
                                "",GaclManager::WMPGACL_EXEC);
                }

                // FQAN authorization
                if (exist) {
                        exec =  gacl.checkAllowPermission(
                                  GaclManager::WMPGACL_VOMS_TYPE,
                                  fqan,
                                  GaclManager::WMPGACL_EXEC);
                        // overrides any-user authorization if VO auth is fine
                        if (exec) {
                                execAU = true;
                        }
                } else if (existAU || existDN) {
                                exec = true;
                        } else {
                                exec = false;
                }

                // DN authorization
                if (existDN){
                        execDN = gacl.checkAllowPermission(
                                        GaclManager::WMPGACL_PERSON_TYPE,
                                        dn,GaclManager::WMPGACL_EXEC);
                        // overrides VO and any-user authorization if DN is fine
                        if (execDN) {
                                exec = true;
                                execAU = true;
                        }
                } else if (existDN){
                        execDN = gacl.checkAllowPermission(
                                        GaclManager::WMPGACL_PERSON_TYPE,
                                        dn,
                                        GaclManager::WMPGACL_EXEC);
                        // overrides VO and any-user authorization if DN is fine
                        if (execDN) {
                                exec = true;
                                execAU = true;
                        }
                } else if (execAU || exec){
                                execDN = true;
                } else {
                                execDN = false;
                }

        } else {
                // user proxy does not have FQAN
                // any-user authorization
                if (existAU){
                        execAU = gacl.checkAllowPermission(
                                GaclManager::WMPGACL_ANYUSER_TYPE,
                                "",GaclManager::WMPGACL_EXEC);
                }
                // DN authorization
                if (existDN){
                        execDN = gacl.checkAllowPermission(
                                        GaclManager::WMPGACL_PERSON_TYPE,dn,
                                        GaclManager::WMPGACL_EXEC )
                                 ||
                                 gacl.checkAllowPermission(
                                        GaclManager::WMPGACL_PERSON_TYPE,dn,
                                        GaclManager::WMPGACL_EXEC );
                        // overrides any-user authorization if DN auth is fine
                        if (execDN) {
                                execAU = true;
                        }
                }

                // gacl file has no valid entries for user proxy without fqan
                if (!(execDN || execAU)){
                        exec = false;
                }
        }

        // Final exec authorization value
        exec = exec && execDN && execAU;
        
	// checks exec permission
	if (!exec) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"checkGaclUserAuthZ()",
			wmputilities::WMS_AUTHORIZATION_ERROR,
			"Authorization error: user not authorized");
	}
	GLITE_STACK_CATCH();
} catch (wmputilities::GaclException &exc) {
  string errmsg = "User not authorized:\n";
  errmsg += exc.what();
  throw wmputilities::GaclException(__FILE__, __LINE__, "checkGaclUserAuthZ()", wmputilities::WMS_GACL_FILE, errmsg);
}

}}}}

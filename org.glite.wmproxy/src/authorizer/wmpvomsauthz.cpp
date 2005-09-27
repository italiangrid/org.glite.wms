/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpvomsauthz.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include <string>
#include <openssl/pem.h>
#include <openssl/ssl.h>

#include "wmpvomsauthz.h"

// Logger
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "utilities/logging.h"

// Exception
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

const char * X509_VOMS_DIR = "X509_VOMS_DIR";
const char * X509_CERT_DIR = "X509_CERT_DIR";
const char * VOMS_DIR = "/etc/grid-security/vomsdir";
const char * CERT_DIR = "/etc/grid-security/certificates";

namespace logger       = glite::wms::common::logger;
namespace wmputilities = glite::wms::wmproxy::utilities;  

using namespace std;
using namespace glite::wms::wmproxy::utilities; //Exception

//
// Printing methods for debugging purposes
//

/*void printvoms(voms *i)
{
  edglog(debug) << "SIGLEN: " << i->siglen << std::endl << "USER:" << i->user << std::endl
	    << "UCA: " << i->userca << std::endl << "SERVER: " << i->server << std::endl
	    << "SCA: " << i->serverca << std::endl << "VO: " << i->voname << std::endl
	    << "URI: " << i->uri << std::endl << "DATE1: " << i->date1 << std::endl
	    << "DATE2: " << i->date2 << std::endl;
	if (i->version >= 1) {
		for (unsigned int j = 0; j < i->fqan.size(); j++) {
			edglog(debug)<<"FQAN: "<<i->fqan[j]<<endl;
		}
	}

  switch (i->type) {
  case TYPE_NODATA:
    edglog(debug) << "NO DATA" << std::endl;
    break;
  case TYPE_CUSTOM:
    edglog(debug) << i->custom << std::endl;
    break;
  case TYPE_STD:
    for (std::vector<data>::iterator j = i->std.begin(); j != i->std.end(); j++)
      edglog(debug) << "GROUP: " << j->group << std::endl
		<< "ROLE: " << j->role << std::endl
		<< "CAP: " << j->cap << std::endl;
    break;
  }
}

void print(vomsdata &d)
{
  std::vector<voms> v = d.data;
  int k = 0;

  for (std::vector<voms>::iterator i=v.begin(); i != v.end(); i++) {
    edglog(debug)<<++k<< std::endl;
    printvoms(&(*i));
#if 0
    edglog(debug) << "SIGLEN: " << i->siglen << std::endl << "USER:" << i->user << std::endl
	      << "UCA: " << i->userca << std::endl << "SERVER: " << i->server << std::endl
	      << "SCA: " << i->serverca << std::endl << "VO: " << i->voname << std::endl
	      << "URI: " << i->uri << std::endl << "DATE1: " << i->date1 << std::endl
	      << "DATE2: " << i->date2 << std::endl;

    switch (i->type) {
    case TYPE_NODATA:
      edglog(debug) << "NO DATA" << std::endl;
      break;
    case TYPE_CUSTOM:
      edglog(debug) << i->custom << std::endl;
      break;
    case TYPE_STD:
      for (std::vector<data>::iterator j = i->std.begin(); j != i->std.end(); j++)
	edglog(debug) << "GROUP: " << j->group << std::endl
		  << "ROLE: " << j->role << std::endl
		  << "CAP: " << j->cap << std::endl;
      break;
    }
#endif
  }
  edglog(debug) << "WORKVO: " << d.workvo << std::endl
	    << "EXTRA: " << d.extra_data << std::endl << std::flush;
}
// END printing methods
*/

STACK_OF(X509) * load_chain(char *certfile)
{
  STACK_OF(X509_INFO) *sk=NULL;
  STACK_OF(X509) *stack=NULL, *ret=NULL;
  BIO *in=NULL;
  X509_INFO *xi;
  int first = 1;

  if(!(stack = sk_X509_new_null())) {
    edglog(severe)<<"Memory allocation failure"<<endl;
    BIO_free(in);
  	sk_X509_INFO_free(sk);
  }

  if(!(in=BIO_new_file(certfile, "r"))) {
    edglog(severe)<<"Error opening the file: "<<string(certfile)<<endl;
    BIO_free(in);
  	sk_X509_INFO_free(sk);
  }

  // This loads from a file, a stack of x509/crl/pkey sets
  if(!(sk=PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
    edglog(severe)<<"Error reading the file: "<<string(certfile)<<endl;
    BIO_free(in);
  	sk_X509_INFO_free(sk);
  }

  // scan over it and pull out the certs
  while (sk_X509_INFO_num(sk)) {
    // skip first cert
    if (first) {
      first = 0;
      continue;
    }
    xi=sk_X509_INFO_shift(sk);
    if (xi->x509 != NULL) {
      sk_X509_push(stack,xi->x509);
      xi->x509=NULL;
    }
    X509_INFO_free(xi);
  }
  if(!sk_X509_num(stack)) {
    edglog(severe)<<"No certificates in file: "<<string(certfile)<<endl;
    sk_X509_free(stack);
    BIO_free(in);
  	sk_X509_INFO_free(sk);
  }
  BIO_free(in);
  ret=stack;
  return ret;
}



VOMSAuthZ::VOMSAuthZ(const string &proxypath)
{
	edglog_fn("VOMSAuthZ::VOMSAuthZ");
	edglog(debug)<<"Proxy path: "<<proxypath<<endl;
	
	if (parseVoms(const_cast<char*>(proxypath.c_str()))) {
		edglog(info)<<"Not a VOMS Proxy"<<endl;
		throw NotAVOMSProxyException(__FILE__, __LINE__,
	    	"VOMSAuthZ::VOMSAuthZ", wmputilities::WMS_NOT_A_VOMS_PROXY,
	    	"Not a VOMS Proxy");
	}
}

VOMSAuthZ::~VOMSAuthZ()
{
	if (this->data) {
		VOMS_Destroy(this->data);
	}
}

string
errormessage(int error)
{
	GLITE_STACK_TRY("errormessage()");
	
	string msg = "Unable to retrive VOMS Proxy information: ";
	switch (error) {
		case VERR_NOEXT:
			msg += "VERR_NOEXT";
		    break;
		case VERR_IDCHECK:
		    msg += "VERR_IDCHECK";
		    break;
		case VERR_TIME:
			msg += "The user delegated Proxy has expired";
		    break;
		case VERR_ORDER:
			msg += "VERR_ORDER";
		    break;
		case VERR_NOSOCKET:
			msg += "VERR_NOSOCKET";
		    break;
		case VERR_NOIDENT:
			msg += "VERR_NOIDENT";
		    break;
		case VERR_COMM:
			msg += "VERR_COMM";
		    break;
		case VERR_PARAM:
			msg += "VERR_PARAM";
		    break;
		case VERR_NOINIT:
			msg += "VERR_NOINIT\n(please check delegated Proxy validity)";
		    break;
		case VERR_EXTRAINFO:
			msg += "VERR_EXTRAINFO";
		    break;
		case VERR_FORMAT:
			msg += "VERR_FORMAT";
		    break;
		case VERR_NODATA:
			msg += "VERR_NODATA";
		    break;
		case VERR_PARSE:
			msg += "VERR_PARSE";
		    break;
		case VERR_DIR:
			msg += "VERR_DIR";
		    break;
		case VERR_SIGN:
			msg += "VERR_SIGN";
		    break;
		case VERR_SERVER:
			msg += "VERR_SERVER";
		    break;
		case VERR_MEM:
			msg += "VERR_MEM";
		    break;
		case VERR_VERIFY:
			msg += "VERR_VERIFY";
		    break;
		case VERR_TYPE:
			msg += "VERR_TYPE";
		    break;
		default:
			msg += "default";
		  	break;
	}
	edglog(debug)<<msg<<endl;
	return msg;
	
	GLITE_STACK_CATCH();
}

char *
VOMSAuthZ::getDN()
{
	GLITE_STACK_TRY("getDN()");
	
	if (this->data) {
		int error = 0;
		struct voms * defaultvoms = VOMS_DefaultData(this->data, &error);
		if (defaultvoms) {
			char * dn = defaultvoms->user;
			return dn;
		} else {
			throw AuthorizationException(__FILE__, __LINE__,
		    	"VOMSAuthZ::getDN", wmputilities::WMS_AUTHZ_ERROR,
		    	errormessage(error));
		}
		free(defaultvoms);
	}
	// Proxy certificate is not a VOMS Proxy certificate
	return NULL;
	
	GLITE_STACK_CATCH();
}

string
VOMSAuthZ::getDefaultFQAN()
{
	GLITE_STACK_TRY("getDefaultFQAN()");
	
	if (this->data) {
		int error = 0;
		struct voms * defaultvoms = VOMS_DefaultData(this->data, &error);
		if (defaultvoms) {
			char ** deffqan = defaultvoms->fqan;
			return string(*deffqan);
		} else {
			throw AuthorizationException(__FILE__, __LINE__,
		    	"VOMSAuthZ::getDefaultFQAN", wmputilities::WMS_AUTHZ_ERROR,
		    	errormessage(error));
		}
		free(defaultvoms);
	}
	// Proxy certificate is not a VOMS Proxy certificate
	return "";
	
	GLITE_STACK_CATCH();
}

string
VOMSAuthZ::getDefaultVO()
{
	GLITE_STACK_TRY("getDefaultVO()");
	
	if (this->data) {
		int error = 0;
		struct voms * defaultvoms = VOMS_DefaultData(this->data, &error);
		if (defaultvoms) {
			return string(defaultvoms->voname);
		} else {
			throw AuthorizationException(__FILE__, __LINE__,
		    	"VOMSAuthZ::getDefaultVO", wmputilities::WMS_AUTHZ_ERROR,
		    	errormessage(error));
		}
		free(defaultvoms);
	}
	// Proxy certificate is not a VOMS Proxy certificate
	return "";
	
	GLITE_STACK_CATCH();
}

	
int
VOMSAuthZ::parseVoms(char * proxypath)
{
	GLITE_STACK_TRY("parseVoms()");
	
	edglog_fn("VOMSAuthZ::parseVoms");
	edglog(debug)<<"Proxy path: "<<string(proxypath)<<endl;
	int error = 0;
	
	char * envval = NULL;
	char * vomsdir = NULL;
	char * certdir = NULL;
	if ((envval = getenv(X509_VOMS_DIR))) {
		vomsdir = envval;
	} else {
		vomsdir = const_cast<char*>(VOMS_DIR);
	}
	if ((envval = getenv(X509_CERT_DIR))) {
		certdir = envval;
	} else {
		certdir = const_cast<char*>(CERT_DIR);
	}
	if (!(data = VOMS_Init(vomsdir, certdir))) {
		edglog(debug)<<"Error in VOMS_Init()"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
	    	"Unable to retrive VOMS Proxy information");
	}
	
	// Make sure the ciphers in the 'names_lh' OpenSSL global hash 
    // get initialised 
    SSL_library_init();
      		
  	BIO * in = NULL;
  	X509 * x = NULL;
  	STACK_OF(X509) * chain = NULL;
  	in = BIO_new(BIO_s_file());
  	if (in) {
    	if (BIO_read_filename(in, proxypath) > 0) {
      		x = PEM_read_bio_X509(in, NULL, 0, NULL);
      		if (!x) {
      			BIO_free(in);
        		edglog(severe)<<"Error in PEM_read_bio_X509: Proxy file "
        			"doesn't exist or has bad permissions"<<endl;
        		throw AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
			    	"Proxy file doesn't exist or has bad permissions");
      		}
      		chain = load_chain(proxypath);

  			// Verifing all flags except for VERIFY_DATE
      		// Time left verification is done by a different method after server
      		// operation call
      		//VOMS_SetVerificationType(VERIFY_NOTARGET | VERIFY_SIGN | VERIFY_ORDER
      		//	| VERIFY_ID, data, &error);
      		if (!VOMS_SetVerificationType(~VERIFY_DATE, data, &error)) {
      			BIO_free(in);
      			free(x);
      			string msg = errormessage(error);
				edglog(severe)<<"Error in VOMS_SetVerificationType: "<<msg<<endl;
      			throw AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
			    	msg);
      		}
  		
      		if (!VOMS_Retrieve(x, chain, RECURSE_CHAIN, data, &error)) {
      			BIO_free(in);
      			free(x);
      			if (error == VERR_NOEXT) {
					// Proxy is not a VOMS Proxy
					return 1;
				}
				string msg = errormessage(error);
				edglog(severe)<<"Error in VOMS_Retrieve: "<<msg<<endl;
				throw AuthorizationException(__FILE__, __LINE__,
			    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
			    	msg);
      		}
      		BIO_free(in);
      		free(x);
    	} else {
    		BIO_free(in);
    		edglog(severe)<<"Error in BIO_read_filename: Proxy file doesn't "
    			"exist or has bad permissions"<<endl;
			throw AuthorizationException(__FILE__, __LINE__,
		    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
		    	"Proxy file doesn't exist or has bad permissions");
		}
	} else {
		edglog(severe)<<"Error in BIO_new"<<endl;
		throw AuthorizationException(__FILE__, __LINE__,
	    	"VOMSAuthZ::parseVoms", wmputilities::WMS_AUTHZ_ERROR,
	    	"Unable to get information from Proxy file");
  	}
	return 0;
	
	GLITE_STACK_CATCH();
}

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

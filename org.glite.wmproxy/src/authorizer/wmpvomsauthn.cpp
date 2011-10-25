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
limitations under the License.  */

#include <string>
#include <openssl/pem.h>
#include <openssl/ssl.h>

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "utilities/logging.h"
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"
#include "wmpvomsauthn.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace authorizer {

const char* X509_VOMS_DIR = "X509_VOMS_DIR";
const char* X509_CERT_DIR = "X509_CERT_DIR";
const char* VOMS_DIR = "/etc/grid-security/vomsdir";
const char* CERT_DIR = "/etc/grid-security/certificates";

namespace logger = glite::wms::common::logger;

using namespace std;
using namespace glite::wms::wmproxy::utilities;

VOMSAuthN::VOMSAuthN(const string &proxypath)
{
	GLITE_STACK_TRY("parseVoms()");
	
	edglog_fn("VOMSAuthN::parseVoms");
	edglog(debug)<<"Proxy path: " << proxypath << endl;
	
	char* envval = NULL;
	char* vomsdir = NULL;
	char* certdir = NULL;
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
	if (!(data_ = vomsdata(vomsdir, certdir))) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
	    		"VOMSAuthN::parseVoms", wmputilities::WMS_AUTHORIZATION_ERROR,
		    	"Unable to init VOMS Proxy");
	}
	FILE* f = fopen(proxypath.c_str(), "r");
	if (f) {
		int error = 0;
		data_.SetVerificationType(~VERIFY_DATE);
		if (!data_.Retrieve(f, RECURSE_DEEP)) {
			throw wmputilities::AuthorizationException(__FILE__, __LINE__,
		    		"VOMSAuthN::parseVoms", wmputilities::WMS_AUTHORIZATION_ERROR,
		    		"Unable to retrieve VOMS Proxy information");
		}
	}
	if (!data_.DefaultData(defaultvoms_)) {
		throw wmputilities::AuthorizationException(__FILE__, __LINE__,
			"VOMSAuthN::getDN", wmputilities::WMS_AUTHORIZATION_ERROR,
			data_.ErrorMessage());
	}

	GLITE_STACK_CATCH();
}

VOMSAuthN::~VOMSAuthN()
{
	if (cert_) {
		X509_free(cert_);
	}
}

bool
VOMSAuthN::hasVOMSExtension()
{
	return data_;
}

std::vector<std::string> getFQANs() {
	return defaultvoms_->std;
}

std::string
VOMSAuthN::getDN()
{
	GLITE_STACK_TRY("getDN()");
	
	if (data_) {
		return std::string(defaultvoms_->user);
	}
	return 0; // not a VOMS Proxy certificate

	GLITE_STACK_CATCH();
}

std::string
VOMSAuthN::getDefaultFQAN()
{
	GLITE_STACK_TRY("getDefaultFQAN()");
	
	if (data_) {
		return string(*(defaultvoms_->fqan));
	}
	return ""; // not a VOMS Proxy certificate

	GLITE_STACK_CATCH();
}

VOProxyInfoStructType *
VOMSAuthN::getDefaultVOProxyInfo()
{
	GLITE_STACK_TRY("getDefaultVOProxyInfo()");
	VOProxyInfoStructType* voproxyinfo = new VOProxyInfoStructType();
	if (data_) {
		voproxyinfo->user = defaultvoms_->user;
		voproxyinfo->userCA = defaultvoms_->userca;
		voproxyinfo->server = defaultvoms_->server;
		voproxyinfo->serverCA = defaultvoms_->serverca;
		voproxyinfo->voName = defaultvoms_->voname;
		voproxyinfo->uri = defaultvoms_->uri;
		voproxyinfo->startTime = boost::lexical_cast<std::string>(
			convASN1Date(defaultvoms_->date1));
		voproxyinfo->endTime = boost::lexical_cast<std::string>(
			convASN1Date(defaultvoms_->date2));
		vector<string> fqanvector;
		char **temp;
    		for (temp = defaultvoms_->fqan; *temp; ++temp) {
			fqanvector.push_back(*temp);
    		}
    		voproxyinfo->attribute = fqanvector;
	}
	return voproxyinfo; // not a VOMS Proxy certificate

	GLITE_STACK_CATCH();
}

ProxyInfoStructType *
VOMSAuthN::getProxyInfo()
{
	static const std::string PROXY = "CN=";

	GLITE_STACK_TRY("getProxyInfo()");
	ProxyInfoStructType* proxyinfo = new ProxyInfoStructType();
	char* subject = X509_NAME_oneline(X509_get_subject_name(cert_), NULL, 0);

	if (subject) {
		string subjectstring = string(subject);
		if (subjectstring.find(PROXY) != string::npos) {
				proxyinfo->type = "proxy";
		} else {
				proxyinfo->type = "x509";
		}
	} else {
		proxyinfo->type = "unknown";
		vector<VOProxyInfoStructType*> voproxyinfovector;
		proxyinfo->vosInfo = voproxyinfovector;
	}
	
	proxyinfo->subject = string(subject);
	OPENSSL_free(subject);

	proxyinfo->issuer   = string(X509_NAME_oneline(X509_get_issuer_name(cert_), NULL, 0));
	// identity is the same as issuser
	proxyinfo->identity = proxyinfo->issuer;
	// getting strength
	int bits = -1;
	EVP_PKEY * key = X509_extract_key(cert_);
	bits = 8 * EVP_PKEY_size(key);
	if (key) {
		EVP_PKEY_free(key);
	}
	proxyinfo->strength = boost::lexical_cast<string>(bits);
	proxyinfo->startTime = boost::lexical_cast<std::string>(ASN1_UTCTIME_get(X509_get_notBefore(cert_)));
	proxyinfo->endTime = boost::lexical_cast<std::string>(ASN1_UTCTIME_get(X509_get_notAfter(cert_)));
	
	// getting voms info, if any
	if (data_) {
		proxyinfo->vosInfo.push_back(getDefaultVOProxyInfo());
	} else {
		edglog(warning)<<"The Proxy does not contain VOMS extension" << endl;
	}

	return proxyinfo;
	GLITE_STACK_CATCH();
}

namespace {

time_t 
ASN1_UTCTIME_get(const ASN1_UTCTIME *s)
{
	struct tm tm;
	int offset;

	memset(&tm, '\0', sizeof tm);
#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
	tm.tm_year = g2(s->data);
	if (tm.tm_year < 50) {
		tm.tm_year += 100;
	}
	tm.tm_mon = g2(s->data + 2) - 1;
	tm.tm_mday = g2(s->data + 4);
	tm.tm_hour = g2(s->data + 6);
	tm.tm_min = g2(s->data + 8);
	tm.tm_sec = g2(s->data + 10);
	if (s->data[12] == 'Z') {
		offset = 0;
	} else {
		offset = g2(s->data + 13) * 60 + g2(s->data + 15);
		if (s->data[12] == '-') {
			offset = -offset;
		}
	}
#undef g2

    return timegm(&tm) - offset * 60;
}

const long 
convASN1Date(const std::string &date)
{
    char     *str;
    time_t    offset;
    time_t    newtime = 0;
    char      buff1[32];
    char     *p;
    int       i;
    struct tm tm;
    int       size = 0;
    ASN1_TIME *ctm = ASN1_TIME_new();
    ctm->data   = (unsigned char *)(date.data());
    ctm->length = date.size();
    switch (ctm->length) {
        case 10: {
            ctm->type = V_ASN1_UTCTIME;
            break;
        }
        case 15: {
            ctm->type = V_ASN1_GENERALIZEDTIME;
            break;
        }
        default: {
            ASN1_TIME_free(ctm);
            ctm = NULL;
            break;
        }
    }
    if (ctm) {
        switch (ctm->type) {
            case V_ASN1_UTCTIME: {
                    size=10;
                    break;
            }
            case V_ASN1_GENERALIZEDTIME: {
                    size=12;
                    break;
            }
        }
        p = buff1;
        i = ctm->length;
        str = (char *)ctm->data;
        if ((i < 11) || (i > 17)) {
                newtime = 0;
        }
        memcpy(p,str,size);
        p += size;
        str += size;


        if ((*str == 'Z') || (*str == '-') || (*str == '+')) {
                *(p++)='0'; *(p++)='0';
        } else {
                *(p++)= *(str++); *(p++)= *(str++);
        }
        *(p++)='Z';
        *(p++)='\0';
        if (*str == 'Z') {
        	offset=0;
        } else {
            if ((*str != '+') && (str[5] != '-')) {
                    newtime = 0;
            }
            offset=((str[1]-'0')*10+(str[2]-'0'))*60;
            offset+=(str[3]-'0')*10+(str[4]-'0');
            if (*str == '-') {
                    offset=-offset;
            }
        }
        tm.tm_isdst = 0;
        int index = 0;
        if (ctm->type == V_ASN1_UTCTIME) {
            tm.tm_year  = (buff1[index++]-'0')*10;
            tm.tm_year += (buff1[index++]-'0');
         } else {
            tm.tm_year  = (buff1[index++]-'0')*1000;
            tm.tm_year += (buff1[index++]-'0')*100;
            tm.tm_year += (buff1[index++]-'0')*10;
            tm.tm_year += (buff1[index++]-'0');
        }
         if (tm.tm_year < 70) {
            tm.tm_year+=100;
        }

         if (tm.tm_year > 1900) {
            tm.tm_year -= 1900;
        }

        tm.tm_mon   = (buff1[index++]-'0')*10;
        tm.tm_mon  += (buff1[index++]-'0')-1;
        tm.tm_mday  = (buff1[index++]-'0')*10;
        tm.tm_mday += (buff1[index++]-'0');
        tm.tm_hour  = (buff1[index++]-'0')*10;
        tm.tm_hour += (buff1[index++]-'0');
        tm.tm_min   = (buff1[index++]-'0')*10;
        tm.tm_min  += (buff1[index++]-'0');
        tm.tm_sec   = (buff1[index++]-'0')*10;
        tm.tm_sec  += (buff1[index++]-'0');
        newtime = (mktime(&tm) + offset*60*60 - timezone);
    }
    return newtime;
}

} // anonymous namespace

} // namespace authorizer
} // namespace wmproxy
} // namespace wms
} // namespace glite

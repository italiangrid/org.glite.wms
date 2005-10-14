
#include <openssl/pem.h>
#include <vector>
#include <iostream>
#include <sstream> //string streams
#include <fstream> // file streams
#include <stdlib.h> // getenv(...)
#include <unistd.h> // getuid()
#include <sys/types.h> // getuid()
#include <ctype.h>
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"


namespace glite {
namespace wms {
namespace wmproxyapiutils {

using namespace std ;
using namespace glite::wms::wmproxyapi;
/*
* Gets the current time
* @return the seconds elapsed since Jan 1, 1970
*/
const time_t getTime( ){
	time_t now = time(NULL);
	struct tm *ns = localtime(&now);
	return mktime (ns);
}

/**
Checks the existance of the input pathname
**/
const char* checkPathExistence(const char* path){
	ifstream f_in(path);
	if (  !f_in.good() ) return NULL;
	else return path;
}

/*****************************************************************
Calculates the  Trusted Certificates directory pathnaìme
******************************************************************/
const char* getTrustedCert(ConfigContext *cfs){
	char *path = NULL ;
	if (cfs!=NULL){
        	if (cfs->trusted_cert_dir!=""){
                	path = (char*)checkPathExistence(cfs->trusted_cert_dir.c_str());
                 }
         } else {
		path = getenv ("X509_CERT_DIR");
                if ( !path ){
			path =(char*) checkPathExistence( "/etc/grid-security/certificates");
                }
         }
        return path ;
}
/*****************************************************************
Calculates the  Proxy file pathname
******************************************************************/
const char* getProxyFile(ConfigContext *cfs){
	char* path = NULL;
	if (cfs!=NULL){
        	if (cfs->proxy_file!=""){
                	path = (char*)checkPathExistence(cfs->proxy_file.c_str());
                 }
	 } else {
		const char * env_proxy = getenv ("X509_USER_PROXY");
		if(env_proxy!=NULL){
                	path = (char*)checkPathExistence(env_proxy);
		}else{
			// Append UID to X509 default file
			stringstream uid_string;
			uid_string << getuid() ;
			string result ="/tmp/x509up_u" + uid_string.str();
			path = (char*)checkPathExistence( result.c_str() );
		}
	}
        return path ;
}

/*****************************************************************
Calculates the EndPoint URL
******************************************************************/
const char* getEndPoint (ConfigContext *cfs){
	char* ep = NULL;
	if (cfs!=NULL){
		ep = (char*)(cfs->endpoint).c_str();
	 } 
        return ep ;
}

/*************************************************************************
	private method:	converts ASN1_UTCTIME to time_t
*************************************************************************/
time_t ASN1_UTCTIME_get(const ASN1_UTCTIME *s)
        {
        struct tm tm;
        int offset;
        memset(&tm,'\0',sizeof tm);
#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
        tm.tm_year=g2(s->data);
        if(tm.tm_year < 50)
                tm.tm_year+=100;
        tm.tm_mon=g2(s->data+2)-1;
        tm.tm_mday=g2(s->data+4);
        tm.tm_hour=g2(s->data+6);
        tm.tm_min=g2(s->data+8);
        tm.tm_sec=g2(s->data+10);
        if(s->data[12] == 'Z')
                offset=0;
        else
                {
                offset=g2(s->data+13)*60+g2(s->data+15);
                if(s->data[12] == '-')
                        offset= -offset;
                }
#undef g2

    return timegm(&tm)-offset*60;
}

/*************************************************************************
* gets the proxy time-left
*************************************************************************/
const long getProxyTimeLeft( std::string pxfile ) {
	time_t timeleft = 0;
	BIO *in = NULL;
	X509 *x = NULL;
	in = BIO_new(BIO_s_file());
	if (in) {
		BIO_set_close(in, BIO_CLOSE);
		if (BIO_read_filename( in, pxfile.c_str() ) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if (x) {
				timeleft = ( ASN1_UTCTIME_get   ( X509_get_notAfter(x))  - time(NULL) ) / 60 ;
			} else{
				throw  *createWmpException(new ProxyFileException, "getProxyTimeLeft",
					string ("unable to read X509 proxy file: "+ pxfile) ) ;
			}
		} else {

			throw  *createWmpException(new ProxyFileException, "getProxyTimeLeft",
				string ("unable to open X509 proxy file: "+ pxfile) ) ;
		}
		BIO_free(in);
		free(x);
	} else {
		throw  *createWmpException(new ProxyFileException, "getProxyTimeLeft",
				string ("unable to allocate memory for the proxy file: "+ pxfile) ) ;
	}
        return timeleft;
}

/*************************************************************************
* private method: checks if the vector v contains the "elem" string
*************************************************************************/
bool contains(const std::vector<std::string> &v, const std::string elem) {
	bool found = false;
	for (unsigned int i=0 ; i < v.size(); i++ ){
		if ( v[i] == elem  ){
			found = true;
			break;
		}
	}
	return found;
}

/*************************************************************************
* private method: parses the proxy info and extracts the fqan's
*************************************************************************/
const string parse(BIO *bp, unsigned char **pp, long length, int offset,
	     int depth, int indent, int dump, std::vector<std::string> &fv)
	{
	unsigned char *p,*ep,*tot,*op,*opp;
	long len;
	int tag,xclass;
	int nl,hl,j,r = 0;
	ASN1_OBJECT *o=NULL;
	ASN1_OCTET_STRING *os=NULL;
	int dump_indent;
	string fqan = "";

#if 0
	dump_indent = indent;
#else
	// Because we know BIO_dump_indent()
	dump_indent = 6;
#endif
	p= *pp;
	tot=p+length;
	op=p-1;
	while ((p < tot) && (op < p))
	{
		op=p;
		j=ASN1_get_object(&p,&len,&tag,&xclass,length);
		if (j & 0x80){
			break;
		}
		hl=(p-op);
		length-=hl;
		if (j & V_ASN1_CONSTRUCTED) {
			ep=p+len;
			if ((j == 0x21) && (len == 0)){
				for (;;) {
						fqan=parse(bp,&p,(long)(tot-p),
							offset+(p - *pp),depth+1,
							indent,dump, fv);
						if (fqan.size()>0){
							break;
						}
					if ((r == 0) ||  ((r == 2) || (p >= tot)) ){
						break;
					}
				}
			}else{
				while (p < ep) {
					fqan=parse(bp,&p,(long)len,
						offset+(p - *pp),depth+1,
						indent,dump, fv);
					if (fqan.size()>0){
							break;
					}
					if (r == 0) {
						break;
					}
				}
			}
		} else if (xclass != 0) {
			p+=len;
		} else {
			nl=0;
			 if (tag == V_ASN1_OCTET_STRING){
				int i,printable=1;
				opp=op;
				os=d2i_ASN1_OCTET_STRING(NULL,&opp,len+hl);
				if (os != NULL) {
					opp=os->data;
					for (i=0; i<os->length; i++) {
						if ((	(opp[i] < ' ') &&
							(opp[i] != '\n') &&
							(opp[i] != '\r') &&
							(opp[i] != '\t')) ||
							(opp[i] > '~')) {
								printable=0;
								break;
							}
					}
					if (printable && (os->length > 0)) {
						fqan = fqan.assign((char*)os->data);
						if (fqan.size()>0 && !contains(fv, fqan)){
							fv.push_back(fqan);
						}
					}
					M_ASN1_OCTET_STRING_free(os);
					os=NULL;
					}
				}
			p+=len;
			if ((tag == V_ASN1_EOC) && (xclass == 0)){
				break;
			}
		}
		length-=len;
	} // while

	if (o != NULL) {
		ASN1_OBJECT_free(o);
	}
	if (os != NULL) {
		M_ASN1_OCTET_STRING_free(os);
	}
	*pp=p;
	return(fqan);
}

/*************************************************************************
* looks for the list of fqan's
*************************************************************************/
const vector<std::string> getFQANs(std::string pxfile){
	int i = 0;
	int lasttag=-1;
	unsigned char *p1 ;
 	string fqan = "";
	FILE *fp = NULL ;
	X509 *cert = NULL ;
	BIO *bp = NULL;
	X509_EXTENSION *ex;
	ASN1_OBJECT *asnobject;
	ASN1_OCTET_STRING *asndata;
	vector<string> vect ;
	fp = fopen(pxfile.c_str(), "r");
	if (!fp){
		throw *createWmpException(new ProxyFileException, "getFQANs", string ("no such proxy file:" + pxfile) );
	}
	cert = PEM_read_X509(fp, NULL, NULL, NULL);
	if (!cert){
		throw  *createWmpException(new ProxyFileException, "getFQANs",
			string ("unable to read X509 proxy file: "+ pxfile) ) ;
	}
	fclose(fp);
   	bp = BIO_new(BIO_s_file());
	if (!bp){
		throw  *createWmpException(new ProxyFileException, "getFQANs",
			string ("ssl error - unable to read X509 proxy file: "+ pxfile) ) ;
	}
	BIO_set_close(bp, BIO_CLOSE);
	BIO_set_fp(bp,stdout,BIO_FP_TEXT);
 	for (i = 0; i < X509_get_ext_count(cert); ++i) {
		lasttag=-1;
       		ex = X509_get_ext(cert, i);
        	asnobject = X509_EXTENSION_get_object(ex);
        	asndata = X509_EXTENSION_get_data(ex);
        	p1 = ASN1_STRING_data(asndata);
		fqan =parse(bp, // bp
				&p1,//pp
				ASN1_STRING_length(asndata), // length
				0, // offset
				0, //depth
				0, //ndent
				0, //dump
				 vect);
		if (fqan.size ()>0){
			break;
		}
	}
	BIO_free(bp);
	return vect ;
}

} // wmproxy namespace
} // wms namespace
} // glite namespace


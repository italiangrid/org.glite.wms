/***************************************************************************
    filename  : NsWrapper.cpp
    begin     : Mon May 13
    author    : Annalisa Terracina
    email     : fpacini@datamat.it
    copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include <iostream>
#include "NsWrapper.h"
#include "glite/lb/producer.h"
#include <pem.h>
#include "openssl/ssl.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/jdl/JobAd.h"
#include "glite/wmsutils/jobid/manipulation.h"  // to_filename method
// DGAS implementation
#include "glite/dgas/hlr-clients/job_auth/jobAuthClient.h"

using namespace std ;
using namespace glite::wmsutils::exception ;
/************************************************
*   UserCredential VOMS implementation
*************************************************/
/******************************************************************
 method :load_chain
******************************************************************/
STACK_OF(X509) *load_chain(char *certfile, std::string& vo_error){
	STACK_OF(X509_INFO) *sk=NULL;
	STACK_OF(X509) *stack=NULL;
	BIO *in=NULL;
	X509_INFO *xi;
	int first = 1;
	if(!(stack = sk_X509_new_null())) {
		vo_error="Memory allocation failure";
		BIO_free(in);
		sk_X509_INFO_free(sk);
		return stack;
	}
	if(!(in=BIO_new_file(certfile, "r"))) {
		vo_error="Error opening proxy file";
		// goto end;
		BIO_free(in);
		sk_X509_INFO_free(sk);
		return stack;
	}
	// This loads from a file, a stack of x509/crl/pkey sets
	if(!(sk=PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
		vo_error="Error reading proxy file";
		// goto end;
		BIO_free(in);
		sk_X509_INFO_free(sk);
		return stack;
	}
	// Scan over it and pull out the certs
	while (sk_X509_INFO_num(sk)){
		//  skip first cert
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
		vo_error= "No certificates in proxy file";
		sk_X509_free(stack);
		BIO_free(in);
		sk_X509_INFO_free(sk);
		// goto end;
		return stack;
	}
	BIO_free(in);
	sk_X509_INFO_free(sk);
	return stack;
}


/** Constructor */
UserCredential::UserCredential ( const std::string& file){
	proxy_file = file ;
};
/** Default Dtor*/
UserCredential::~UserCredential()  { }  ;



/******************************************************************
private method:ASN1_UTCTIME_get (used by getExpiration)
*******************************************************************/
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

    return mktime(&tm)-offset*60;
    /* FIXME: mktime assumes the current timez one
     * instead of UTC, and unless we rewrite OpenSSL
     * in Lisp we cannot locally change the timezone
     * without possibly interfering with other parts
     * of the program. timegm, which uses UTC, is
     * non-standard.
     * Also time_t is inappropriate for general
     * UTC times because it may a 32 bit type.  */
}





/******************************************************************
private method:getIssuer
*******************************************************************/
std::string  UserCredential::getIssuer(){
	BIO *in = NULL;
	X509 *x = NULL;   // It is not used anymore. should be equal to pcd->ucert
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename(in, proxy_file.c_str()) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			// ISSUER
			char* texto ;
			texto = X509_NAME_oneline( X509_get_issuer_name  ( x )     , NULL , 200);
			string issuer (texto) ;
			OPENSSL_free(  texto  );
			return issuer ;
		}
	}
	return "" ;
}
/******************************************************************
private method:getExpiration
*******************************************************************/
int UserCredential::getExpiration(){
	BIO *in = NULL;
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename(in, proxy_file.c_str()) > 0)
			return ASN1_UTCTIME_get    (   X509_get_notAfter    (PEM_read_bio_X509(in, NULL, 0, NULL)  )    )    ;
	}
	return 0 ;
}

/******************************************************************
private method: load_voms
*******************************************************************/
int  UserCredential::load_voms (vomsdata& d){
	BIO  *in = NULL;
	X509 *x  = NULL;
	d.data.clear() ;
	STACK_OF(X509) *chain = NULL;
	SSLeay_add_ssl_algorithms();
	char *of   = const_cast<char *>(proxy_file.c_str());
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename(in, of) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if(!x){
				// Couldn't find a valid proxy.
				vo_data_error =VERR_FORMAT;
			}
			string vo_error;
			chain = load_chain(of, vo_error);
			d.SetVerificationType((verify_type)(VERIFY_SIGN | VERIFY_KEY));
			if (!d.Retrieve(x, chain, RECURSE_CHAIN)){
				d.SetVerificationType((verify_type)(VERIFY_NONE));
				if (d.Retrieve(x, chain, RECURSE_CHAIN)){
					// WARNING: Unable to verify signature!"
				}
			}
			sk_X509_free(chain);
		}else {
			// Couldn't find a valid proxy.
			vo_data_error =VERR_FORMAT;
		}
	}
	vo_data_error = d.error ;
	// Release memory
	BIO_free(in);
	return (vo_data_error==VERR_NONE);
}
/******************************************************************
 method:load_groups
 this private method is used by getDefaultGroups and getGroups methods
*******************************************************************/
vector <string> load_groups( voms &v ){
	// check the data
	vector <string> vect ; // contains the groups
	if (  v.type!=TYPE_STD  ){
		return vect ;
	}
	// appending group names
	for (  vector<data>::iterator i = v.std.begin() ; i!= v.std.end() ; i++  ) vect.push_back ( string ( i->group )  );
	return vect ;
}


/******************************************************************
 method: getDefaultVoName
*******************************************************************/
std::string  UserCredential::getDefaultVoName (){
	vomsdata vo_data ;
	if (load_voms(  vo_data )  ) return "";
	voms v;
	// get Default voms
	if (   !vo_data.DefaultData(  v  )   ){
		vo_data_error = vo_data.error ;
		return "" ;
	}
	return string (  v.voname );
};



/******************************************************************
 method: getDefaultVoName
*******************************************************************/
std::string  UserCredential::getDefaultFQAN (){
	vomsdata vo_data ;
	if (load_voms(  vo_data )  ) return "";
	voms v;
	// get Default voms
	if (   !vo_data.DefaultData(  v  )   ){
		vo_data_error = vo_data.error ;
		return "" ;
	}
	std::vector<string> fqans = v.fqan;
	if (fqans.size()){
		return fqans[0];
	}
	else return "";
}


/******************************************************************
 method: getVoNames
*******************************************************************/

std::vector <std::string> UserCredential::getVoNames (){
	vector <string> vect ; // contains the vo names
	vomsdata vo_data ;
	if (   load_voms(  vo_data )  ) return vect ;
	vector <voms> v = vo_data.data;
	for ( vector<voms>::iterator it = v.begin() ; it!= v.end() ; it++ )  vect.push_back ( string ( it->voname ) ) ;
	return vect ;
};

/******************************************************************
 method: getGroups
*******************************************************************/
std::vector <std::string> UserCredential::getGroups ( const std::string& voname ) {
	vomsdata vo_data ;
	vector<string> empty ;
	if (   load_voms(  vo_data ) )
		return  empty;
	vector <voms> vect = vo_data.data;
	for ( vector<voms>::iterator it = vect.begin() ; it!= vect.end() ; it++ )
		if ( voname == it->voname ) return load_groups( *it  ) ;
	// If this step is reached then the searched voms has not been found
	vo_data_error = VERR_FORMAT ;
	return empty ;
}
/******************************************************************
 method: getDefaultGroups
*******************************************************************/
std::vector <std::string > UserCredential::getDefaultGroups (){
	vomsdata vo_data ;
	vector<string> empty ;
	if (   load_voms(  vo_data )   )
		return empty ;
	voms v;
	// get Default voms
	if (!vo_data.DefaultData(v)) {
		vo_data_error = vo_data.error ;
		return empty ;
	}
	return load_groups( v ) ;
}
/******************************************************************
 method: containsVo
*******************************************************************/
bool UserCredential::containsVo ( const std::string& voname ) {
	vomsdata vo_data ;
	if (   load_voms(  vo_data ) )
		return false;
	vector <voms> v = vo_data.data;
	for ( vector<voms>::iterator it = v.begin() ; it!= v.end() ; it++ )
		if ( voname == it->voname ) return true ;
	return false ;
}
/******************************************************************
 method: get_error
*******************************************************************/
string UserCredential::get_error() {

	switch ( vo_data_error ){
		case  VERR_NONE:
			return "No Error Found" ;
		case VERR_NOEXT:
			return "Unable to find VOMS Extensions inside credential" ;
		case VERR_SIGN:
			return "Unable to verify proxy credential signature. Cannot find VOMS Server certificate";
		case VERR_NODATA:
			return "Unable to find VOMS extension";
		case VERR_NOSOCKET:
			return "VOMS Socket Problems during connection";
		case VERR_FORMAT:
			return "VOMS format error" ;
		case VERR_DIR:
			return "Unable to find VOMS directory" ;
		default:
			return "Generic VOMS error found" ;
	}
}



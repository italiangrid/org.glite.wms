/* **************************************************************************
*  filename  : UserCredential.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include <unistd.h>
#include "glite/wmsui/api/UserCredential.h"
#include "CredentialException.h"
#include <sys/types.h> // in order to get pid
#include <globus_gsi_system_config.h>  //GLOBUS_GSI_SYSCONFIG_CHECK_KEYFILE
#include <globus_gsi_proxy.h>  // GLOBUS_GSI_PROXY_MODULE PEM_read_bio_X509
#include "glite/security/voms/voms_api.h"  // credential info
namespace glite {
namespace wmsui {
namespace api {
using namespace glite::wmsutils::exception ;
using namespace std ;
//   Initialize static mutex variable
pthread_mutex_t UserCredential::mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************
private method:ASN1_UTCTIME_get (used by getTimeLeft)
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
private method: getProxy
*******************************************************************/
const char* getProxy ( const string &cred_path){
	char* result ;
	if (cred_path==""){
		if (GLOBUS_GSI_SYSCONFIG_GET_PROXY_FILENAME
			(&result,GLOBUS_PROXY_FILE_INPUT)!=GLOBUS_SUCCESS){
			throw   CredProxyException(__FILE__,__LINE__,"getProxy",WMS_CRED,"determine");
		}
		return result ;
	}else{ return cred_path.c_str();}

}
/******************************************************************
Default Constructor*
*******************************************************************/
UserCredential::UserCredential(){
	proxy_file = NULL;
}
/******************************************************************
Default Destructor
*******************************************************************/
UserCredential::~UserCredential(){
	if (proxy_file){free(proxy_file);}
}
/******************************************************************
 method: checkProxy
*******************************************************************/
int UserCredential::checkProxy (const string& cred_path){
	GLITE_STACK_TRY("checkProxy (const string& cred_path)")   ;
	int i =  getTimeLeft(cred_path) ;
	if (i  <  PROXY_TIME_LIMIT   ) { throw  ProxyException ( __FILE__ , __LINE__ , METHOD ); }
	return i ;
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
}
/******************************************************************
 method: getIssuer
*******************************************************************/
string UserCredential::getIssuer (const string& cred_path){
	BIO *in = NULL;
	X509 *x = NULL;   // It is not used anymore. should be equal to pcd->ucert
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename( in, getProxy(cred_path)) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			// ISSUER
			char* texto ;
			texto = X509_NAME_oneline( X509_get_issuer_name  ( x )     , NULL , 200);
			string issuer (texto) ;
			OPENSSL_free(  texto  );
			return issuer ;
		}
	}
	// This line is reached only when unable to find/load proxy certificate
	throw  ProxyException ( __FILE__ , __LINE__ , "UserCredential::getIssuer" );
}
/******************************************************************
 method: getSubject
*******************************************************************/
string UserCredential::getSubject     (const string& cred_path){
	BIO *in = NULL;
	X509 *x = NULL;   // It is not used anymore. should be equal to pcd->ucert
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename( in, getProxy(cred_path)) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			// ISSUER
			char* texto ;
			texto = X509_NAME_oneline( X509_get_subject_name  ( x )     , NULL , 200);
			string issuer (texto) ;
			OPENSSL_free(  texto  );
			return issuer ;
		}
	}
	// This line is reached only when unable to find/load proxy certificate
	throw  ProxyException ( __FILE__ , __LINE__ , "UserCredential::getSubject" );
};
/******************************************************************
 method: getCredType: DEPRECATED
*******************************************************************/
int UserCredential::getCredType    (const string& cred_path) {
	throw  ProxyException ( __FILE__ , __LINE__ , "DEPRECATED use of UserCredential::getCredType" );
}
/******************************************************************
 method: getStrenght DEPRECATED
*******************************************************************/
int UserCredential::getStrenght    (const string& cred_path) {
	throw  ProxyException ( __FILE__ , __LINE__ , "DEPRECATED use of UserCredential::getStrenght" );
};
/******************************************************************
 method: getTimeLeft
*******************************************************************/
int UserCredential::getTimeLeft (const string& cred_path) {
	BIO *in = NULL;
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename( in, getProxy(cred_path)) > 0) {
			return ASN1_UTCTIME_get    (   X509_get_notAfter    (PEM_read_bio_X509(in, NULL, 0, NULL)  )    )    ;
		}
	}
	// This line is reached only when unable to find/load proxy certificate
	throw  ProxyException ( __FILE__ , __LINE__ ,"UserCredential::getTimeLeft");
}

/******************************************************************
 method: getInfo DEPRECATED
*******************************************************************/
void UserCredential::getInfo(string& subj, string& issuer, int& cred_type, int& strength, int& time_left, const string& cred_path ) {
	throw  ProxyException ( __FILE__ , __LINE__ , "DEPRECATED use of UserCredential::getInfo");
};

/******************************************************************
 method:  destroy
*******************************************************************/
void UserCredential::destroy(const string& cred_path ) {
	GLITE_STACK_TRY("destroy(const string& cred_path ) ");
	pthread_mutex_lock( &mutex);  //   LOCK RESOURCES
	remove(  getProxy ( cred_path)   );
	pthread_mutex_unlock( &mutex);  //  UNLOCK RESOURCES
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
		VOMS IMPLEMENTATION
*******************************************************************/
/******************************************************************
 method :load_chain
*******************************************************************/
STACK_OF(X509) *load_chain(char *certfile)
{
	STACK_OF(X509_INFO) *sk=NULL;
	STACK_OF(X509) *stack=NULL, *ret=NULL;
	BIO *in=NULL;
	X509_INFO *xi;
	int first = 1;
	if(!(stack = sk_X509_new_null())) {
		BIO_free(in);
		sk_X509_INFO_free(sk);
		throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "");
	}
	if(!(in=BIO_new_file(certfile, "r"))) {
		BIO_free(in);
		sk_X509_INFO_free(sk);
		throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "" );
	}
	// This loads from a file, a stack of x509/crl/pkey sets
	if(!(sk=PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
		BIO_free(in);
		sk_X509_INFO_free(sk);
		throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "" );
	}
  /* scan over it and pull out the certs */
  while (sk_X509_INFO_num(sk)) {
    /* skip first cert */
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
    printf("no certificates in file, %s\n",certfile);
    sk_X509_free(stack);
  BIO_free(in);
  sk_X509_INFO_free(sk);
    // goto end;
    throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "" );
  }
  ret=stack;
  return(ret);
}

/******************************************************************
private method: load_voms
*******************************************************************/
void UserCredential::load_voms ( vomsdata& d  ){
	string METHOD("load_voms(vomsdata vo)");
	if   (proxy_file ==NULL){
		if (GLOBUS_GSI_SYSCONFIG_GET_PROXY_FILENAME( &proxy_file, GLOBUS_PROXY_FILE_INPUT) != GLOBUS_SUCCESS ){
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "determine" );
		}
	}
	d.data.clear() ;
	BIO *in = NULL;
	X509 *x = NULL;
	in = BIO_new(BIO_s_file());
	SSLeay_add_ssl_algorithms();
	if (in) {
		if (BIO_read_filename(in,proxy_file)>0){
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			if(!x){
				throw CredProxyException(__FILE__ , __LINE__ ,
					METHOD,  WMS_VO_LOAD  , "read bio" );
			}
			STACK_OF(X509) *chain = load_chain(proxy_file);
			d.SetVerificationType((verify_type)(VERIFY_SIGN|VERIFY_KEY));
			if (!d.Retrieve( x , chain, RECURSE_CHAIN)  ){
				d.SetVerificationType((verify_type)(VERIFY_NONE));
				if (d.Retrieve(x, chain, RECURSE_CHAIN)){
					throw CredProxyException(__FILE__,__LINE__,
						METHOD,WMS_VO_LOAD,"retrieve");
				}
			}
			sk_X509_free(chain);
		} else{
			throw CredProxyException(__FILE__,__LINE__ ,
				METHOD,WMS_VO_LOAD,"read BIO filename");
		}
	} else {
		throw CredProxyException(__FILE__,__LINE__ ,
			METHOD,WMS_VO_LOAD,"read in");
	}
	if (d.error!=VERR_NONE){
		// throw CredProxyException(__FILE__,__LINE__ , METHOD,WMS_VO_LOAD,"parse");
	}
	// Release memory
	BIO_free(in);
}


/******************************************************************
 STATIC method: getDefaultVoName
 this private method is used by getDefaultGroups and getGroups methods
*******************************************************************/
vector <string> UserCredential::load_groups( voms &v ){
	// check the data
	if (  v.type!=TYPE_STD  )
		throw CredProxyException(__FILE__ , __LINE__ , "load_groups" ,WMS_VO_TYPE, v.voname );
	// appending group names
	vector <string> vect ; // contains the groups
	for (  vector<data>::iterator i = v.std.begin() ; i!= v.std.end() ; i++  ){
		vect.push_back ( string ( i->group )  );
	}
	return vect ;
}


/******************************************************************
 method: getDefaultVoName
*******************************************************************/
std::string  UserCredential::getDefaultVoName (){
	vomsdata vo_data ;
	load_voms(  vo_data ) ;
	voms v;
	// get Default voms
	if (   !vo_data.DefaultData(  v  )   )
		throw CredProxyException(__FILE__ , __LINE__ ,"getDefaultVoName", WMS_VO_DEFAULT  , "DefaultData" );
	return string (  v.voname );
};


/******************************************************************
 method: getVoNames
*******************************************************************/

std::vector <std::string> UserCredential::getVoNames (){
	vomsdata vo_data ;
	load_voms(  vo_data ) ;
	vector <string> vect ; // contains the vo names
	vector <voms> v = vo_data.data;
	for ( vector<voms>::iterator it = v.begin() ; it!= v.end() ; it++ ){
		vect.push_back ( string ( it->voname ) ) ;
	}
	return vect ;
};

/******************************************************************
 method: getGroups
*******************************************************************/
std::vector <std::string> UserCredential::getGroups ( const std::string& voname ) {
	vomsdata vo_data ;
	load_voms(  vo_data ) ;
	vector <voms> vect = vo_data.data;
	for ( vector<voms>::iterator it = vect.begin() ; it!= vect.end() ; it++ )
		if ( voname == it->voname ) return load_groups( *it  ) ;
	// If this step is reached then the searched voms has not been found
	throw CredProxyException(__FILE__ , __LINE__ ,"load_groups" , WMS_VO_TYPE  , "voname" );
}
/******************************************************************
 method: getDefaultGroups
*******************************************************************/
std::vector <std::string > UserCredential::getDefaultGroups (){
	vomsdata vo_data  ;
	load_voms(  vo_data ) ;
	voms v;
	// get Default voms
	if (!vo_data.DefaultData(v))
		throw CredProxyException(__FILE__ , __LINE__ ,"getDefaultGroups" , WMS_VO_DEFAULT  , "" );
	return load_groups( v ) ;

}
/******************************************************************
 method: containsVo
*******************************************************************/
bool UserCredential::containsVo ( const std::string& voname ) {
	try{
		vomsdata vo_data ;
		load_voms(  vo_data ) ;
		vector <voms> v = vo_data.data;
		for ( vector<voms>::iterator it = v.begin() ; it!= v.end() ; it++ )
			if ( voname == it->voname ) return true ;
	}catch (std::exception &exc) { /** Do not raise any exception but return false*/return false ; }
	return false ;
}

} // api
} // wmsui
} // glite



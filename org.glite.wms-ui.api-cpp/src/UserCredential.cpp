/* **************************************************************************
*  filename  : UserCredential.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include <unistd.h>
#include "glite/wmsui/api/UserCredential.h"
#include "CredentialException.h"
#include <sys/types.h> // in order to get pid
#include <iostream>  //cout

#include <globus_openssl.h>  // globus_result_t
#include <globus_gsi_system_config.h>  //GLOBUS_GSI_SYSCONFIG_CHECK_KEYFILE
#include <globus_gsi_credential.h>  // globus_gsi_cred_handle
#include <globus_gsi_proxy.h>  // GLOBUS_GSI_PROXY_MODULE
#include <openssl/safestack.h>  // sk_X509_new_null

namespace glite {
namespace wmsui {
namespace api {

using namespace glite::wmsutils::exception ;
using namespace std ;
//   Initialize static mutex variable
pthread_mutex_t UserCredential::mutex = PTHREAD_MUTEX_INITIALIZER;
/******************************************************************
 method: checkProxy
*******************************************************************/
 int UserCredential::checkProxy (const string& cred_path) {
     GLITE_STACK_TRY("checkProxy (const string& cred_path)")   ;
     int i =  getTimeLeft(cred_path) ;
     if (i  <  PROXY_TIME_LIMIT   )
                  throw  ProxyException ( __FILE__ , __LINE__ , METHOD );
     return i ;
   GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 };

/******************************************************************
 method: getIssuer
*******************************************************************/
string UserCredential::getIssuer      (const string& cred_path){
     GLITE_STACK_TRY("getIssuer      (const string& cred_path)")   ;
     string subj , issuer;
     int cred_type, strength, time_left;
     getInfo ( subj,  issuer,  cred_type,  strength,  time_left);
     return issuer ;
    GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};
/******************************************************************
 method: getSubject
*******************************************************************/
 string UserCredential::getSubject     (const string& cred_path){
     GLITE_STACK_TRY("getSubject     (const string& cred_path) ");
     string subj ;
     string issuer;
     int cred_type, strength, time_left;
     getInfo ( subj,  issuer,  cred_type,  strength,  time_left);
     return issuer ;
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
  };
/******************************************************************
 method: getCredType
*******************************************************************/
 int UserCredential::getCredType    (const string& cred_path) {
     GLITE_STACK_TRY("getCredType    (const string& cred_path)");
     string subj , issuer;
     int cred_type, strength, time_left;
     getInfo ( subj,  issuer,  cred_type,  strength,  time_left);
     return  cred_type;
    GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
  };
/******************************************************************
 method: getStrenght
*******************************************************************/
 int UserCredential::getStrenght    (const string& cred_path) {
     GLITE_STACK_TRY("getStrenght    (const string& cred_path) ");
     string subj , issuer;
     int cred_type, strength, time_left;
     getInfo ( subj,  issuer,  cred_type,  strength,  time_left);
     return  strength;
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
 };
/******************************************************************
 method: getTimeLeft
*******************************************************************/
 int UserCredential::getTimeLeft (const string& cred_path) {
     GLITE_STACK_TRY("getTimeLeft (const string& cred_path)");
     string subj , issuer;
     int cred_type, strength, time_left;
     getInfo ( subj,  issuer,  cred_type,  strength,  time_left);
     return  time_left;
     GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
  };

/******************************************************************
 method: getInfo
*******************************************************************/
void UserCredential::getInfo(string& subj, string& issuer, int& cred_type, int& strength, int& time_left, const string& cred_path ) {
	GLITE_STACK_TRY("getInfo(string& subj, string& issuer, int& cred_type, int& strength, int& time_left, const string& cred_path ) ");
	pthread_mutex_lock( &mutex);  //LOCK RESOURCES
	try{
		proxy_file = NULL;
		globus_gsi_cred_handle_t  proxy_cred = NULL ;
		EVP_PKEY     *proxy_pubkey= NULL ;
		// globus_gsi_cert_utils_cert_type_t   cert_type;
		char *tmp ;
		time_t                lifetime;

		// Activation modesl:
		globus_module_activate(GLOBUS_OPENSSL_MODULE) ;
		globus_module_deactivate(GLOBUS_GSI_PROXY_MODULE);
		globus_module_activate(GLOBUS_GSI_CREDENTIAL_MODULE) ;
		globus_module_activate(GLOBUS_GSI_SYSCONFIG_MODULE );

		// Check the file:
		if (cred_path !=""){
			proxy_file = (char *)cred_path.c_str();
			if (GLOBUS_GSI_SYSCONFIG_CHECK_KEYFILE( proxy_file , NULL ) != GLOBUS_SUCCESS)   // Warning: NULL pointer passed
				throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "determine" );
		}else{
			if (GLOBUS_GSI_SYSCONFIG_GET_PROXY_FILENAME( &proxy_file, GLOBUS_PROXY_FILE_INPUT) != GLOBUS_SUCCESS )
				throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "determine" );
		}
		if ( globus_gsi_cred_handle_init(&proxy_cred, NULL) != GLOBUS_SUCCESS )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "initialize" );
		if ( globus_gsi_cred_read_proxy(proxy_cred, proxy_file) !=GLOBUS_SUCCESS)
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "get" );
		// subject
		if    (globus_gsi_cred_get_subject_name(proxy_cred, &tmp) != GLOBUS_SUCCESS  )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read subject" );
		subj = string (tmp) ;
	#ifdef WIN32
		OPENSSL_free(tmp);
	#else
		free(tmp);
	#endif
		// issuer
		if (globus_gsi_cred_get_issuer_name(proxy_cred, &tmp) != GLOBUS_SUCCESS  )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read issuer" );
		issuer = string (tmp) ;
	#ifdef WIN32
		OPENSSL_free(tmp);
	#else
		free(tmp);
	#endif
		// validity
		if (globus_gsi_cred_get_lifetime(proxy_cred, &lifetime ) != GLOBUS_SUCCESS  )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read validity" );
		time_left = (int) lifetime ;

		if (globus_gsi_cred_get_key(proxy_cred, &proxy_pubkey ) != GLOBUS_SUCCESS  )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read key" );

		// Strength
		strength = 8 * EVP_PKEY_size(proxy_pubkey);

		// type: restricted, limited or full
/*
		globus_gsi_cert_utils_proxy_type_t cert_type ;
		if ( globus_gsi_cred_check_proxy(proxy_cred, &cert_type) !=GLOBUS_SUCCESS)
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read type" );
*/

		globus_gsi_cred_handle_t  handle ;
		globus_gsi_cert_utils_cert_type_t  cert_type ;

		if (  globus_gsi_cred_get_cert_type ( handle , &cert_type ) !=GLOBUS_SUCCESS )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "read type" );

		cred_type = (int) cert_type ;
		globus_module_deactivate(GLOBUS_OPENSSL_MODULE);
		globus_module_deactivate(GLOBUS_GSI_PROXY_MODULE);
		globus_module_deactivate(GLOBUS_GSI_CREDENTIAL_MODULE) ;
		pthread_mutex_unlock( &mutex);  //   UNLOCK RESOURCES
	}catch (CredProxyException &exc){
		pthread_mutex_unlock( &mutex);  //   UNLOCK RESOURCES
		throw exc ;
	}
	GLITE_STACK_CATCH() ; //Exiting from method: remove line from stack trace
};

/******************************************************************
 method:  destroy
*******************************************************************/
void UserCredential::destroy(const string& cred_path ) {
	GLITE_STACK_TRY("destroy(const string& cred_path ) ");
	string proxy_path;
	char uid [128] ;
	sprintf (uid, "%i" , getuid() ) ;
	if (cred_path != "")
			//  NO Cred Path Specified
			proxy_path = getenv("X509_USER_PROXY") ;
	else
			//  Cred Path Specified
			proxy_path = cred_path ;
/*
	if (proxy_path == ""){   //TBD
		// X509_USER_PROXY not set
		proxy_path = string(DEFAULT_SECURE_TMP_DIR) +
				string(FILE_SEPERATOR) +
				string(X509_USER_PROXY_FILE) +
				string (uid) ;
	}
*/
	pthread_mutex_lock( &mutex);  //   LOCK RESOURCES
	if (proxy_path == "") remove(proxy_path.c_str() );
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
    printf("memory allocation failure\n");
  BIO_free(in);
  sk_X509_INFO_free(sk);
    throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "");
  }

  if(!(in=BIO_new_file(certfile, "r"))) {
    printf("error opening the file, %s\n",certfile);
    // goto end;
  BIO_free(in);
  sk_X509_INFO_free(sk);
    throw CredProxyException(__FILE__ , __LINE__ , "load_chain" ,WMS_VO_TYPE, "" );
  }

  /* This loads from a file, a stack of x509/crl/pkey sets */
  if(!(sk=PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
    printf("error reading the file, %s\n",certfile);
    // goto end;
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
private method: load_voms
*******************************************************************/
// void UserCredential::load_voms (){
void UserCredential::load_voms ( vomsdata& d  ){
	string METHOD("load_voms(vomsdata vo)");
	if   (proxy_file ==NULL)
		if (GLOBUS_GSI_SYSCONFIG_GET_PROXY_FILENAME( &proxy_file, GLOBUS_PROXY_FILE_INPUT) != GLOBUS_SUCCESS )
			throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "determine" );

/* OLD IMPLEMENTATION
	if (proxy_file ==""){
		proxy_get_filenames(pcd, 1, NULL, NULL, &proxy_file, NULL, NULL);
		if (!proxy_file)  throw   CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_CRED  , "determine" );
	}
*/
	d.data.clear() ;
	BIO *in = NULL;
	X509 *x = NULL;   // It is not used anymore. should be equal to pcd->ucert
	in = BIO_new(BIO_s_file());
	if (in) {
		if (BIO_read_filename(in, proxy_file) > 0) {
			x = PEM_read_bio_X509(in, NULL, 0, NULL);
			STACK_OF(X509) *chain = load_chain(  proxy_file );
			// if ( d.Retrieve( pcd->ucert , chain, RECURSE_CHAIN)  )
			if ( d.Retrieve( x , chain, RECURSE_CHAIN)  ){
				return ;
			}
			//  if this point is reached then exception is thrown
			throw CredProxyException(__FILE__ , __LINE__ ,METHOD,  WMS_VO_LOAD  , "retrieve" );
		}
		throw CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_VO_LOAD  , "BIO_read_filename" );
	}
	throw CredProxyException(__FILE__ , __LINE__ ,METHOD, WMS_VO_LOAD  , "BIO_new" );
} ;


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



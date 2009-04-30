/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE utility functions
 *
 * Authors: Moreno Marzolla <moreno.marzolla@pd.infn.it>
 *          Alvise Dorigo <alvise.dorigo@pd.infn.it>
 */

#include "iceUtils.h"
#include "iceConfManager.h"

#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <cerrno>
#include <vector>
#include <ctype.h>
#include <cstdio>
#include <sstream>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "boost/thread/recursive_mutex.hpp"
#include <boost/format.hpp>
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

extern int h_errno;
extern int errno;

int setET(char **errtxt, int rc);


using namespace std;
namespace api_util = glite::ce::cream_client_api::util;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	pair<bool, time_t> isvalid( const std::string& proxyfile ) throw() {
	  
	  X509* x;
	  try {
	    x = glite::ce::cream_client_api::certUtil::read_BIO(proxyfile);
	  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex) {
	    CREAM_SAFE_LOG(
			   glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()->errorStream()
			   << "util::isvalid() - " << ex.what();
			   )
	      return make_pair(false, (time_t)0);
	  }
	  
	  boost::shared_ptr< X509 > tmpIn( x, X509_free );
	  
	  time_t proxyTimeEnd = glite::ce::cream_client_api::certUtil::ASN1_UTCTIME_get( X509_get_notAfter(x) );
	  
	  if( proxyTimeEnd > time(0)+5 ) return make_pair(true, proxyTimeEnd);
	  return make_pair(false, (time_t)0);
	}

	
	/**
	 * Utility function which converts a binary blob into a string.
	 *
	 * @param buf The binary data blob to convert
	 *
	 * @param len The length of the binary data bloc buf
	 *
	 * @return a string univocally representing the buffer buf. The
	 * string contains a list of exactly (len * 2) hex characters, as
	 * follows.  If the blob contains the following bytes (in hex):
	 *
	 * 0xfe 0xa0 0x01 0x90
	 *
	 * Then the resulting string will be the following:
	 *
	 * "fea00190"
	 */ 
	string bintostring( unsigned char* buf, size_t len ) {
	  string result;
	  const char alpha[] = "0123456789abcdef";
	  
	  for ( size_t i=0; i<len; ++i ) {
            result.push_back( alpha[ ( buf[i] >> 4 ) & 0x0f ] );
            result.push_back( alpha[ buf[i] & 0x0f ] );
	  }
	  return result;
	};

string computeSHA1Digest( const string& proxyfile ) throw(runtime_error&) {

  static char* method_name = "util::computeSHA1Digest() - ";

  unsigned char bin_sha1_digest[SHA_DIGEST_LENGTH];
  char buffer[ 1024 ]; // buffer for file data
  SHA_CTX ctx;
  int fd; // file descriptor
  unsigned long nread = 0; // number of bytes read

  fd = open( proxyfile.c_str(), O_RDONLY );
  
  if ( fd < 0 ) {
    int saveerr = errno;
    CREAM_SAFE_LOG( api_util::creamApiLogger::instance()->getLogger()->errorStream()
		    << method_name
		    << "Cannot open proxy file ["
		    << proxyfile << "]: " << strerror(saveerr)
		    );  
    throw runtime_error( string( "Cannot open proxy file [" + proxyfile + "]: " + strerror(saveerr) ) );
  }
  
  SHA1_Init( &ctx );
  while ( ( nread = read( fd, buffer, 1024 ) ) > 0 ) {
    SHA1_Update( &ctx, buffer, nread );
  }
  SHA1_Final( bin_sha1_digest, &ctx );
  
  close( fd );

  return bintostring( bin_sha1_digest, SHA_DIGEST_LENGTH );
}

//________________________________________________________________________
string getHostName( void ) throw ( runtime_error& )
{
    char name[256];
    
    if ( ::gethostname(name, 256) == -1 ) { // FIXME: is it thread safe ?
        throw runtime_error( string( "Could not resolve local hostname: ") 
                             + string(strerror(errno) ) );
    }

    struct addrinfo * result;
    struct addrinfo * res;
    int error;
    
    /* resolve the domain name into a list of addresses */
    error = getaddrinfo(name, NULL, NULL, &result);
    
    if (0 != error)
      {
	//perror("error in getaddrinfo: ");
	//return "UnresolvedHost";
	throw runtime_error( string( "Could not resolve local hostname: ") 
			     +string(strerror(errno)));
      }
    
    if (result == NULL)
      {
	throw runtime_error( string( "Could not resolve local hostname: ") 
			     +string(strerror(errno)));
      }
    
    string myname = "UnresolvedHost";
    
    for (res = result; res != NULL; res = res->ai_next)
      {
	char hostname[NI_MAXHOST] = "";
	
	error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
	
	if (0 != error)
	  {
	    //fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
	    continue;
	  }
	
	if (*hostname)
	  {
	    //printf("hostname: %s\n", hostname);
	    myname = hostname;
	    break;
	  }
	
      }
 
    if( myname == "UnresolvedHost" )
      {
	freeaddrinfo(result);
	throw runtime_error( string( "Could not resolve local hostname for an unknown reason"));
      }

    freeaddrinfo(result);

    //cout << "getHostName() - RETURNING myname=[" << myname << "]"<<endl;

    return myname;

}

//________________________________________________________________________
string getURL( void ) throw ( runtime_error& )
{
  string tmp_myname, tmp_prefix;
  
  try {
        tmp_myname = getHostName();        
  } catch(runtime_error& ex) {
    throw ex;
  }

  if( iceConfManager::getInstance()->getConfiguration()->ice()->listener_enable_authn() )
        tmp_prefix = "https";
  else
        tmp_prefix = "http";
    
  string url = boost::str( boost::format("%1%://%2%:%3%") % tmp_prefix % tmp_myname % iceConfManager::getInstance()->getConfiguration()->ice()->listener_port() );
  return url;
}

//________________________________________________________________________
string time_t_to_string( time_t tval ) {
    char buf[26]; // ctime_r wants a buffer of at least 26 bytes
    ctime_r( &tval, buf );
    if(buf[strlen(buf)-1] == '\n')
      buf[strlen(buf)-1] = '\0';
    return string( buf );
}

//________________________________________________________________________
string int_to_string( const int val ) {
  ostringstream os("");
  os << val;
  return os.str();
}

//________________________________________________________________________
void makePath(const string& filename) throw(exception&)
{
  boost::filesystem::path tmpFile( filename, boost::filesystem::native );
  boost::filesystem::path parent = tmpFile.branch_path();
  string currentPath("");
  try {
    if( !boost::filesystem::exists( parent ) )
    {
      for(boost::filesystem::path::iterator it=parent.begin();
	it != parent.end();
	++it)
      {
	currentPath += (*it) + "/";
	while(currentPath.find("//", 0) != string::npos)
	  boost::replace_first( currentPath, "//", "/");
	boost::filesystem::path tmpPath( currentPath, boost::filesystem::native );
	if( !boost::filesystem::exists( tmpPath ) )
	  {
	    boost::filesystem::create_directory( tmpPath );
	  }
      }
    } 
  } catch( std::exception& ex) {
    throw;
  }
}

namespace {
  
  class canonizerObject {
    string target;

  public:
    canonizerObject() : target("") {}

    ~canonizerObject() throw() {}

    void operator()( const char c ) throw() {
      if(isalnum((int)c)) {
	target.append( 1, c );
      } else {
	char tmp[16];
	sprintf( tmp, "%X", c );
	target.append( tmp );
      }
    } // end operator()
    
    string getString( void ) const { return target; }
  };

};

//______________________________________________________________________________
string canonizeString( const string& aString ) throw()
{
  canonizerObject c;
  c = for_each(aString.begin(), aString.end(), c);
  return c.getString();
}

//______________________________________________________________________________
string compressed_string( const string& name ) {
  string result;
  unsigned char buf[ SHA_DIGEST_LENGTH ]; // output buffer
  const unsigned char idx[ 17 ] = "0123456789ABCDEF"; // must be 17 chars, as the trailing \0 counts
  SHA1( (const unsigned char*)name.c_str(), name.length(), buf ); // stores SHA1 hash in buf
  for ( int i=0; i<SHA_DIGEST_LENGTH; ++i ) {
    unsigned char to_append;
    // left nibble;
    to_append = idx[ ( buf[i] & 0xf0 ) >> 4 ];
    result.push_back( to_append );
    // right nibble
    to_append = idx[ buf[i] & 0x0f ];
    result.push_back( to_append );
  }
  return result;
}

} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

//________________________________________________________________________
int setET(char **errtxt, int rc)
{
    if (rc) *errtxt = strerror(rc);
       else *errtxt = (char *)"unexpected error";
    return 0;
}

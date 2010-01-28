/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.
See the License for the specific language governing permissions and
limitations under the License.

END LICENSE */

#include "iceUtils.h"
#include "iceConfManager.h"
#include "ice-core.h"
#include "Request.h"

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

#include "boost/algorithm/string.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/format.hpp"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include "classad_distribution.h"

//extern int h_errno;
//extern int errno;

int setET(char **errtxt, int rc);

using namespace std;
namespace api_util   = glite::ce::cream_client_api::util;
namespace ceurl_util = glite::ce::cream_client_api::util::CEUrl;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	//______________________________________________________________________
	void full_request_unparse(Request* request,
				  CreamJob& theJob,
				  std::string& commandStr )
	  throw(ClassadSyntax_ex&, JobRequest_ex&)
	{
	  string protocolStr;
	  string jdl;
	  {// Classad-mutex protected region  
	    boost::recursive_mutex::scoped_lock M_classad( glite::wms::ice::Ice::ClassAd_Mutex );
	    
	    classad::ClassAdParser parser;
	    classad::ClassAd *rootAD = parser.ParseClassAd( request->to_string() );
	    
	    if (!rootAD) {
	      throw ClassadSyntax_ex( boost::str( boost::format( "iceCommandSubmit: ClassAd parser returned a NULL pointer parsing request: %1%" ) % request->to_string() ) );        
	    }
	    
	    boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( rootAD );
	    
	    // Parse the "command" attribute
	    if ( !classad_safe_ptr->EvaluateAttrString( "command", commandStr ) ) {
	      throw JobRequest_ex( boost::str( boost::format( "iceCommandSubmit: attribute 'command' not found or is not a string in request: %1%") % request->to_string() ) );
	    }
	    boost::trim_if( commandStr, boost::is_any_of("\"") );
	    
	    if ( !boost::algorithm::iequals( commandStr, "submit" ) ) {
	      throw JobRequest_ex( boost::str( boost::format( "iceCommandSubmit:: wrong command parsed: %1%" ) % commandStr ) );
	    }
	    
	    // Parse the "version" attribute
	    if ( !classad_safe_ptr->EvaluateAttrString( "Protocol", protocolStr ) ) {
	      throw JobRequest_ex("attribute \"Protocol\" not found or is not a string");
	    }
	    // Check if the version is exactly 1.0.0
	    if ( protocolStr.compare("1.0.0") ) {
	      throw JobRequest_ex("Wrong \"Protocol\" for jobRequest: expected 1.0.0, got " + protocolStr );
	    }
	    
	    classad::ClassAd *argumentsAD = 0; // no need to free this
	    // Parse the "arguments" attribute
	    if ( !classad_safe_ptr->EvaluateAttrClassAd( "arguments", argumentsAD ) ) {
	      throw JobRequest_ex("attribute 'arguments' not found or is not a classad");
	    }
	    
	    classad::ClassAd *adAD = 0; // no need to free this
	    // Look for "JobAd" attribute inside "arguments"
	    if ( !argumentsAD->EvaluateAttrClassAd( "jobad", adAD ) ) {
	      throw JobRequest_ex("Attribute \"JobAd\" not found inside 'arguments', or is not a classad" );
	    }
	    
	    // initializes the m_jdl attribute
	    classad::ClassAdUnParser unparser;
	    unparser.Unparse( jdl, argumentsAD->Lookup( "jobad" ) );
	    
	  } // end classad-mutex protected regions
	  
	  try {
	    theJob.set_jdl( jdl ); // this puts another mutex
	    theJob.set_status( glite::ce::cream_client_api::job_statuses::UNKNOWN );
	  } catch( ClassadSyntax_ex& ex ) {
	    
	    CREAM_SAFE_LOG(
			   api_util::creamApiLogger::instance()->getLogger()->errorStream() 
			   << "util::full_request_unparse() - Cannot instantiate a job from jdl=[" << jdl
			   << "] due to classad excaption: " << ex.what()
			   
			   );
	    throw( ClassadSyntax_ex( ex.what() ) );
	  }
	} // full_request_unparse

	
	//____________________________________________________________________________
	/**
	   Remember to call this method inside a mutex-protected region
	   'cause classad is not thread-safe
	*/
	void creamJdlHelper( const string& oldJdl,
			     string& newjdl ) 
	  throw( ClassadSyntax_ex& )
	{ 
	  const glite::wms::common::configuration::WMConfiguration* WM_conf = iceConfManager::getInstance()->getConfiguration()->wm();
	  
	  classad::ClassAdParser parser;
	  classad::ClassAd *root = parser.ParseClassAd( oldJdl );
	  
	  if ( !root ) {
	    throw ClassadSyntax_ex( boost::str( boost::format( "ClassAd parser returned a NULL pointer parsing request=[%1%]") % oldJdl ) );
	  }
	  
	  boost::scoped_ptr< classad::ClassAd > classad_safe_ptr( root );
	  
	  string ceid;
	  if ( !classad_safe_ptr->EvaluateAttrString( "ce_id", ceid ) ) {
	    throw ClassadSyntax_ex( "ce_id attribute not found" );
	  }
	  boost::trim_if( ceid, boost::is_any_of("\"") );
	  
	  vector<string> ceid_pieces;
	  ceurl_util::parseCEID( ceid, ceid_pieces );
	  string bsname = ceid_pieces[2];
	  string qname = ceid_pieces[3];
	  
	  // Update jdl to insert two new attributes needed by cream:
	  // QueueName and BatchSystem.
	  
	  classad_safe_ptr->InsertAttr( "QueueName", qname );
	  classad_safe_ptr->InsertAttr( "BatchSystem", bsname );
	  
	  if ( 0 == classad_safe_ptr->Lookup( "maxOutputSandboxSize" ) && WM_conf ) {
	    classad_safe_ptr->InsertAttr( "maxOutputSandboxSize", WM_conf->max_output_sandbox_size());
	  }
	  
	  updateIsbList( classad_safe_ptr.get() );
	  updateOsbList( classad_safe_ptr.get() );
	  
#ifdef PIPPO  
	  // Set CERequirements
	  if ( WM_conf ) {
	    std::vector<std::string> reqs_to_forward(WM_conf->ce_forward_parameters());
	    
	    // In order to forward the required attributes, these
	    // have to be *removed* from the CE ad that is used
	    // for flattening.
	    classad::ClassAd* local_ce_ad(new classad::ClassAd(*ce_ad));
	    classad::ClassAd* local_job_ad(new classad::ClassAd(*job_ad));
	    
	    std::vector<std::string>::const_iterator cur_req;
	    std::vector<std::string>::const_iterator req_end = 
	      reqs_to_forward.end();
	    for (cur_req = reqs_to_forward.begin();
		 cur_req != req_end; ++cur_req)
	      {
		local_ce_ad->Remove(*cur_req);
		// Don't care if it doesn't succeed. If the attribute is
		// already missing, so much the better.
	      }
	    
	    // Now, flatten the Requirements expression of the Job Ad
	    // with whatever info is left from the CE ad.
	    // Recipe received from Nick Coleman, ncoleman@cs.wisc.edu
	    // on Tue, 8 Nov 2005
	    classad::MatchClassAd mad;
	    mad.ReplaceLeftAd( local_job_ad );
	    mad.ReplaceRightAd( local_ce_ad );
	    
	    classad::ExprTree *req = mad.GetLeftAd()->Lookup( "Requirements" );
	    classad::ExprTree *flattened_req = 0;
	    classad::Value fval;
	    if( ! ( mad.GetLeftAd()->Flatten( req, fval, flattened_req ) ) ) {
	      // Error while flattening. Result is undefined.
	      return result;
	    }
	    
	    // No remaining requirement. Result is undefined.
	    if (!flattened_req) return result;
	  }
#endif
	  // Produce resulting JDL
	  
	  classad::ClassAdUnParser unparser;
	  unparser.Unparse( newjdl, classad_safe_ptr.get() ); // this is safe: Unparse doesn't deallocate its second argument
	  
	} // end of creamJdlHelper(...) 
	

	//______________________________________________________________________________
	void updateIsbList( classad::ClassAd* jdl )
	{ 
	  // synchronized block because the caller is Classad-mutex synchronized
	  const static char* method_name = "iceCommandSubmit::updateIsbList() - ";
	  string default_isbURI = "gsiftp://";
	  default_isbURI.append( glite::wms::ice::Ice::instance()->getHostName() );
	  default_isbURI.push_back( '/' );
	  string isbPath;
	  if ( jdl->EvaluateAttrString( "InputSandboxPath", isbPath ) ) {
	    default_isbURI.append( isbPath );
	  } else {
	    CREAM_SAFE_LOG( api_util::creamApiLogger::instance()->getLogger()->warnStream() << method_name
			    << "\"InputSandboxPath\" attribute in the JDL. "
			    << "Hope this is correct..."
			    
			    );     
	  }
	  
	  // If the InputSandboxBaseURI attribute is defined, remove it
	  // after saving its value; the resulting jdl will NEVER have
	  // the InputSandboxBaseURI attribute defined.
	  string isbURI;
	  if ( jdl->EvaluateAttrString( "InputSandboxBaseURI", isbURI ) ) {
	    boost::trim_if( isbURI, boost::is_any_of("\"") );
	    boost::trim_right_if( isbURI, boost::is_any_of("/") );
	    // remove the attribute
	    jdl->Delete( "InputSandboxBaseURI" );
	  } else {
	    isbURI = default_isbURI;
	  }
	  
	  pathName isbURIobj( isbURI );
	  
	  // OK, not check each item in the InputSandbox and modify it if
	  // necessary
	  classad::ExprList* isbList;
	  if ( jdl->EvaluateAttrList( "InputSandbox", isbList ) ) {
	    
	    /**
	     * this pointer is used below as argument of ClassAd::Insert. The classad doc
	     * says that that ptr MUST NOT be deallocated by the caller: 
	     * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	     */
	    classad::ExprList* newIsbList = new classad::ExprList();
	    
	    /*CREAM_SAFE_LOG(m_log_dev->debugStream()
	      << "iceCommandSubmit::updateIsbList() - "
	      << "Starting InputSandbox manipulation..."
	      );
	    */
	    string newPath;
	    for ( classad::ExprList::iterator it=isbList->begin(); it != isbList->end(); ++it ) {
	      
	      classad::Value v;
	      string s;
	      if ( (*it)->Evaluate( v ) && v.IsStringValue( s ) ) {
                pathName isbEntryObj( s );
                pathName::pathType_t pType( isbEntryObj.getPathType() );
		
                switch( pType ) {
                case pathName::absolute:
		  newPath = default_isbURI + '/' + isbEntryObj.getFileName();
		  break;
                case pathName::relative:
		  newPath = isbURI + '/' + isbEntryObj.getFileName();
		  break;
                case pathName::invalid: // should abort??
                case pathName::uri:
		  newPath = s;
		  break;
                }                
	      }
	      CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->debugStream() << method_name
			     << s << " became " << newPath
			     );
	      
	      // Builds a new value
	      classad::Value newV;
	      newV.SetStringValue( newPath );
	      // Builds the new string
	      newIsbList->push_back( classad::Literal::MakeLiteral( newV ) );
	    } 
	    /**
	     * The pointer newIsbList pointer is used as argument of ClassAd::Insert. The classad doc
	     * says that that ptr MUST NOT be deallocated by the caller: 
	     * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	     */
	    jdl->Insert( "InputSandbox", newIsbList );
	  }
	}

	//______________________________________________________________________________
	void updateOsbList( classad::ClassAd* jdl )
	{
	  // If no OutputSandbox attribute is defined, then nothing has to be done
	  if ( 0 == jdl->Lookup( "OutputSandbox" ) )
	    return;
	  
	  string default_osbdURI = "gsiftp://";
	  default_osbdURI.append( glite::wms::ice::Ice::instance()->getHostName() );
	  default_osbdURI.push_back( '/' );
	  string osbPath;
	  if ( jdl->EvaluateAttrString( "OutputSandboxPath", osbPath ) ) {
	    default_osbdURI.append( osbPath );
	  } else {
	    CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->warnStream()
			   << "util::updateOsbList() - found no "
			   << "\"OutputSandboxPath\" attribute in the JDL. "
			   << "Hope this is correct..."
			   );        
	  }
	  
	  if ( 0 != jdl->Lookup( "OutputSandboxDestURI" ) ) {
	    
	    // Remove the OutputSandboxBaseDestURI from the classad
	    // OutputSandboxDestURI and OutputSandboxBaseDestURI cannot
	    // be given at the same time.
	    if( 0 != jdl->Lookup( "OutputSandboxBaseDestURI") )
	      jdl->Delete( "OutputSandboxBaseDestURI" );
	    
	    // Check if all the entries in the OutputSandboxDestURI
	    // are absolute URIs
	    
	    classad::ExprList* osbDUList;
	    /**
	     * this pointer is used below as argument of ClassAd::Insert. The classad doc
	     * says that that ptr MUST NOT be deallocated by the caller: 
	     * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	     */
	    classad::ExprList* newOsbDUList = new classad::ExprList();
	    
	    if ( jdl->EvaluateAttrList( "OutputSandboxDestURI", osbDUList ) ) {
	      
	      string newPath;
	      for ( classad::ExprList::iterator it=osbDUList->begin(); 
		    it != osbDUList->end(); ++it ) {
                
                classad::Value v;
                string s;
                if ( (*it)->Evaluate( v ) && v.IsStringValue( s ) ) {
		  pathName osbEntryObj( s );
		  pathName::pathType_t pType( osbEntryObj.getPathType() );
                  
		  switch( pType ) {
		  case pathName::absolute:
		    newPath = default_osbdURI + '/' + osbEntryObj.getFileName();
		    break;
		  case pathName::relative:
		    newPath = default_osbdURI + '/' + osbEntryObj.getFileName();
		    break;
		  case pathName::invalid: // should abort??
		  case pathName::uri:
		    newPath = s;
		    break;
		  }                
                }
		
		CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->debugStream()
			       << "util::updateOsbList() - After input sandbox manipulation, "
			       << s << " became " << newPath
			       );        
		
                // Builds a new value
                classad::Value newV;
                newV.SetStringValue( newPath );
                // Builds the new string
                newOsbDUList->push_back( classad::Literal::MakeLiteral( newV ) );
	      }
	      
	      /**
	       * The pointer newOsbDUList pointer is used as argument of ClassAd::Insert. The classad doc
	       * says that that ptr MUST NOT be deallocated by the caller: 
	       * http://www.cs.wisc.edu/condor/classad/c++tut.html#insert
	       */
	      jdl->Insert( "OutputSandboxDestURI", newOsbDUList );
	    }
	  } else {       
	    if ( 0 == jdl->Lookup( "OutputSandboxBaseDestURI" ) ) {
	      // Put a default OutpuSandboxBaseDestURI attribute
	      jdl->InsertAttr( "OutputSandboxBaseDestURI",  default_osbdURI );
	    }
	  }
	}

	//-----------------------------------------------------------------------------
	// URI utility class
	//-----------------------------------------------------------------------------
	pathName::pathName( const string& p ) :
	  m_log_dev(glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger()),
	  m_fullName( p ),
	  m_pathType( invalid )
	{
	  boost::regex uri_match( "gsiftp://[^/]+(:[0-9]+)?/([^/]+/)*([^/]+)" );
	  boost::regex rel_match( "([^/]+/)*([^/]+)" );
	  boost::regex abs_match( "(file://)?/([^/]+/)*([^/]+)" );
	  boost::smatch what;
	  
	  CREAM_SAFE_LOG(
			 api_util::creamApiLogger::instance()->getLogger()->debugStream()
			 << "util::pathName::CTOR() - Trying to unparse " << p
			 );
	  
	  if ( boost::regex_match( p, what, uri_match ) ) {
	    // is a uri
	    m_pathType = uri;
	    
	    m_fileName = '/';
	    m_fileName.append(what[3].first,what[3].second);
	    if ( what[2].first != p.end() )
	      m_pathName.assign(what[2].first,what[2].second);
	    m_pathName.append( m_fileName );
	  } else if ( boost::regex_match( p, what, rel_match ) ) {
	    // is a relative path
	    m_pathType = relative;
	    
	    m_fileName.assign(what[2].first,what[2].second);
	    if ( what[1].first != p.end() )
	      m_pathName.assign( what[1].first, what[1].second );
	    m_pathName.append( m_fileName );
	  } else if ( boost::regex_match( p, what, abs_match ) ) {
	    // is an absolute path
	    m_pathType = absolute;
	    
	    m_pathName = '/';
	    m_fileName.assign( what[3].first, what[3].second );
	    if ( what[2].first != p.end() ) 
	      m_pathName.append( what[2].first, what[2].second );
	    m_pathName.append( m_fileName );
	  }
	  
	  CREAM_SAFE_LOG(
			 api_util::creamApiLogger::instance()->getLogger()->debugStream()
			 << "util::pathName::CTOR() - "
			 << "Unparsed as follows: filename=[" 
			 << m_fileName << "] pathname=["
			 << m_pathName << "]"
			 );
	  
	}

	//______________________________________________________________________________
	pair<bool, time_t> isgood( const std::string& proxyfile ) throw() {
	  X509* x;
	  try {
	    x = glite::ce::cream_client_api::certUtil::read_BIO(proxyfile);
	  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex) {
	    CREAM_SAFE_LOG(
			   api_util::creamApiLogger::instance()->getLogger()->errorStream()
			   << "util::isgood() - " << ex.what();
			   )
	      return make_pair(false, (time_t)0);
	  }
	  
	  boost::shared_ptr< X509 > tmpIn( x, X509_free );
	  
	  time_t proxyTimeEnd = glite::ce::cream_client_api::certUtil::ASN1_UTCTIME_get( X509_get_notAfter(x) );
	  
	  return make_pair(true, proxyTimeEnd);
	}
	
	//______________________________________________________________________________
	pair<bool, time_t> isvalid( const std::string& proxyfile ) throw() {
	  
	  X509* x;
	  try {
	    x = glite::ce::cream_client_api::certUtil::read_BIO(proxyfile);
	  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex) {
	    CREAM_SAFE_LOG(
			   api_util::creamApiLogger::instance()->getLogger()->errorStream()
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
	//______________________________________________________________________________
	string bintostring( unsigned char* buf, size_t len ) {
	  string result;
	  const char alpha[] = "0123456789abcdef";
	  
	  for ( size_t i=0; i<len; ++i ) {
            result.push_back( alpha[ ( buf[i] >> 4 ) & 0x0f ] );
            result.push_back( alpha[ buf[i] & 0x0f ] );
	  }
	  return result;
	};
	
	//______________________________________________________________________________
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
	
	namespace {
	  
	  class canonizerObject {
	    string target;
	    
	  public:
	    canonizerObject() : target("") {
	    }
	    
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

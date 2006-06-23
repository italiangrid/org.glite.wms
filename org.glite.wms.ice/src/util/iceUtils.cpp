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
 * Author: Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "iceUtils.h"
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <cerrno>
#include <vector>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "boost/thread/recursive_mutex.hpp"

extern int h_errno;
extern int errno;

int setET(char **errtxt, int rc);
//int setETni(char **errtxt, int rc);
int IP2String(unsigned int, int, char *, int );
int resolveHostAddr(const char *InetName, struct sockaddr InetAddr[], int maxipa=1, char** errtxt=0);
int resolveAddrName(const char *, int, char**, char**, char**);
char *resolveHostName(const char *, char**);
char *resolveHostName(struct sockaddr &, char**);
int resolveHostName(struct sockaddr &, char *InetName[], int, char**);    
int resolveHostAddr(const char *InetName, struct sockaddr &InetAddr, char  **errtxt=0);    
    
using namespace std;

namespace glite {
namespace wms {
namespace ice {
namespace util {

//________________________________________________________________________
string getHostName( void ) throw ( runtime_error& )
{
    char name[256];
    
    if ( ::gethostname(name, 256) == -1 ) { // FIXME: is it thread safe ?
        throw runtime_error( string( "Could not resolve local hostname: ") 
                             + string(strerror(errno) ) );
    }
    struct hostent *H=::gethostbyname(name);// FIXME gethostbyname is not thread safe
    if ( !H ) {
        throw runtime_error( string( "Could not resolve local hostname: ") 
                             + string(strerror(errno) ) );
    }
    return string(H->h_name);
}

//________________________________________________________________________
string time_t_to_string( time_t tval ) {
    char buf[26]; // ctime_r wants a buffer of at least 26 bytes
    ctime_r( &tval, buf );
    return string( buf );
}

//________________________________________________________________________
void makePath(const string& filename) throw(exception&)
{
  boost::filesystem::path tmpFile( filename );
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
	//cout << "currentPath=["<<currentPath<<"]"<<endl;
	boost::filesystem::path tmpPath( currentPath );
	if( !boost::filesystem::exists( tmpPath ) )
	  {
	    //cout << "Creating ["<<currentPath << "]"<<endl;
	    boost::filesystem::create_directory( tmpPath );
	  }
      }
    } 
  } catch( std::exception& ex) {
    throw;
  }
}

//________________________________________________________________________
string getNotificationClientDN( const string& DN )
{
  vector<string> pieces;
  boost::split(pieces, DN, boost::is_any_of("CN="));
  if ( pieces.empty() ) return "";
  vector<string>::const_iterator it = pieces.end() - 1;

  return getCompleteHostname( *it );
}

//________________________________________________________________________
string getCompleteHostname( const string& hostname )
{
  //struct sockaddr_in netaddr;
  char *hosterrmsg;// = 0;
  
  //cerr << "Resolving ["<< hostname << "]"<<endl;
  
  char *fullname = resolveHostName( hostname.c_str(), &hosterrmsg);
  
  if( strcmp(fullname, "0.0.0.0") == 0 )
  {
    //cerr << "Error resolving hostname: " << *hosterrmsg << endl;;
    //throw exception(string("Cannot resolve hostname [") + hostname + "]");
    return "";
  }
  
  //cerr << "*** Complete hostname is [" << fullname << "]"<<endl;
  
  //exit(1);
  string FullName = fullname;
  free( fullname );
  return FullName;
}

//________________________________________________________________________
string getIPFromHostname( const string& hostname )
{
  // Will be implemented later; for now it is not urgent
  return "";  
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


//________________________________________________________________________
int IP2String(unsigned int ipaddr, int port, char *buff, int blen)
{
   struct in_addr in;
   int sz;

// Convert the address
//
   in.s_addr = ipaddr;
   if (port <= 0)
      sz = snprintf(buff,blen,"%s",   inet_ntoa((const struct in_addr)in));
      else 
      sz = snprintf(buff,blen,"%s:%d",inet_ntoa((const struct in_addr)in),port);
   return (sz > blen ? blen : sz);
}

//________________________________________________________________________
int resolveHostAddr(const char *InetName,
		    struct sockaddr &InetAddr, 
		    char  **errtxt) 
{
  return resolveHostAddr(InetName, &InetAddr, 1, errtxt);
}

//________________________________________________________________________
int resolveHostAddr(const char *InetName,
                    struct sockaddr  InetAddr[],
                    int maxipa,
                    char **errtxt)
{
#ifdef HAS_NAMEINFO
   struct addrinfo   *rp, *np, *pnp=0;
   struct addrinfo    myhints = {AI_CANONNAME};
#else
   unsigned int addr;
   struct hostent hent, *hp;
   char **p, hbuff[1024];
#endif
   struct sockaddr_in *ip;
   int i, rc;

// Make sure we have something to lookup here
//
    if (!InetName || !InetName[0])
    {
      ip = (struct sockaddr_in *)&InetAddr[0];
      ip->sin_family      = AF_INET;
      ip->sin_port        = 0;
      ip->sin_addr.s_addr = INADDR_ANY;
      memset((void *)ip->sin_zero, 0, sizeof(ip->sin_zero));
      return 1;
    }

// Determine how we will resolve the name
//
   rc = 0;
#ifndef HAS_NAMEINFO
    if (!isdigit((int)*InetName))

       gethostbyname_r(InetName, &hent, hbuff, sizeof(hbuff), &hp, &rc);
       else if ((int)(addr = inet_addr(InetName)) == -1)
               return (errtxt ? setET(errtxt, EINVAL) : 0);
               else gethostbyaddr_r((char *)&addr,sizeof(addr), AF_INET, &hent,
                                    hbuff, sizeof(hbuff), &hp, &rc);
    if (rc) return (errtxt ? setET(errtxt, rc) : 0);

// Check if we resolved the name
//
   for (i = 0, p = hent.h_addr_list; *p != 0 && i < maxipa; p++, i++)
       {ip = (struct sockaddr_in *)&InetAddr[i];
        memcpy((void *)&(ip->sin_addr), (const void *)*p, sizeof(ip->sin_addr));
        ip->sin_port   = 0;
        ip->sin_family = hent.h_addrtype;
        memset((void *)ip->sin_zero, 0, sizeof(ip->sin_zero));
       }

#else

   if (!strncmp(InetName,"localhost",9)) myhints.ai_family = PF_INET;

   // Translate the name to an address list
   if (isdigit((int)*InetName)) myhints.ai_flags |= AI_NUMERICHOST;
   rc = getaddrinfo(InetName,0,(const addrinfo *)&myhints, &rp);
   if (rc || !(np = rp)) return (errtxt ? setETni(errtxt, rc) : 0);


   i = 0;
   do {if (!pnp
       ||  memcmp((const void *)pnp->ai_addr, (const void *)np->ai_addr,
                  sizeof(struct sockaddr)))
           memcpy((void *)&InetAddr[i++], (const void *)np->ai_addr,
                  sizeof(struct sockaddr));
       pnp = np; np = np->ai_next;
      } while(i < maxipa && np);
   freeaddrinfo(rp);

#endif

   return i;
}
   
//________________________________________________________________________
int resolveAddrName(const char *InetName,
                    int maxipa, char **Addr, char **Name,
                    char **errtxt)
{

// Host or address and output arrays must be defined
//
   if (!InetName || !Addr || !Name) return 0;

// Max 10 addresses and names
//
   maxipa = (maxipa > 1 && maxipa < 10) ? maxipa : 1;

// Number of addresses
   struct sockaddr_in ip[10];
   int n = resolveHostAddr(InetName,(struct sockaddr *)ip, maxipa, errtxt);

   // Fill address / name strings, if required
   int i = 0;
   for (; i < n; i++ ) {
 
      // The address
      char buf[255];
      inet_ntop(ip[i].sin_family, &ip[i].sin_addr, buf, sizeof(buf));
      Addr[i] = strdup(buf);

      // The name
      char *names[1] = {0};
      int hn = resolveHostName((struct sockaddr &)ip[i], names, 1, errtxt);
      if (hn)
         Name[i] = strdup(names[0]);
      else
         Name[i] = strdup(Addr[i]);

      // Cleanup
      if (names[0]) free(names[0]);
   }

   // We are done
   return n;
}

//________________________________________________________________________
char *resolveHostName(const char *InetName, char **errtxt)
{
   char myname[256];
   const char *hp;
   struct sockaddr InetAddr;
  
// Identify ourselves if we don't have a passed hostname
//
   if (InetName) hp = InetName;
      else if (gethostname(myname, sizeof(myname))) 
              {if (errtxt) setET(errtxt, errno); return strdup("0.0.0.0");}
              else hp = myname;

// Get the address
//
   if (!resolveHostAddr(hp, InetAddr, errtxt)) return strdup("0.0.0.0");

// Convert it to a fully qualified host name and return it
//
   return resolveHostName(InetAddr, errtxt);
}

//________________________________________________________________________
char *resolveHostName(struct sockaddr &InetAddr, char **errtxt)
{
  char *result;
  if (resolveHostName(InetAddr, &result, 1, errtxt)) return result;

  char dnbuff[64];
  unsigned int ipaddr;
  struct sockaddr_in *ip = (sockaddr_in *)&InetAddr;
  memcpy(&ipaddr, &ip->sin_addr, sizeof(ipaddr));
  IP2String(ipaddr, -1, dnbuff, sizeof(dnbuff));
  return strdup(dnbuff);
}

//________________________________________________________________________
int resolveHostName(struct sockaddr &InetAddr,
                    char *InetName[],
                    int maxipn,
                    char **errtxt)
{
   char mybuff[256];
   int i, rc;
   string tmp;

// Preset errtxt to zero
//
   if (errtxt) *errtxt = 0;

// Some platforms have nameinfo but getnameinfo() is broken. If so, we revert
// to using the gethostbyaddr().
//
#if defined(HAS_NAMEINFO) && !defined(__macos__)
    struct addrinfo   *rp, *np;
    struct addrinfo    myhints = {AI_CANONNAME};
#elif defined(HAS_GETHBYXR)
   struct sockaddr_in *ip = (sockaddr_in *)&InetAddr;
   struct hostent hent, *hp;
   char *hname, hbuff[1024];
#else
   static boost::recursive_mutex mutex;
          //XrdOucMutexHelper getHNhelper;
   struct sockaddr_in *ip = (sockaddr_in *)&InetAddr;
   struct hostent *hp;
   unsigned int ipaddr;
   char *hname;
#endif

// Make sure we can return something
//
   if (maxipn < 1) return (errtxt ? setET(errtxt, EINVAL) : 0);

// Check for unix family which is equl to localhost
//
  if (InetAddr.sa_family == AF_UNIX) 
     {InetName[0] = strdup("localhost"); return 1;}

#if !defined(HAS_NAMEINFO) //|| defined(__macos__)

// Convert it to a host name
//
   rc = 0;
#ifdef HAS_GETHBYXR
   gethostbyaddr_r(&(ip->sin_addr), sizeof(struct in_addr),
                   AF_INET, &hent, hbuff, sizeof(hbuff), &hp, &rc);
#else
   memcpy(&ipaddr, &ip->sin_addr, sizeof(ipaddr));
   boost::recursive_mutex::scoped_lock M( mutex );
   if (!(hp=gethostbyaddr((const char *)&ipaddr, sizeof(InetAddr), AF_INET)))
      rc = (h_errno ? h_errno : EINVAL);
#endif

   if (rc)
      {hname = (char *)inet_ntop(ip->sin_family,
                                 (const void *)(&ip->sin_addr),
                                 mybuff, sizeof(mybuff));
       if (!hname) return (errtxt ? setET(errtxt, errno) : 0);
       InetName[0] = strdup(hname);
       return 1;
      }

// Return first result
//
  {
    tmp = hp->h_name;
    boost::to_lower( tmp );
    InetName[0] = strdup( tmp.c_str() );
  }
// Return additional names
//
   hname = *hp->h_aliases;
   for (i = 1; i < maxipn && hname; i++, hname++) {
       tmp = hname;
       boost::to_lower( tmp );
       InetName[i] = strdup( tmp.c_str() );
   }
#else

// Do lookup of canonical name. We can't use getaddrinfo() for this on all
// platforms because AI_CANONICAL was never well defined in the spec.
//
   if ((rc = getnameinfo(&InetAddr, sizeof(struct sockaddr),
                        mybuff, sizeof(mybuff), 0, 0, 0)))
      return (errtxt ? setETni(errtxt, rc) : 0);

// Return the result if aliases not wanted
//
   if (maxipn < 2)
   {
     tmp = mybuff;
     boost::to_lower( tmp );
     InetName[0] = strdup( tmp.c_str() );
     return 1;
   }

// Disable IPv6 for 'localhost...' (potential confusion in the
// default /etc/hosts on some platforms, e.g. MacOsX)
//
   if (!strncmp(mybuff,"localhost",9)) myhints.ai_family = PF_INET;

// Get the aliases for this name
//
    rc = getaddrinfo(mybuff,0,(const addrinfo *)&myhints, &rp);
    if (rc || !(np = rp)) return (errtxt ? setETni(errtxt, rc) : 0);

// Return all of the names
//
   for (i = 0; i < maxipn && np; i++)
   {
     tmp = np->ai_canonname;
     boost::to_lower( tmp );
     InetName[i] = strdup( tmp.c_str() );
     np = np->ai_next;
   }
   freeaddrinfo(rp);

#endif

// All done
//
   return i;
}

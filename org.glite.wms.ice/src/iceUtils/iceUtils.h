
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
#ifndef GLITE_WMS_ICE_UTIL_ICEUTILS_HH
#define GLITE_WMS_ICE_UTIL_ICEUTILS_HH

#include <string>
#include <map>
#include <list>
#include <set>
#include <stdexcept>
#include <ctime>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>

#include "creamJob.h"
#include "ClassadSyntax_ex.h"
#include "JobRequest_ex.h"

#include <algorithm>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

#include "iceDb/GetJobsToPoll.h" // for definition of type JobToPoll

namespace glite {
namespace wms {
namespace ice {
namespace util {

  std::pair<bool, time_t> isgood( const std::string& proxyfile ) throw();

  std::pair<bool, time_t> isvalid( const std::string& proxyfile ) throw();

  std::string computeSHA1Digest( const std::string& proxyfile ) throw( std::runtime_error& );
  std::string bintostring( unsigned char* buf, size_t len );

  class Request;

  void full_request_unparse(Request* request,
			    CreamJob& theJob,
			    std::string& cmdtype )
    throw( ClassadSyntax_ex&, JobRequest_ex& );
  
  void creamJdlHelper( const std::string& oldJdl,
		       std::string& newjdl ) 
    throw( ClassadSyntax_ex& );

  void updateIsbList( classad::ClassAd* );
  void updateOsbList( classad::ClassAd* jdl );

  // Inner class definition, used to manipulate paths
  class pathName {
  public:
    typedef enum { invalid=-1, absolute, uri, relative } pathType_t;
    
    pathName( const std::string& p );
    virtual ~pathName( ) { }
    // accessors
    pathType_t getPathType( void ) const { return m_pathType; }
    const std::string& getFullName( void ) const { return m_fullName; }
    const std::string& getPathName( void ) const { return m_pathName; }
    const std::string& getFileName( void ) const { return m_fileName; }
    
  protected:
    
    log4cpp::Category* m_log_dev;
    const std::string m_fullName;
    pathType_t m_pathType;
    std::string m_pathName;
    std::string m_fileName;
  };

  //
  // Utility function: Computes a SHA1 hash of the input string. The
  // resulting hash is made of 40 printable characters, each
  // character in the range [0-9A-F].
  //
  // @input name an input string; 
  //
  // @return a string of 40 printable characters, each in the range [0-9A-F]
  //
  std::string compressed_string( const std::string& name );

  struct ltstring {
    bool operator()( const std::pair<std::string, std::string>& s1, 
		     const std::pair<std::string, std::string>& s2) const
    {
      if ( s1.first.compare(s2.first) < 0 ) return true;
      else {
	if(s2.first.compare(s1.first) < 0 ) return false;
	else {
	  if( s1.second.compare(s2.second) < 0 ) return true;
	  else return false;
	}
      }
      
    }
  };
  
  /**
   * Utility function to return the hostname
   *
   * @return the host name
   */
  std::string getHostName( void ) throw ( std::runtime_error& );
  
  std::string getURL( void ) throw ( std::runtime_error& );
  
  /**
   * Converts a time_t value to a string. This function
   * uses the thread-safe function ctime_r.
   *
   * @param tval the time_t value to convert
   * @result the textual representation of timestamp tval
   */ 
  std::string time_t_to_string( time_t tval );
  
  std::string int_to_string( const int val );
  
  /**
   * This function outputs a string containing the CREAM and grid
   * jobid for a given job. This function should be used whenever
   * any information related to a job needs to be logged into the
   * log files.
   *
   * @param job the job whose cream/grid id should be returned
   *
   * @return the string containing cream and grid job id, in
   * human-readable form
   */
  //std::string describe_job( const CreamJob& job );
  
  //  void makePath(const std::string& file) throw(std::exception&); // FIXME: never used?
  
  //std::string getNotificationClientDN( const std::string& );
  
  //std::string getCompleteHostname( const std::string& );
  
  std::string canonizeString( const std::string& ) throw();
  
  /**
   * Ad-hoc implementation of the copy_n algorithm, returning an
   * InputIterator. There actually is a copy_n algorithm defined in
   * GNU C++ implementation of the STL
   * (/usr/include/g++-3/stl_algobase.h), but it says that it is not
   * part of the C++ standard.
   *
   * This function copies at most n elements from the range
   * [InputIterator, InputIterator+n-1] (bounds included) into the
   * range [OutputIterator, OutputIterator+n-1] (bounds included).
   * If the source range is less than n elements wide, only the
   * elements in the range are copied.
   *
   * This function assumes that first and end are iterators to a
   * container of CreamJob objects. dest must be an iterator to a
   * container of string objects. This function copies the CREAM Job
   * ID from object referenced by the first iterator into the second
   * iterator.
   *
   * @param first the iterator of the first element in the source range
   *
   * @param end the iterator of the end of the source range. This
   * iterator is used to check if the input range is less than n
   * elements wide.
   *
   * @param n the maximum number of items to copy
   *
   * @param dest the iterator to the first element in the destination range
   *
   * @return an iterator to the input element PAST the last element copied.
   */
//   template <class InputIterator, class Size, class OutputIterator> 
//     InputIterator copy_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest) {
//     for ( ; n > 0 && first != end; --n ) {
//       //*dest = first->getCreamJobID();
//       *dest = *first;
//       ++first;
//       ++dest;
//     }
//     return first;
//   }
  
//   template <class InputIterator, class Size, class OutputIterator, class UnaryOperation> 
//     InputIterator transform_n_elements( InputIterator first, InputIterator end, Size n, OutputIterator dest, UnaryOperation f) {
//     for ( ; n > 0 && first != end; --n ) {
//       *dest = f( *first );
//       ++first;
//       ++dest;
//     }
//     return first;
//   }
  
  
  
//   class jobMap_appender {
    
//     std::map< std::pair<std::string, std::string>, std::list< CreamJob >, ltstring>& m_jobMap;
    
//     bool (*m_predicate)(const CreamJob& );
    
//   public:
//     jobMap_appender( std::map< std::pair<std::string, std::string>, std::list< CreamJob >, ltstring>& jobMap, bool (*pred)(const CreamJob&)) : m_jobMap( jobMap ), m_predicate( pred ) {
//     }
      
//       void operator()( const CreamJob& j ) {
// 	if( m_predicate( j ) )
// 	  m_jobMap[std::make_pair( j.get_user_dn(), j.get_creamurl())].push_back( j );
//       }
//   };
  
} // namespace util
} // namespace ice
} // namespace wms
} // namespace glite

#endif

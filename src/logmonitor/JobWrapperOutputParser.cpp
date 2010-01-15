// Copyright (c) Members of the EGEE Collaboration. 2009. 
// See http://www.eu-egee.org/partners/ for details on the copyright holders.  

// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//     http://www.apache.org/licenses/LICENSE-2.0 
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

#include <cstring>
#include <cerrno>

#include <fstream>
#include <memory>

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"

#include "glite/jobid/JobId.h"

#include "glite/wms/common/logger/logstream.h"
#include "glite/wms/common/logger/manipulators.h"
#include "jobcontrol_namespace.h"
#include "common/files.h"

#include "JobWrapperOutputParser.h"

using namespace std;
USING_COMMON_NAMESPACE;
RenameLogStreamNS( elog );

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor {

namespace {

typedef  JobWrapperOutputParser   JWOP;

struct JWErrors {
  const char         *jwe_error;
  JWOP::status_type   jwe_status;
};

} // Anonymous namespace

JobWrapperOutputParser::JobWrapperOutputParser( const string &edgid ) : jwop_dagid(), jwop_edgid( edgid ) {}

JobWrapperOutputParser::JobWrapperOutputParser( const string &dagid, const string &edgid ) : jwop_dagid( dagid ), jwop_edgid( edgid ) {}

JobWrapperOutputParser::~JobWrapperOutputParser( void ) {}

bool JobWrapperOutputParser::parseStream( istream &is, string &errors, int &retcode, status_type &stat, string &sc, string &done_reason)
{
  bool                found = false;
  char                buffer[BUFSIZ];
  struct JWErrors     jwErrors[] = { { "Working directory not writable", resubmit },
				     { "GLOBUS_LOCATION undefined",      resubmit },
				     { "/etc/globus-user-env.sh",        resubmit },
				     { "not found or unreadable",        abort },
				     { "Cannot download",                resubmit },
				     { "Cannot upload",                  resubmit },
				     { "Cannot take token!",             resubmit },
				     { "prologue failed with error",     resubmit },    
				     { "epilogue failed with error",     resubmit },    
                                     { NULL, unknown },
  };
  struct JWErrors    *errIt;
  
  sc.assign("NoToken");

  if( is.good() ) {
    do {
      is.getline( buffer, BUFSIZ );

      if( !is.eof() ) {
	if( is.good() ) {
	  for( errIt = jwErrors; errIt->jwe_error != NULL; ++errIt )
	    if( strstr(buffer, errIt->jwe_error) != NULL ) {
	      errors.assign( buffer );
	      stat = errIt->jwe_status;
	      found = true;

	      break;
	    }

          if( strstr(buffer, "job exit status = ") == buffer ) {
            if( sscanf(buffer, "job exit status = %d", &retcode) == 1 ) {
              errors.assign( buffer );
              found = true;
              stat = good;
            }
            else retcode = -1;
          }

          std::string jw_stdout_err(buffer);
          int reason_begin_tag = jw_stdout_err.find("LM_log_done_begin", 0);
          if (reason_begin_tag > 0) {
            int reason_begin_tag_len = std::string("LM_log_done_begin").size();
            done_reason = jw_stdout_err.substr(
              reason_begin_tag + reason_begin_tag_len,
              jw_stdout_err.find("LM_log_done_end", 0) - reason_begin_tag - reason_begin_tag_len
            );

            if (strstr(buffer, "Take token: ") == buffer) {
              char s[256];
              if (sscanf(buffer, "Take token: %255s", s) == 1) {
                s[255] = '\0';
                sc.assign(s);
              } else { // The sequence code is not set... so what can we do?
                sc.assign("");
              }
            }	
	  }
        } else {
	  errors.assign( "IO error while reading file: " );
	  errors.append( strerror(errno) );
	  retcode = -1;
	  stat = resubmit;

	  break;
	}
      }
    } while( !is.eof() );
  }
  else {
    errors.assign( "File not available." );
    retcode = -1;
    stat = resubmit;
  }

  return found;
}

JWOP::status_type JobWrapperOutputParser::parse_file( int &retcode, string &errors, string &sc, string& done_reason)
{
  const configuration::LMConfiguration    *lmconfig = configuration::Configuration::instance()->lm();

  bool                       found = false;
  status_type                stat = good;
  glite::jobid::JobId               id( this->jwop_edgid );
  auto_ptr<jccommon::Files>  files( (this->jwop_dagid.size() != 0) ? new jccommon::Files(this->jwop_dagid, this->jwop_edgid) :
				    new jccommon::Files(this->jwop_edgid) );
  ifstream                   ifs;
  logger::StatePusher        pusher( elog::cedglog, "JobWrapperOutputParser::parse_file(...)" );

  errors.erase(); retcode = -1;

  elog::cedglog << logger::setlevel( logger::high ) << "Going to parse standard output file." << endl;

  ifs.open( files->standard_output().native_file_string().c_str() );
  found = this->parseStream( ifs, errors, retcode, stat, sc, done_reason);
  ifs.close();
  if( !found ) {
    errors.assign( "Standard output does not contain useful data." );
    elog::cedglog << logger::setlevel( logger::null ) << errors << endl;

    if( lmconfig->use_maradona_file() ) {
      elog::cedglog << logger::setlevel( logger::high )
		    << "Standard output was not useful, passing ball to Maradona..." << endl;

      ifs.clear();
      ifs.open( files->maradona_file().native_file_string().c_str() );
      found = this->parseStream( ifs, errors, retcode, stat, sc, done_reason );
      ifs.close();
      if( found )
	elog::cedglog << logger::setlevel( logger::null )
		      << "Got info from Maradona..." << endl;
      else {
	errors.append( "Cannot read JobWrapper output, both from Condor and from Maradona. " );

	elog::cedglog << logger::setlevel( logger::null ) << errors << endl;

	retcode = -1;
	stat = resubmit;
      }
    }
    else {
      elog::cedglog << logger::setlevel( logger::null )
		    << "Maradona disabled, cannot check for alternate output." << endl;

      retcode = -1;
      stat = resubmit;
    }
  }

  return stat;
}

} // Namespace logmonitor

} JOBCONTROL_NAMESPACE_END

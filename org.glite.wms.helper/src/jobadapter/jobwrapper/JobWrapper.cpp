/***************************************************************************
 *  filename  : JobWrapper.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>
#include <string>

#include <classad_distribution.h>

#include "JobWrapper.h"
#include "glite/wms/jdl/JobAdManipulation.h"

using namespace std;
using namespace classad;

namespace url = glite::wms::helper::jobadapter::url;
namespace jdl = glite::wms::jdl;

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

const string JobWrapper::s_brokerinfo_default = ".BrokerInfo";

JobWrapper::JobWrapper(const string& job)
  : m_job(job), m_create_subdir(false), m_nodes(0), m_wmp_support(false)
{  
}

JobWrapper::~JobWrapper(void)
{
}

void
JobWrapper::standard_input(const string& file)
{
  m_standard_input = file;
}

void
JobWrapper::standard_output(const string& file)
{
  m_standard_output = file;
}

void
JobWrapper::standard_error(const string& file)
{
  m_standard_error = file;
}

void
JobWrapper::input_sandbox(const url::URL& base_url,
                          const vector<string>& files)
{
  m_input_base_url = base_url;
  copy(files.begin(), files.end(), back_inserter(m_input_files));
}

void
JobWrapper::output_sandbox(const url::URL& base_url,
                           const vector<string>& files)
{
  m_output_base_url = base_url;
  copy(files.begin(), files.end(), back_inserter(m_output_files));
}
 
void
JobWrapper::create_subdir(void)
{
  m_create_subdir = true;
}

void
JobWrapper::brokerinfo(const string& file)
{
  assert(file.find_first_of("/") == string::npos);
  assert(file != "");

  m_brokerinfo = file;
}

void
JobWrapper::job_Id(const string& jobid)
{
  m_jobid = jobid;
}

void
JobWrapper::job_id_to_filename(const string& jobid_to_filename)
{
  m_jobid_to_filename = jobid_to_filename;
}

void
JobWrapper::arguments(const string& args)
{
  m_arguments = args;
}

void
JobWrapper::maradonaprotocol(const string& protocol, const string& filename)
{
  m_maradonaprotocol.assign(protocol);
  m_maradonaprotocol.append(filename); 
}

void
JobWrapper::gatekeeper_hostname(const string& gatekeeper)
{
  m_gatekeeper_hostname = gatekeeper;
}

void
JobWrapper::globus_resource_contact_string(
                const string& globus_resource_contact_string)
{
  m_globus_resource_contact_string = globus_resource_contact_string;
}

void 
JobWrapper::environment(const vector<string>& env)
{
  m_environment = env;
}

void
JobWrapper::nodes(int node)
{
  m_nodes = node;
}

void
JobWrapper::outputdata(const ExprList* output_data)
{
  m_outputdata.reset(static_cast<ExprList*>(output_data->Copy()));
}

void
JobWrapper::vo(const string& vo)
{
  m_vo = vo;
}

void
JobWrapper::dsupload(const url::URL& id)
{
  m_dsupload.assign("DSUpload_");
  m_dsupload.append(id.path().substr(1));
  m_dsupload.append(".out");
}

void
JobWrapper::wmp_support(void)
{
  m_wmp_support = true;
}

void 
JobWrapper::wmp_input_sandbox_support(const std::vector<std::string>& input_base_files)
{
  copy(input_base_files.begin(), input_base_files.end(), back_inserter(m_wmp_input_base_files));

  for (std::vector<std::string>::const_iterator it = input_base_files.begin();
      it != input_base_files.end(); it++) {

    string::size_type pos = it->find_last_of("/");
    if (pos == string::npos) {
      // throw ExInvalidURL("there is not ://");
      std::cerr << "ERROR" ;
    }
    string filename = it->substr(pos);
    m_wmp_input_files.push_back(filename); 
  }

}

void 
JobWrapper::wmp_output_sandbox_support(const std::vector<std::string>& output_files,
                                  const std::vector<std::string>& output_dest_files)
{
  copy(output_files.begin(), output_files.end(), back_inserter(m_wmp_output_files));
  copy(output_dest_files.begin(), output_dest_files.end(), back_inserter(m_wmp_output_dest_files));
}

ostream&
JobWrapper::set_shell(ostream& os) const
{
  return os << "#!/bin/sh" << endl 
	    << endl;
}

ostream&
JobWrapper::set_subdir(ostream& os, 
		          const string& dir) const
{
  return os << "newdir=" << dir << endl
            << "mkdir ${newdir}" << endl
            << "cd ${newdir}" << endl 
            << endl;	 
}

ostream&
JobWrapper::clean_subdir(ostream& os) const
{
  return os << "cd .." << endl
            << "rm -rf ${newdir}" << endl 
	    << endl;
}

ostream&
JobWrapper::check_workdir(ostream& os) const
{
  os << "if [ ! -w . ]; then" << endl
     << "  echo \"Working directory not writable\"" << endl << endl;
  
  os << " "; this->set_lb_sequence_code(os, 
		  "Done", 
		  "Working directory not writable!",
		  "FAILED", 
		  "0");
  
  return os << "  exit 1" << endl
	    << "fi" << endl 
	    << endl;
}

ostream&
JobWrapper::set_workdir(ostream& os) const
{
  return os << "workdir=\"`pwd`\"" << endl
	    << endl;
}

ostream&
JobWrapper::set_environment(ostream& os, 
		            const vector<string>& env) const
{
  for (vector<string>::const_iterator it = m_environment.begin();
      it != m_environment.end(); it++) {
    os << "export " << *it << endl;
  }
  os << endl;
  return os;        	
}

ostream&
JobWrapper::set_environment(ostream& os,
		            const string& variable,
			    const string& value) const
{
  return os << "export " << variable << "=\"" << value << "\"" << endl;
}

ostream&
JobWrapper::check_set_environment(ostream& os,
		                  const string& variable,
		                  const string& value) const
{
  os << "if [ -z \"${" << variable << "}\" ]; then" << endl;
  os << "  "; this->set_environment(os, variable, value); 
  return os << "fi" << endl << endl;
}

ostream&
JobWrapper::set_umask(ostream& os) const
{
  return os << "umask 022" << endl
            << endl;
}

ostream&
JobWrapper::check_file_mod(ostream& os,
		           const string& file) const
{
  os << "if [ -e \"" << file << "\" ]; then" << endl
     << "  chmod +x \"" << m_job << "\" 2> /dev/null" << endl
     << "else" << endl
     << "  echo \"" << file << " not found or unreadable\"" << endl
     << "  echo \"" << file << " not found or unreadable\" >> \"${maradona}\"" << endl << endl;

  string message(file);
  message.append(" not found or unreadable!");
  
  os << "  "; this->set_lb_sequence_code(os, 
		  "Done",
		  message,
                  "FAILED",
                  "0");
  	    
  return os << "  doExit 1" << endl
	    << "fi" << endl 
	    << endl;
}

ostream&
JobWrapper::set_lb_sequence_code(ostream& os,
                                 const string& event,
				 const string& reason,
				 const string& scode,
				 const string& ecode) const
{
  os << "GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent" << " \\" << endl
     << " --jobid=" << "\"" << "$GLITE_WMS_JOBID" << "\"" << " \\" << endl
     << " --source=LRMS" << " \\" << endl
     << " --sequence=" << "\"" << "$GLITE_WMS_SEQUENCE_CODE" << "\"" << "\\" << endl
     << " --event=" << "\"" << event << "\"" << "\\" << endl;

     if (!event.compare("Running")) {
       os << " --node=$host" << "\\" << endl;
     } else if (!scode.compare("FAILED")) {
       os << " --reason=" << "\"" << reason << "\"" << "\\" << endl
          << " --status_code=" << scode << "\\" << endl
          << " --exit_code=" << ecode << "\\" << endl;
     } else {
       os << " --status_code=" << scode << "\\" << endl
	  << " --exit_code=" << ecode << "\\" << endl;
     }

  return os << " || echo $GLITE_WMS_SEQUENCE_CODE`" << endl
	    << "export GLITE_WMS_SEQUENCE_CODE" << endl
	    << endl;
}

ostream& 
JobWrapper::check_globus(std::ostream& os) const
{
  os << "if [ -z \"${GLOBUS_LOCATION}\" ]; then" << endl
     << "  echo \"GLOBUS_LOCATION undefined\"" << endl
     << "  echo \"GLOBUS_LOCATION undefined\" >> \"${maradona}\"" << endl << endl;

  os << "  "; this->set_lb_sequence_code(os,
		  "Done",
		  "GLOBUS_LOCATION undefined",
                  "FAILED",
                  "0"); 

  os << "  doExit 1" << endl
     << "elif [ -r \"${GLOBUS_LOCATION}/etc/globus-user-env.sh\" ]; then" << endl
     << "  . ${GLOBUS_LOCATION}/etc/globus-user-env.sh" << endl
     << "else" << endl
     << "  echo \"${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable\"" << endl
     << "  echo \"${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable\" >> \"${maradona}\"" << endl << endl;
  
  os << "  "; this->set_lb_sequence_code(os,
		  "Done",
		  "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable",
		  "FAILED",
		  "0");
  
  return os << "  doExit 1" << endl
	    << "fi" << endl 
	    << endl;
}

ostream&
JobWrapper::prepare_transfer(ostream& os,
		             const vector<string>& file) const
{
  os << "for f in";
  for (vector<string>::const_iterator it = file.begin();
       it != file.end(); ++it) {
    os << " \"" << *it << "\"";
  }
  return os << "; do" << endl;
}

ostream&
JobWrapper::make_transfer(ostream& os,
		          const url::URL& prefix,
			  const bool& input) const
{
  // source and destination must be '/' terminated
  string value = prefix.as_string();
  if (value[value.size() - 1] != '/') {
    value.append( 1, '/' );
  }

  if (input) {
    os << "  globus-url-copy " << value << "${f}" << " file://${workdir}/${f}" <<
       endl
       << "  if [ $? != 0 ]; then" << endl
       << "    echo \"Cannot download ${f} from " << value << "\"" << endl
       << "    echo \"Cannot download ${f} from " << value << "\" >> \"${maradona}\"" << endl << endl;

    string imessage("Cannot download ${f} from ");
    imessage.append(value);
    
    os << "    "; this->set_lb_sequence_code(os,
		    "Done",
		    imessage,
		    "FAILED",
		    "0");

    os << "    doExit 1" << endl;
  } else {
    os << "  if [ -r \"${f}\" ]; then" << endl;
        
    os << "    output=`dirname $f`" << endl
       << "    if [ \"x${output}\" = \"x.\" ]; then" << endl
       << "      ff=$f" << endl
       << "    else" << endl
       << "      ff=${f##*/}" << endl
       << "    fi" << endl;
	    
    os << "    globus-url-copy file://${workdir}/${f} " << value << "${ff}" << endl
       << "    if [ $? != 0 ]; then" << endl
       << "      echo \"Cannot upload ${f} into " << value << "\"" << endl
       << "      echo \"Cannot upload ${f} into " << value << "\" >> \"${maradona}\"" << endl << endl;

    string omessage("Cannot upload ${f} into ");
    omessage.append(value);
    
    os << "      "; this->set_lb_sequence_code(os,
		       "Done",
		       omessage,
		       "FAILED",
		       "0");

    os << "      doExit 1" << endl
       << "    fi" << endl;
  }
  return os << "  fi" << endl
	    << "done" << endl 
	    << endl;
}

ostream&
JobWrapper::make_transfer_wmp_support(ostream& os,
                          const bool& input) const
{
  // source and destination must be '/' terminated
//  string value = prefix.as_string();
//  if (value[value.size() - 1] != '/') {
//    value.append( 1, '/' );
//  }

  os << "  file=`basename $f`" << endl;

  if (input) {
    os << "  globus-url-copy ${f} file://${workdir}/${file}" <<
       endl
       << "  if [ $? != 0 ]; then" << endl
       << "    echo \"Cannot download ${file} from ${f}\"" << endl
       << "    echo \"Cannot download ${file} from ${f}\" >> \"${maradona}\"" << endl << endl;

    string imessage("Cannot download ${file} from ${f}");
//    imessage.append(value);

    os << "    "; this->set_lb_sequence_code(os,
                    "Done",
                    imessage,
                    "FAILED",
                    "0");

    os << "    doExit 1" << endl;
  } else {
    os << "  if [ -r \"${file}\" ]; then" << endl;

//    os << "    output=`dirname $f`" << endl
//       << "    if [ \"x${output}\" = \"x.\" ]; then" << endl
//       << "      ff=$f" << endl
//       << "    else" << endl
//       << "      ff=${f##*/}" << endl
//       << "    fi" << endl;

    os << "    globus-url-copy file://${workdir}/${file} ${f}" << endl
       << "    if [ $? != 0 ]; then" << endl
       << "      echo \"Cannot upload ${file} into ${f}\"" << endl
       << "      echo \"Cannot upload ${file} into ${f}\" >> \"${maradona}\"" << endl << endl;

    string omessage("Cannot upload ${file} into ${f}");
//    omessage.append(value);

    os << "      "; this->set_lb_sequence_code(os,
                       "Done",
                       omessage,
                       "FAILED",
                       "0");

    os << "      doExit 1" << endl
       << "    fi" << endl;
  }
  return os << "  fi" << endl
            << "done" << endl
            << endl;
}

ostream& 
JobWrapper::make_bypass_transfer(ostream& os,
				 const string& prefix) const
{
  return os;
}

ostream&
JobWrapper::execute_job(ostream& os,
		        const string& arguments,
			const string& job,
			const string& stdi,
			const string& stdo,
			const string& stde,
			int   node) const
{
  os << "(" << endl
     << "  perl -e '" << endl
     << "    unless (defined($ENV{\"EDG_WL_NOSETPGRP\"})) {" << endl
     << "      $SIG{\"TTIN\"} = \"IGNORE\";" << endl
     << "      $SIG{\"TTOU\"} = \"IGNORE\";" << endl
     << "      setpgrp(0, 0);" << endl
     << "    }" << endl
     << "    exec(@ARGV);" << endl
     << "    warn \"could not exec $ARGV[0]: $!\\n\";" << endl
     << "    exit(127);" << endl
     << "  ' ";

  if (arguments != "") {
    os << "\"" << job << "\"" << " " << arguments << " $*";
  }
  else {
    os << "\"" << job << "\" $*";
  }

  if (stdi != "") {
    os << " < \"" << stdi << "\"";
  }

  if (stdo != "") {
    os << " > \"" << stdo << "\"";
  } else {
    os << " > /dev/null ";
  }

  if (stde != "") {
    if (stdo != "") {
      if (stdo == stde) {
        os << " 2>&1";
      } else {
        os << " 2> \"" << stde << "\"";
      }
    }
  } else {
    os << " 2> /dev/null";
  }

  return os << " &" << endl
            << endl
            << "  user_job=$!" << endl
            << endl
            << "  exec 2> /dev/null" << endl
            << endl
            << "  perl -e '" << endl
            << "    while (1) {" << endl
            << "      $time_left = `grid-proxy-info -timeleft 2> /dev/null` || 0;" << endl
            << "      last if ($time_left <= 0);" << endl
            << "      sleep($time_left);" << endl
            << "    }" << endl
            << "    kill(defined($ENV{\"EDG_WL_NOSETPGRP\"}) ? 9 : -9, '\"$user_job\"');" << endl
            << "    exit(1);" << endl
            << "  ' &" << endl
            << endl
            << "  watchdog=$!" << endl
            << endl
            << "  wait $user_job" << endl
            << "  status=$?" << endl
            << endl
            << "  kill -9 $watchdog $user_job -$user_job" << endl
            << endl
            << "  exit $status" << endl
            << ")" << endl 
	    << endl;
}

ostream&
JobWrapper::create_maradona_file(ostream& os, 
		                 const string& jobid_to_filename) const
{
  // create the maradona file
  return os << "maradona=\".maradona." << jobid_to_filename 
	    << ".output\"" << endl
            << "touch \"${maradona}\"" << endl 
            << endl;	
}

ostream&
JobWrapper::send_dgas_gianduia_lsf_files(ostream& os,
                const string& globus_resource_contact_string,
                const string& gatekeeper) const
{
  os << "if [ -n \"${LSB_JOBID}\" ]; then" << endl;
  os << "  cat \"${X509_USER_PROXY}\" ";
  os << "| ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient ";
  os << "-s " << gatekeeper << ":56569: ";
  os << "-L lsf_${LSB_JOBID} ";
  os << "-G ${GLITE_WMS_JOBID} ";
  os << "-C " << globus_resource_contact_string << " ";
  os << "-H \"$HLR_LOCATION\"" << endl;
  os << "  if [ $? != 0 ]; then" << endl;
  os << "  echo \"Error transferring gianduia with command: ";
  os << "cat ${X509_USER_PROXY} ";
  os << "| ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient ";
  os << "-s " << gatekeeper << ":56569: ";
  os << "-L lsf_${LSB_JOBID} ";
  os << "-G ${GLITE_WMS_JOBID} ";
  os << "-C " << globus_resource_contact_string << " ";
  os << "-H $HLR_LOCATION\"" << endl;
  os << "  fi" << endl;
  os << "fi" << endl << endl;
  return os;
}

ostream&
JobWrapper::send_dgas_gianduia_pbs_files(ostream& os,
                const string& globus_resource_contact_string,
                const string& gatekeeper) const
{
  os << "if [ -n \"${PBS_JOBID}\" ]; then" << endl;
  os << "  cat ${X509_USER_PROXY} ";
  os << "| ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient ";
  os << "-s " << gatekeeper << ":56569: ";
  os << "-L pbs_${PBS_JOBID} ";
  os << "-G ${GLITE_WMS_JOBID} ";
  os << "-C " << globus_resource_contact_string << " ";
  os << "-H \"$HLR_LOCATION\"" << endl;
  os << "  if [ $? != 0 ]; then" << endl;
  os << "  echo \"Error transferring gianduia with command: ";
  os << "cat ${X509_USER_PROXY} ";
  os << "| ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient ";
  os << "-s " << gatekeeper << ":56569: ";
  os << "-L pbs_${PBS_JOBID} ";
  os << "-G ${GLITE_WMS_JOBID} ";
  os << "-C " << globus_resource_contact_string << " ";
  os << "-H $HLR_LOCATION\"" << endl;
  os << "  fi" << endl;
  os << "fi" << endl << endl;
  return os;
}

ostream&
JobWrapper::doExit(ostream& os, 
		   const string& maradonaprotocol, 
		   bool create_subdir) const
{
  // The doExit function used to copy the maradona file, too...
  os << "doExit()" << endl << "{" << endl
     << "  stat=$1" << endl << endl
     << "  echo \"jw exit status = ${stat}\"\n"
     << "  echo \"jw exit status = ${stat}\" >> \"${maradona}\"\n\n"
     << "  globus-url-copy \"file://${workdir}/${maradona}\" "
     << "\"" << maradonaprotocol << "\"" << endl << endl;

  // create subdirectory and cd into it if requested
  if (create_subdir) {
    clean_subdir(os);
  }

  os << "  exit $stat" << endl
     << "}" << endl 
     << endl;

  return os;  
}

ostream&
JobWrapper::handle_output_data(ostream& os) const
{
  ExprList::const_iterator it;
  bool check;
  
  os << "return_value=0" << endl;
  
  os << "if [ $status -eq 0 ]; then" << endl 
     << "  local=`doDSUploadTmp`" << endl
     << "  status=$?" << endl
     << "  return_value=$status" << endl
     << endl;
  
  for (it = m_outputdata->begin(); it != m_outputdata->end(); ++it) {
    ClassAd* ad = dynamic_cast<ClassAd*>(*it);
    if (ad != 0) {
      string outputfile = jdl::get_output_file(*ad);
      string lfn        = jdl::get_logical_file_name(*ad, check);
      string se         = jdl::get_storage_element(*ad, check);
      
      os << "  local=`doCheckReplicaFile " << outputfile << "`" << endl 
         << "  status=$?" << endl
	 << "  if [ $status -ne 0 ]; then" << endl
	 << "    return_value=1" << endl
	 << "  else" << endl;

      if (lfn.empty() && se.empty()) {
	os << "    local=`doReplicaFile " << outputfile << "`" << endl;
      }
      else if ((!lfn.empty()) && se.empty()) {
	os << "    local=`doReplicaFilewithLFN " << outputfile 
	   << " " << lfn << "`" << endl;
      } 
      else if (lfn.empty() && (!se.empty())) {
        os << "    local=`doReplicaFilewithSE " << outputfile
           << " " << se << "`" << endl;
      }
      else {
	os << "    local=`doReplicaFilewithLFNAndSE " << outputfile
           << " " << lfn << " " << se << "`" << endl;
      }
    
      os << "    status=$?" << endl
         << "  fi" << endl
         << endl;
    }
  }	

  os << "  local=`doDSUpload`" << endl
     << "  status=$?" << endl
     << "fi" << endl 
     << endl;

  return os;
}

ostream&
JobWrapper::doDSUploadTmp(ostream& os) const
{
  os << "doDSUploadTmp()" << endl
     << "{" << endl
     << "  filename=" << m_dsupload << endl
     << "  echo \"#\" >> $filename.tmp" << endl
     << "  echo \"# Autogenerated by JobWrapper!\" >> $filename.tmp" << endl
     << "  echo \"#\" >> $filename.tmp" << endl
     << "  echo \"# The file contains the results of the upload and registration\" >> $filename.tmp" << endl
     << "   echo \"# process in the following format:\" >> $filename.tmp" << endl
     << "   echo \"# <outputfile> <lfn|guid|Error>\" >> $filename.tmp" << endl
     << "   echo \"\" >> $filename.tmp" << endl
     << "}" << endl
     << endl;

  return os;
}

ostream&
JobWrapper::doDSUpload(ostream& os) const
{
  os << "doDSUpload()" << endl
     << "{" << endl
     << "  filename=" << m_dsupload << endl
     << "  mv -fv $filename.tmp $filename" << endl
     << "}" << endl
     << endl;

  return os;
}

ostream&
JobWrapper::doCheckReplicaFile(ostream& os) const
{
  os << "doCheckReplicaFile()" << endl
     << "{" << endl
     << "  sourcefile=$1" << endl
     << "  filename=" << m_dsupload << endl
     << "  exit_status=0" << endl << endl
     << "  if [ ! -f \"${workdir}/$sourcefile\" ]; then" << endl
     << "    echo \"$sourcefile    Error: File $sourcefile has not been found on the WN $host\" >> $filename.tmp" << endl
     << "    exit_status=1" << endl
     << "  fi" << endl << endl
     << "  echo \"\" >> $filename.tmp" << endl
     << "  return $exit_status" << endl
     << "}" << endl
     << endl;

  return os;
}

ostream&
JobWrapper::doReplicaFile(ostream& os) const
{
  os << "doReplicaFile()" << endl
     << "{" << endl
     << "  sourcefile=$1" << endl
     << "  filename=" << m_dsupload << endl
     << "  exit_status=0" << endl 
     << endl;

  os << "  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo 
     << " copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`" << endl
     << "  result=$?" << endl
     << "  if [ $result -eq 0 ]; then" << endl
     << "    echo \"$sourcefile    $local\" >> $filename.tmp" << endl
     << "  else" << endl
     << "    echo \"$sourcefile    Error: $local\" >> $filename.tmp" << endl
     << "    exit_status=1" << endl
     << "  fi" << endl 
     << endl;
  
  os << "  echo \"\" >> $filename.tmp" << endl
     << "  return $exit_status" << endl
     << "}" << endl
     << endl;

  return os;  
}

ostream&
JobWrapper::doReplicaFilewithLFN(ostream& os) const
{
  os << "doReplicaFilewithLFN()" << endl
     << "{" << endl
     << "  sourcefile=$1" << endl
     << "  lfn=$2" << endl
     << "  filename=" << m_dsupload << endl
     << "  exit_status=0" << endl 
     << endl;
  
  os << "  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`" << endl
     << "  result=$?" << endl
     << "  if [ $result -eq 0 ]; then" << endl
     << "    echo \"$sourcefile    $lfn\" >> $filename.tmp" << endl
     << "  else" << endl
     << "    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`" << endl
     << "    result=$?" << endl
     << "    if [ $result -eq 0 ]; then" << endl
     << "      echo \"$sourcefile    $localnew\" >> $filename.tmp" << endl
     << "    else" << endl
     << "      echo \"$sourcefile    Error: $local; $localnew\" >> $filename.tmp" << endl
     << "      exit_status=1" << endl
     << "    fi" << endl
     << "  fi" << endl 
     << endl;
  
  os << "  echo \"\" >> $filename.tmp" << endl
     << "  return $exit_status" << endl
     << "}" << endl
     << endl;

  return os;
}

ostream&
JobWrapper::doReplicaFilewithSE(ostream& os) const
{
  os << "doReplicaFilewithSE()" << endl
     << "{" << endl
     << "  sourcefile=$1" << endl
     << "  se=$2" << endl
     << "  filename=" << m_dsupload << endl
     << "  exit_status=0" << endl
     << endl;

  os << "  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`" << endl
     << "  result=$?" << endl
     << "  if [ $result -eq 0 ]; then" << endl
     << "    echo \"$sourcefile    $local\" >> $filename.tmp" << endl
     << "  else" << endl
     << "    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`" << endl
     << "    result=$?" << endl
     << "    if [ $result -eq 0 ]; then" << endl
     << "      echo \"$sourcefile    $localnew\" >> $filename.tmp" << endl
     << "    else" << endl
     << "      echo \"$sourcefile    Error: $local; $localnew\" >> $filename.tmp" << endl
     << "      exit_status=1" << endl
     << "    fi" << endl
     << "  fi" << endl
     << endl;

  os << "  echo \"\" >> $filename.tmp" << endl
     << "  return $exit_status" << endl
     << "}" << endl
     << endl;

  return os;
}

ostream&
JobWrapper::doReplicaFilewithLFNAndSE(ostream& os) const
{
  os << "doReplicaFilewithLFNAndSE()" << endl
     << "{" << endl
     << "  sourcefile=$1" << endl
     << "  lfn=$2" << endl
     << "  se=$3" << endl
     << "  filename=" << m_dsupload << endl
     << "  exit_status=0" << endl
     << endl;

  os << "  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn -d $se 2>&1`" << endl
     << "  result=$?" << endl
     << "  if [ $result -eq 0 ]; then" << endl
     << "    echo \"$sourcefile    $lfn\" >> $filename.tmp" << endl
     << "  else" << endl
     << "    localse=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`" << endl
     << "    result=$?" << endl
     << "    if [ $result -eq 0 ]; then" << endl
     << "      echo \"$sourcefile    $localse\" >> $filename.tmp" << endl
     << "    else" << endl
     << "      locallfn=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`" << endl
     << "      result=$?" << endl
     << "      if [ $result -eq 0 ]; then" << endl
     << "        echo \"$sourcefile    $locallfn\" >> $filename.tmp" << endl
     << "      else" << endl
     << "        localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=" << m_vo
     << " copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`" << endl
     << "        result=$?" << endl
     << "        if [ $result -eq 0 ]; then" << endl
     << "          echo \"$sourcefile    $localnew\" >> $filename.tmp" << endl
     << "        else" << endl
     << "          echo \"$sourcefile    Error: $local; $localse; $locallfn; $localnew\" >> $filename.tmp" << endl
     << "          exit_status=1" << endl
     << "        fi" << endl   
     << "      fi" << endl
     << "    fi" << endl
     << "  fi" << endl
     << endl;
	  
  os << "  echo \"\" >> $filename.tmp" << endl
     << "  return $exit_status" << endl
     << "}" << endl
     << endl;

  return os;  
}
		    

ostream&
JobWrapper::print(ostream& os) const
{
  // set shell	
  set_shell(os);	

  // write the doExit function
  doExit(os, m_maradonaprotocol, m_create_subdir);

  // write the doReplicaFile function
  if (m_outputdata != 0) {
    doDSUploadTmp(os);
    doDSUpload(os);
    doCheckReplicaFile(os);
    doReplicaFile(os);
    doReplicaFilewithLFN(os);
    doReplicaFilewithSE(os);
    doReplicaFilewithLFNAndSE(os);
  }
	
  // check and set the environment variable GLITE_WMS_LOG_DESTINATION with the 
  // name of the gatekeeper node
  if (m_gatekeeper_hostname != "") {
    check_set_environment(os, "GLITE_WMS_LOG_DESTINATION", m_gatekeeper_hostname);
  }
  // set the environment variable GLTIE_WMS_JOBID
  if (m_jobid != "") {
    set_environment(os, "GLITE_WMS_JOBID", m_jobid);
    os << endl;
  }
  // set the environment variable GLITE_WMS_SEQUENCE_CODE
  set_environment(os, "GLITE_WMS_SEQUENCE_CODE", "$1");
  os << "shift" << endl << endl;
  // check and set the environment variable GLITE_WMS_LOCATION
  string edg_location_value("${GLITE_LOCATION:-/opt/glite}");
  check_set_environment(os, "GLITE_WMS_LOCATION", edg_location_value);  

  // create subdirectory and cd into it if requested
  if (m_create_subdir) {
    set_subdir(os, m_jobid_to_filename);
  }
  // check if the working directory is writable and set it
  check_workdir(os);
  set_workdir(os);

  // set the environment variable GLITE_WMS_RB_BROKERINFO
  if (m_brokerinfo != "") {
    string broker_info_value("`pwd`");
    broker_info_value.append("/");
    broker_info_value.append(m_brokerinfo);
    set_environment(os, "GLITE_WMS_RB_BROKERINFO", broker_info_value);
    os << endl;
  }
  
  // Handling Maradona
  create_maradona_file(os, m_jobid_to_filename);

  // check globus
  check_globus(os);
  
  // set the jdl environment variables
  set_environment(os, m_environment);

  send_dgas_gianduia_lsf_files(os, m_globus_resource_contact_string,
                               m_gatekeeper_hostname);
  send_dgas_gianduia_pbs_files(os, m_globus_resource_contact_string,
                               m_gatekeeper_hostname);

  // set the umask
  set_umask(os);
 
  // transfer input sandbox
  
//  prepare_transfer(os, m_input_files);
  if (m_wmp_support) {
    prepare_transfer(os, m_wmp_input_base_files);
    make_transfer_wmp_support(os, true);
  } else { 
    prepare_transfer(os, m_input_files); 
    make_transfer(os, m_input_base_url, true);
  }

  // check the file mod
  check_file_mod(os, m_job);
  
  // It is used for the interactive job
  // Extract host
  string host(m_input_base_url.protocol());
  host.append("://");
  host.append(m_input_base_url.host());
  host.append("/");
  make_bypass_transfer(os, host);
  
  os << "host=`hostname -f`" << endl;
  // update the environment variable GLITE_WMS_SEQUENCE_CODE
  set_lb_sequence_code(os, "Running", "The job starts running!", "", "");

  // execute job
  execute_job(os, m_arguments, m_job, 
	      m_standard_input, m_standard_output, m_standard_error,
	      m_nodes);

  // log job return code to JSS
  os << "status=$?" << endl;

  // replica the outputdata
  if (m_outputdata != 0) {	  
    handle_output_data(os);
  }
	
  os << "echo \"job exit status = ${status}\"" << endl; 
  os << "echo \"job exit status = ${status}\" >> \"${maradona}\"" << endl
     << endl; 
  
  // transfer output sandbox
  os << "error=0" << endl;
  if (m_wmp_support) {
    prepare_transfer(os, m_wmp_output_dest_files);
    make_transfer_wmp_support(os, false);
  } else {
    prepare_transfer(os, m_output_files);
    make_transfer(os, m_output_base_url, false);
  }

  // update the environment variable GLITE_WMS_SEQUENCE_CODE
  set_lb_sequence_code(os, 
		  "Done", 
		  "The job finished correctly!", 
		  "OK", 
		  "$status");
  
  // Copy back the Maradona file
  os << "doExit 0" << endl;

  return os;
}

std::ostream&
operator<<(std::ostream& os, const JobWrapper& jw)
{
  return jw.print(os);
}

} // namespace jobwrapper
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

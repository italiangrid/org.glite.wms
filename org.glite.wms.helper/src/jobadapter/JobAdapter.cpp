/***************************************************************************
 *  filename  : JobAdapter.cpp
 *  authors   : Elisabetta Ronchieri <elisbetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <fstream>
#include <vector>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#include <openssl/md5.h>

#include <memory>

#include <classad_distribution.h>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include "jobadapter/url/URL.h"
#include "JobAdapter.h"
#include "jobadapter/jobwrapper/JobWrapper.h"

#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/boost_fs_add.h"
#include "glite/wms/common/utilities/edgstrstream.h"

#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"

#include "glite/wms/helper/exceptions.h"
#include "exceptions.h"

namespace fs = boost::filesystem;
namespace config = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;
namespace utilities = glite::wms::common::utilities;
namespace jobwrapper = glite::wms::helper::jobadapter::jobwrapper;
namespace url = glite::wms::helper::jobadapter::url;
namespace jdl = glite::wms::jdl;

namespace {
std::string const helper_id("JobAdapterHelper");
}

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

namespace {
class Beginning {
public:
  Beginning(const std::string& str) 
    : m_what(str)
    {
    }

  bool operator()(const std::string& str) const
  {
    return str.find(m_what) != std::string::npos;
  }
private:
  std::string m_what;
};
}

/******************************************************************
 *  method :   replace
 *  field: string class
 *  Replace "what" value with "with" value
 *******************************************************************/
void
replace(std::string& where, const std::string& what, const std::string& with)
{
  size_t fpos = where.find(what);

  while (fpos  != std::string::npos) {
    where.replace(fpos, what.length(), with);
    fpos = where.find(what, fpos + what.length() + 1);
  }
}

JobAdapter::JobAdapter(classad::ClassAd const* ad)
 : m_ad(ad)
{
}

JobAdapter::~JobAdapter(void)
{
}

classad::ClassAd*
JobAdapter::resolve(void)  
try {
  std::auto_ptr<classad::ClassAd> result(new classad::ClassAd);
  
  /* Mandatory */
  std::string executable(jdl::get_executable(*m_ad));
  if (executable.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::EXECUTABLE,
                                        executable,
                                        "not empty",
                                        helper_id);
  }
  
  /* Mandatory */
  /* It is renamed as usersubjectname and reinserted with */
  std::string certificatesubject(jdl::get_certificate_subject(*m_ad));
  if (certificatesubject.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::CERT_SUBJ,
                                        certificatesubject,
                                        "not empty",
                                        helper_id);
  }  

  jdl::set_user_subject_name(*result, certificatesubject);
  
  /* Not Mandatory */
  vector<std::string>  outputsandbox;
  utilities::EvaluateAttrListOrSingle(*m_ad, "outputsandbox", outputsandbox); 
 
  bool b_osb_dest_uri = false;
  vector<std::string>  outputsandboxdesturi;
  if (!outputsandbox.empty()) {
    utilities::EvaluateAttrListOrSingle(*m_ad, "outputsandboxdesturi", outputsandboxdesturi);
    if (!outputsandboxdesturi.empty()) {
      b_osb_dest_uri = true;
    }
  }
  
  /* Not Mandatory */
  vector<std::string>  inputsandbox;
  utilities::EvaluateAttrListOrSingle(*m_ad, "inputsandbox", inputsandbox);

  /* Not Mandatory */
  bool b_wmpisb_base_uri = false;
  std::string wmpisb_base_uri(jdl::get_wmpinput_sandbox_base_uri(*m_ad, b_wmpisb_base_uri));
  if (!wmpisb_base_uri.empty()) {
    b_wmpisb_base_uri = true;
  }

  /* Mandatory */
  // InputSandboxPath is always included
  std::string inputsandboxpath;
  inputsandboxpath.append(jdl::get_input_sandbox_path(*m_ad));
  if (inputsandboxpath.empty()) { 
    throw helper::InvalidAttributeValue(jdl::JDLPrivate::INPUT_SANDBOX_PATH,
                                        inputsandboxpath,
                                        "not empty",
                                        helper_id);
  }
	  
  std::string outputsandboxpath;
  if (!b_osb_dest_uri && !b_wmpisb_base_uri) {
    /* Mandatory */
    outputsandboxpath.append(jdl::get_output_sandbox_path(*m_ad));
    if (outputsandboxpath.empty()) {
      throw helper::InvalidAttributeValue(jdl::JDLPrivate::OUTPUT_SANDBOX_PATH,
                                          outputsandboxpath,
                                          "not empty",
                                          helper_id);
    }
  }

  /* Not Mandatory */
  bool   b_arg;
  std::string arguments(jdl::get_arguments(*m_ad, b_arg));
 
  /* Not Mandatory */
  vector<std::string> env;
  utilities::EvaluateAttrListOrSingle(*m_ad, jdl::JDL::ENVIRONMENT, env);

  /* Mandatory */
  /* It is renamed in globusscheduler. (Below) */
  std::string globusresourcecontactstring(jdl::get_globus_resource_contact_string(*m_ad));
  if (globusresourcecontactstring.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::GLOBUSRESOURCE,
                                        globusresourcecontactstring,
                                        "not empty",
                                        helper_id);
  }

  // Get the gatekeeper hostname
  std::string::size_type pos = globusresourcecontactstring.find(":");
  if (pos == 0 || pos == std::string::npos) {
    throw helper::InvalidAttributeValue(jdl::JDL::GLOBUSRESOURCE,
                                        globusresourcecontactstring,
                                        "contains a hostname before a :",
                                        helper_id);
  }
  std::string gatekeeper_hostname(globusresourcecontactstring.substr(0, pos));
 
  /* Mandatory */
  /* x509 user proxy is mandatory for the condor submit file. */
  std::string userproxy(jdl::get_x509_user_proxy(*m_ad));
  if (userproxy.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDLPrivate::USERPROXY,
                                        userproxy,
                                        "not empty",
                                        helper_id);
  }
 
  /* Mandatory */
  /* queuname is mandatory to build the globusrsl string. */
  std::string queuename(jdl::get_queue_name(*m_ad));
  if (queuename.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::QUEUENAME,
                                        queuename,
                                        "not empty",
                                        helper_id);
  }

  /* Mandatory */
  /* lrms type is mandatory for the mpich job */
  /* and forwarded to Condor-C in any case.   */
  std::string lrmstype(jdl::get_lrms_type(*m_ad));

  // Figure out the local host name (this should really come from
  // some common entity).
  char   hostname[1024];
  std::string local_host_name;
  
  if (gethostname(hostname, sizeof(hostname)) >= 0) {
    hostent resolved_host;
    hostent *resolver_result=NULL;
    int resolver_work_buffer_size = 2048;
    char* resolver_work_buffer = new char[resolver_work_buffer_size];
    int resolve_errno = ERANGE;
    while ((gethostbyname_r(hostname, &resolved_host,
                            resolver_work_buffer,
                            resolver_work_buffer_size,
                            &resolver_result,
                            &resolve_errno) ) == ERANGE ) {
      delete[] resolver_work_buffer;
      resolver_work_buffer_size *= 2;
      resolver_work_buffer = new char[resolver_work_buffer_size];
    }
    if (resolver_result != NULL) {
      local_host_name.assign(resolved_host.h_name);
    }
    delete[] resolver_work_buffer;
  }

  /* FIXME: Test whether this is a 'blahpd' or 'condor' resource so that  */
  /* FIXME: the submit file can be adjusted accordingly.      */
  /* FIXME: Eventually, CondorG should be able to handle this */

  static boost::regex expression_blahce(":[0-9]+/blah-");
  boost::smatch       result_blahce;
  bool is_blahp_resource = false;
  bool is_condor_resource = false;

  if (boost::regex_search(globusresourcecontactstring,
			  result_blahce,
                          expression_blahce)) {
    is_blahp_resource = true;
  } else {
    static boost::regex expression_condorce(":[0-9]+/condor-");
    boost::smatch       result_condorce;

    if (boost::regex_search(globusresourcecontactstring,
			    result_condorce,
                            expression_condorce)) {
      is_condor_resource = true;
    }
  }

  std::string condor_submit_environment;

  /* Mandatory */
  jdl::set_universe(*result, "grid"); 

  config::JCConfiguration const* jcconfig = config::Configuration::instance()->jc();

  if (!is_blahp_resource && !is_condor_resource) {
    jdl::set_grid_type(*result, "globus");
    jdl::set_copy_to_spool(*result, false);
    jdl::set_transfer_executable(*result, true);
  } else {
    jdl::set_grid_type(*result, "condor");
    if (is_condor_resource || lrmstype == "condor") {
      // condor resource, as per Nate Mueller's May 4, 2005 recipe
      jdl::set_remote_job_universe(*result, 9);
      jdl::set_remote_job_grid_type(*result, "condor");
      jdl::set_remote_requirements(*result, "true");
      jdl::set_remote_remote_schedd(*result, gatekeeper_hostname);
      jdl::set_remote_remote_pool(*result, queuename);
      jdl::set_remote_globus_resource(*result, "BOGUS");
      jdl::set_remote_remote_job_universe(*result, 5);
      // FIXME: these should obviously not be hardwired.
      jdl::set_remote_remote_requirements(*result, "((Arch == \"INTEL\") && (Opsys == \"LINUX\")) && HasFileTransfer");
      jdl::set_remote_remote_file_system_domain(*result, gatekeeper_hostname);
      jdl::set_remote_remote_uid_system_domain(*result, gatekeeper_hostname);
      jdl::set_remote_remote_should_transfer_files(*result, "YES");
      jdl::set_remote_remote_when_to_transfer_output(*result, "ON_EXIT");
    } else {
      // blah resource
      jdl::set_remote_job_universe(*result, 9);
      jdl::set_remote_sub_universe(*result, "blah");
      jdl::set_remote_job_grid_type(*result, "blah");
      jdl::set_remote_remote_grid_type(*result, lrmstype);
      jdl::set_remote_remote_queue(*result, queuename);
      // We have to make sure the *jobwrapper* gets transferred to 
      // the worker nodes.
      jdl::set_remote_remote_stagecmd(*result, true);
    }

    unsigned char md5_cert_hash[MD5_DIGEST_LENGTH];
    utilities::oedgstrstream md5_hex_hash;

    // Without the local host name, the CondorC/BLAH submission
    // will most likely not work. We bail out in this case:
    if (local_host_name.empty()) {
      throw helper::InvalidAttributeValue("local hostname",
                                          local_host_name,
                                          "not empty",
                                          helper_id);
    }

    // Virtual Organization is used to compose remote Condor daemons
    // unique name, but we don't really care if it's empty
    std::string vo(jdl::get_virtual_organisation(*m_ad));

    std::string hostandcertificatesubject(local_host_name);
    hostandcertificatesubject.append("/");
    hostandcertificatesubject.append(certificatesubject);
    MD5((unsigned char *)hostandcertificatesubject.c_str(),
        hostandcertificatesubject.length(),
        md5_cert_hash);
    md5_hex_hash.setf(std::ios_base::hex, std::ios_base::basefield);
    md5_hex_hash.width(2);
    md5_hex_hash.fill('0');

    for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
      md5_hex_hash.width(2);
      md5_hex_hash << (unsigned int)md5_cert_hash[i];
    }

    std::string hashedcertificatesubject(md5_hex_hash.str());

    jdl::set_daemon_unique_name(*result, hashedcertificatesubject);

    std::string remote_schedd(hashedcertificatesubject);
    remote_schedd.append("@");
    remote_schedd.append(gatekeeper_hostname);
    jdl::set_remote_schedd(*result, "$$(Name)");

    std::string schedd_requirements("Name==\"");
    schedd_requirements.append(remote_schedd);
    schedd_requirements.append("\"");
    jdl::set_condor_requirements(*result, schedd_requirements);

    jdl::set_globus_resource(*result, "$$(Name)");

    jdl::set_copy_to_spool(*result, false);
    jdl::set_should_transfer_files(*result, "YES");
    jdl::set_when_to_transfer_output(*result, "ON_EXIT");
    jdl::set_transfer_input_files(*result, userproxy);

    // Compute proxy file basename: remove trailing whitespace and slashes.
    std::string userproxy_basename(userproxy);
    char c = userproxy_basename[userproxy_basename.length()-1];
    while (c == '/' || c == ' ' || c == '\n' || c == '\r' || c == '\t') {
      userproxy_basename.erase(userproxy_basename.length()-1);
      c = userproxy_basename[userproxy_basename.length()-1];
    }

    // Then remove leading dirname.
    userproxy_basename.assign(userproxy_basename.substr(userproxy_basename.rfind('/')+1));

    if (! is_condor_resource) {
      condor_submit_environment.assign("X509_USER_PROXY=");
      condor_submit_environment.append(userproxy_basename);
    }

    // Handle remotely-managed environment additions.
    // (GRAM cannot do this).
    if (!condor_submit_environment.empty()) condor_submit_environment.append(";");

    condor_submit_environment.append("$$(GLITE_ENV:UNDEFINED)");

    jdl::set_remote_env(*result,condor_submit_environment);

    // Cause the job to pop out of the Condor queue if no successful
    // match with 'B' daemons could occur in 15 (default) minutes.
    utilities::oedgstrstream periodic_hold_expression;
    periodic_hold_expression << "Matched =!= TRUE && CurrentTime > QDate + ";
    periodic_hold_expression << jcconfig->maximum_time_allowed_for_condor_match();
    jdl::set_periodic_hold(*result,periodic_hold_expression.str());

    // Build the jobmanager-fork contact string.
    std::string::size_type pos = globusresourcecontactstring.find("/");
    if (pos == 0 || pos == std::string::npos) {
      throw helper::InvalidAttributeValue(jdl::JDL::GLOBUSRESOURCE,
                                          globusresourcecontactstring,
                                          "contains a slash",
                                          helper_id);
    }
    std::string gatekeeper_fork(globusresourcecontactstring.substr(0, pos));
    gatekeeper_fork.append("/jobmanager-fork");

    jdl::set_site_name(*result,gatekeeper_hostname);
    jdl::set_site_gatekeeper(*result,gatekeeper_fork);
  } /* End of blah || condor case */

  jdl::set_globus_scheduler(*result, globusresourcecontactstring);
  jdl::set_x509_user_proxy(*result, userproxy);

  /* Mandatory */
  jdl::set_notification(*result, "never");

  /* Not Mandatory */
  bool b_std;
  std::string stdinput(jdl::get_std_input(*m_ad, b_std));
  std::string stderror(jdl::get_std_error(*m_ad, b_std));
  std::string stdoutput(jdl::get_std_output(*m_ad, b_std));

  /* TEMP patch: forward ce_id */
  bool   b_ce_id;
  std::string ce_id(jdl::get_ce_id(*m_ad, b_ce_id));
  jdl::set_ce_id(*result, ce_id, b_ce_id);

  /* Mandatory */
  /* job id is mandatory to build the globusrsl string and to create the */
  /* job wrapper                                                         */
  std::string job_id(jdl::get_edg_jobid(*m_ad));
  if (job_id.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::JOBID,
                                        job_id,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_edg_jobid(*result, job_id);

  /* keep the dag id if present */
  bool b_present = false;
  std::string dag_id(jdl::get_edg_dagid(*m_ad, b_present));
  if (b_present && !dag_id.empty()) {
    jdl::set_edg_dagid(*result, dag_id);
  }  

  /* Mandatory */
  std::string requirements("EDG_WL_JDL_REQ");
  requirements.append(" '");
  std::string reqvalue(jdl::unparse_requirements(*m_ad));
  if (reqvalue.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::REQUIREMENTS,
                                        reqvalue,
                                        "not empty",
                                        helper_id);
  }
  replace(reqvalue, "\"", "\\\"");
  
  requirements.append(reqvalue);
  requirements.append("'");
  
  /* Create globusrsl value */
  std::string globusrsl("(queue=");
  globusrsl.append(queuename);
  globusrsl.append(")(jobtype=single)");

  //globusrsl.append("(environment=(");
  //globusrsl.append(requirements);
  //globusrsl.append("))");

  /* Not Mandatory */
  bool b_hlr;
  std::string hlrlocation("EDG_WL_HLR_LOCATION");
  hlrlocation.append(" '");
  std::string hlrvalue(jdl::get_hlrlocation(*m_ad, b_hlr));
  if (!hlrvalue.empty()) {
    if (is_blahp_resource || is_condor_resource) {
      condor_submit_environment.append(";EDG_WL_HLR_LOCATION=");
      condor_submit_environment.append(hlrvalue);
      jdl::set_remote_env(*result,condor_submit_environment);
    }
    replace(hlrvalue, "\"", "\\\"");
    hlrlocation.append(hlrvalue);
    hlrlocation.append("'");
    globusrsl.append("(environment=(");
    globusrsl.append(hlrlocation);
    globusrsl.append(") (");
    std::string jid("EDG_WL_JOBID");
    jid.append(" '");
    std::string jidvalue(job_id);
    replace(jidvalue, "\"", "\\\"");
    jid.append(jidvalue);
    jid.append("'");
    globusrsl.append(jid);
    globusrsl.append("))");
  }
  
  /* Mandatory */
  std::string type(jdl::get_type(*m_ad));
  if (type.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::TYPE,
                                        type,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_type(*result, type);
  
  /* Mandatory */
  /* job type is mandatory to build the correct job wrapper file */
  std::string jobtype(jdl::get_job_type(*m_ad));
  if (jobtype.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::JOBTYPE,
                                        jobtype,
                                        "not empty",
                                        helper_id);
  }
  
  /* start preparing JobWrapper file */
  std::auto_ptr<jobwrapper::JobWrapper> jw;
   
  /* convert the jobid into filename */
  std::string jobid_to_file(jobid::to_filename(job_id));
  /* concert the dagid into filename */
  std::string dagid_to_file;
  if (!dag_id.empty()) {
    dagid_to_file.append(jobid::to_filename(dag_id));
  }

  /* check if there is '/' in the executable */
  if (executable[0] != '/') {
    executable.insert(0, "./", 2);
  }    
	
  /* lowercase all jobtype characters */
  std::string ljobtype(jobtype);
  transform(ljobtype.begin(), ljobtype.end(), ljobtype.begin(), ::tolower); 
  
  if (ljobtype == "mpich") {
    /* lowercase all lrmstype characters */
    std::string llrmstype(lrmstype);
    std::transform(llrmstype.begin(), llrmstype.end(), llrmstype.begin(), ::tolower);    
    if (llrmstype != "lsf" && llrmstype != "pbs") {
      throw helper::InvalidAttributeValue(jdl::JDL::LRMS_TYPE,
                                          lrmstype,
                                          "lsf or pbs",
                                          helper_id);
    }
    
    /* Mandatory */
    /* node number is mandatory for the mpich job */
    int    nodenumber = jdl::get_node_number(*m_ad);

    if (is_blahp_resource || is_condor_resource) {
      jdl::set_remote_remote_nodenumber(*result, nodenumber);
    }

    std::string nn(boost::lexical_cast<std::string>(nodenumber));
    
    globusrsl.append("(count=");
    globusrsl.append(nn);
    globusrsl.append(")(hostCount=");
    globusrsl.append(nn);
    globusrsl.append(")");
   
    std::string exec;
    std::string::size_type pos = executable.find("./");
    if (pos == std::string::npos) {
      exec.append(executable);
    } else {
      exec.append(executable.substr(pos+2));
    }
 
    if (llrmstype == "lsf") {
      jw.reset(new jobwrapper::JobWrapper(exec));
      jw->set_job_type(jobwrapper::MPI_LSF);
    }
    else if ((llrmstype == "pbs") || (llrmstype == "torque")) {
      jw.reset(new jobwrapper::JobWrapper(exec));
      jw->set_job_type(jobwrapper::MPI_PBS);
    } else {
      // not possible;
    }
    jw->nodes(nodenumber);

  } else if (ljobtype == "interactive") {
    if (find_if(env.begin(), env.end(), 
	Beginning("BYPASS_SHADOW_PORT=")) == env.end())
    {
      std::ostringstream env_str;
      std::copy(env.begin(), env.end(),
                std::ostream_iterator<std::string>(env_str, ":"));
      throw helper::InvalidAttributeValue(jdl::JDL::ENVIRONMENT,
                                          env_str.str(),
                                          "contains BYPASS_SHADOW_PORT",
                                          helper_id);
    }
    if (find_if(env.begin(), env.end(),
	    Beginning(std::string("BYPASS_SHADOW_HOST="))) == env.end()
    )
    {
      std::ostringstream env_str;
      std::copy(env.begin(), env.end(),
                std::ostream_iterator<std::string>(env_str, ":"));
      throw helper::InvalidAttributeValue(jdl::JDL::ENVIRONMENT,
                                          env_str.str(),
                                          "contains BYPASS_SHADOW_HOST",
                                          helper_id);
    }

    env.push_back("LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH");

    std::string::size_type pos = executable.find("./");
    if (pos == std::string::npos) {
      jw.reset(new jobwrapper::JobWrapper(executable));
      jw->set_job_type(jobwrapper::INTERACTIVE);
    } else {
      jw.reset(new jobwrapper::JobWrapper(executable.substr(pos+2)));
      jw->set_job_type(jobwrapper::INTERACTIVE);
    }
  } else {
    jw.reset(new jobwrapper::JobWrapper(executable));
      jw->set_job_type(jobwrapper::NORMAL);
  }
 
  // PerusalFileEnable is not mandatory
  bool b_perusal;
  bool perusal = jdl::get_perusal_file_enable(*m_ad, b_perusal);
  if (b_perusal & perusal) {
    // PerusalTimeInterval is mandatory
    int perusaltimeinterval = jdl::get_perusal_time_interval(*m_ad);

    // PerusalFilesDestURI is mandatory
    std::string perusalfilesdesturi = jdl::get_perusal_files_dest_uri(*m_ad);
    if (perusalfilesdesturi.empty()) {
      throw helper::InvalidAttributeValue(jdl::JDL::PU_FILES_DEST_URI,
                                          perusalfilesdesturi,
                                          "not empty",
                                          helper_id);
    }

    // PerusalListFileURI is mandatory
    std::string perusallistfileuri = jdl::get_perusal_list_file_uri(*m_ad);
    if (perusallistfileuri.empty()) {
      throw helper::InvalidAttributeValue(jdl::JDLPrivate::PU_LIST_FILE_URI,
                                          perusallistfileuri,
                                          "not empty",
                                          helper_id);
    }
    jw->perusal_support();
    jw->perusal_timeinterval(perusaltimeinterval);
    jw->perusal_filesdesturi(perusalfilesdesturi);
    jw->perusal_listfileuri(perusallistfileuri);
  }
 
 
  // OutputData is not mandatory
  bool b_od;
  classad::ExprTree* odtree = jdl::get_output_data(*m_ad, b_od);
  std::string otptdata("Warning");
  if (b_od) {
    classad::ExprList* odtree_list = dynamic_cast<classad::ExprList*>(odtree);

    if (odtree_list) {
      jw->outputdata(odtree_list);
    } else {
      classad::ClassAd *odtree_classad = dynamic_cast<classad::ClassAd*>(odtree);
      if (odtree_classad) {
        std::auto_ptr<classad::ExprList> list(new classad::ExprList);
        list->push_back(odtree->Copy());
        jw->outputdata(list.get());
      } else {
        throw helper::InvalidAttributeValue(jdl::JDL::OUTPUTDATA,
			                    otptdata,
					    "either list of classad or a classad",
					    helper_id);
      }
    }
      
    /* Virtual Organization is mandatory if OutputData is in the JDL */
    std::string vo(jdl::get_virtual_organisation(*m_ad));
    if (vo.empty()) {
      throw helper::InvalidAttributeValue(jdl::JDL::VIRTUAL_ORGANISATION,
                                          vo,
                                          "not empty",
                                          helper_id);
    } else {
       jw->vo(vo);	    
    }
    try {
      jw->dsupload(url::URL(job_id));
    } catch (url::ExInvalidURL& ex) {
      throw CannotCreateJobWrapper(ex.parameter());
    }
  }
	
  jdl::set_globus_rsl(*result, globusrsl);
  
  /* Mandatory for shallow resubmission */
  config::WMConfiguration const* wmconfig = config::Configuration::instance()->wm();
  std::string token_file(wmconfig->token_file());

  jw->standard_input(stdinput);
  jw->standard_output(stdoutput);
  jw->standard_error(stderror);
  jw->brokerinfo();
  jw->create_subdir();
  jw->arguments(arguments);
  jw->job_Id(job_id);
  jw->job_id_to_filename(jobid_to_file);
  jw->environment(env);
  jw->gatekeeper_hostname(globusresourcecontactstring.substr(0, pos));
  jw->globus_resource_contact_string(globusresourcecontactstring);
 
  //check if there is the protocol in the inputsandbox path.
  //if no the protocol gsiftp:// is added to the inputsandboxpath.
  try { 
    if (inputsandboxpath.find("://") == std::string::npos) {
      std::string new_inputsandboxpath("gsiftp://");
      new_inputsandboxpath.append(local_host_name);
      new_inputsandboxpath.append(inputsandboxpath);
      url::URL url_(new_inputsandboxpath);
      if (!b_wmpisb_base_uri) {
        jw->input_sandbox(url_, inputsandbox);
      } else {
        jw->wmp_input_sandbox_support(url_, inputsandbox);
      }
      // set token for the shallow resubmission
      std::string::size_type const pos = new_inputsandboxpath.rfind("/input");
      std::string const token_path(new_inputsandboxpath, 0, pos);
      jw->token(token_path + '/' + token_file);
    } else {
      url::URL url_(inputsandboxpath);
      if (!b_wmpisb_base_uri) {
        jw->input_sandbox(url_, inputsandbox);
      } else {
        jw->wmp_input_sandbox_support(url_, inputsandbox);
      }
      // set token for the shallow resubmission
      std::string::size_type const pos = inputsandboxpath.rfind("/input");
      std::string const token_path(inputsandboxpath, 0, pos);
      jw->token(token_path + '/' + token_file);
    }
  } catch (url::ExInvalidURL& ex) {
    throw CannotCreateJobWrapper(ex.parameter());
  }

  if (!b_wmpisb_base_uri) {
    //check if there is the protocol in the outputsandbox path. 
    //if no the protocol gsiftp:// is added to the outputsandboxpath.
    try {
      if (outputsandboxpath.find("://") == std::string::npos) {
        std::string new_outputsandboxpath("gsiftp://");     
        new_outputsandboxpath.append(local_host_name);
        new_outputsandboxpath.append(outputsandboxpath);
        jw->output_sandbox(url::URL(new_outputsandboxpath), outputsandbox);
      } else {  
        jw->output_sandbox(url::URL(outputsandboxpath), outputsandbox);
      }
    } catch (url::ExInvalidURL& ex) {
      throw CannotCreateJobWrapper(ex.parameter());
    }
  } else if (b_osb_dest_uri) {
    jw->wmp_output_sandbox_support(outputsandbox, outputsandboxdesturi);
  }

  //check if we need to support new Input/Output Sandboxes in WMProxy
  if (b_osb_dest_uri || b_wmpisb_base_uri) {
    jw->wmp_support();
  }
  // end preparation JobWrapper file

  config::LMConfiguration const* logconfig = config::Configuration::instance()->lm();
  
  if (!b_wmpisb_base_uri) {
    // Mandatory
    // Maradone file path
    std::string maradonapr(logconfig->maradona_transport_protocol());

    config::NSConfiguration const* nsconfig = config::Configuration::instance()->ns();
  
    fs::path maradona_path(
      fs::normalize_path(nsconfig->sandbox_staging_path()),
      fs::native);

    maradona_path /= jobid::get_reduced_part(job_id);
    maradona_path /= jobid_to_file;
    maradona_path /= "Maradona.output";
    jw->maradonaprotocol(maradonapr, maradona_path.native_file_string());
  } else {
    jw->maradonaprotocol(wmpisb_base_uri, "/Maradona.output");
  }

  // read the submit file path
  std::string jw_name("JobWrapper.");
  jw_name += jobid_to_file + ".sh";
  fs::path jw_path(
    fs::normalize_path(jcconfig->submit_file_dir()),
    fs::native);
  if (!dag_id.empty()) {
    jw_path /= jobid::get_reduced_part(dag_id);
    std::string jw_dagid_name("dag.");
    jw_dagid_name.append(dagid_to_file);
    jw_path /= jw_dagid_name;
  } else {  
    jw_path /= jobid::get_reduced_part(job_id);
  }
  jw_path /= jw_name;

  try { 
    fs::create_parents(jw_path.branch_path());
  } catch(fs::filesystem_error &err) {
    throw CannotCreateJobWrapper(jw_path.native_file_string());
  }

  // write the JobWrapper in the submit file path
  std::ofstream ofJW(jw_path.native_file_string().c_str());
  if (!ofJW) {
    throw CannotCreateJobWrapper(jw_path.native_file_string());
  }	  
  ofJW << *jw;

  // The jdl::get_output_data function returns a _copy_ of the object.
  delete odtree;
  
  // Mandatory
  // the new executable value is the jobwrapper file path
  jdl::set_executable(*result, jw_path.native_file_string());

  // Prepare the output file path
  
  std::string output_file_path(jcconfig->output_file_dir());
  output_file_path.append("/");
  if (!dag_id.empty()) {
    output_file_path.append(jobid::get_reduced_part(dag_id));
    output_file_path.append("/");
    output_file_path.append(dagid_to_file);
  } else {
    output_file_path.append(jobid::get_reduced_part(job_id));
  }  
  output_file_path.append("/");
  output_file_path.append(jobid_to_file);
  output_file_path.append("/");

  if (is_blahp_resource || is_condor_resource) {
    // Condor-C will not move Standard Output and Error to the absolute
    // path specified in the submit file, but to the Initial Dir.
    // This defaults to /tmp, so StandardOutput/Error files from different
    // jobs may overwrite each other as well.
    // So, set a different InitialDir per job.
    jdl::set_initial_dir(*result, output_file_path);
  }

  // New parameter Mandatory
  // Output file path is mandatory
  std::string output(output_file_path);
  output.append("StandardOutput");
  jdl::set_output(*result, output);

  // New parameter Mandatory
  // Error file path is mandatory
  std::string error(output_file_path);
  error.append("StandardError");
  jdl::set_error_(*result, error);

  /* Mandatory */
  jdl::set_stream_output(*result, false);
  jdl::set_stream_error(*result, false);

  // Mandatory
  // Condor Log file path
  std::string condor_file_path(logconfig->condor_log_dir());
  std::string log(condor_file_path);
  log.append("/");
  log.append("CondorG.log");
  jdl::set_log(*result, log);
  
  return result.release();

} catch (jdl::CannotGetAttribute const& e) {
  throw helper::CannotGetAttribute(e, helper_id);
} catch (jdl::CannotSetAttribute const& e) {
  throw helper::CannotSetAttribute(e, helper_id);
} 

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite


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

#include <memory>

#include <classad_distribution.h>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include "jobadapter/url/URL.h"
#include "JobAdapter.h"
#include "jobadapter/jobwrapper/JobWrapper.h"
#include "jobadapter/jobwrapper/MpiLsfJobWrapper.h"
#include "jobadapter/jobwrapper/MpiPbsJobWrapper.h"
#include "jobadapter/jobwrapper/InteractiveJobWrapper.h"

#include "glite/wms/jobid/JobId.h"
#include "glite/wms/jobid/manipulation.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/utils.h"
#include "glite/wms/common/utilities/boost_fs_add.h"

#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"

#include "glite/wms/helper/exceptions.h"
#include "exceptions.h"

using namespace std;
using namespace classad;

namespace config = glite::wms::common::configuration;
namespace jobid = glite::wms::jobid;
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
  Beginning(const string& str) 
    : m_what(str)
    {
    }

  bool operator()(const string& str) const
  {
    return str.find(m_what) != string::npos;
  }
private:
  string m_what;
};
}

JobAdapter::JobAdapter(ClassAd const* ad)
 : m_ad(ad)
{
}

JobAdapter::~JobAdapter(void)
{
}

ClassAd*
JobAdapter::resolve(void)  
try {
  auto_ptr<ClassAd> result(new ClassAd);
  
  /* Mandatory */
  string executable(jdl::get_executable(*m_ad));
  if (executable.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::EXECUTABLE,
                                        executable,
                                        "not empty",
                                        helper_id);
  }
  
  /* Mandatory */
  /* It is renamed as usersubjectname and reinserted with */
  string certificatesubject(jdl::get_certificate_subject(*m_ad));
  if (certificatesubject.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::CERT_SUBJ,
                                        certificatesubject,
                                        "not empty",
                                        helper_id);
  }  

  jdl::set_user_subject_name(*result, certificatesubject);
  
  /* Not Mandatory */
  vector<string>  outputsandbox;
  utilities::EvaluateAttrListOrSingle(*m_ad, "outputsandbox", outputsandbox); 
  
  /* Not Mandatory */
  vector<string>  inputsandbox;
  utilities::EvaluateAttrListOrSingle(*m_ad, "inputsandbox", inputsandbox);

  /* Mandatory */
  string inputsandboxpath(jdl::get_input_sandbox_path(*m_ad));
  if (inputsandboxpath.empty()) { 
    throw helper::InvalidAttributeValue(jdl::JDLPrivate::INPUT_SANDBOX_PATH,
                                        inputsandboxpath,
                                        "not empty",
                                        helper_id);
  }
	  
  /* Mandatory */
  string outputsandboxpath(jdl::get_output_sandbox_path(*m_ad));
  if (outputsandboxpath.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDLPrivate::OUTPUT_SANDBOX_PATH,
                                        outputsandboxpath,
                                        "not empty",
                                        helper_id);
  }
  
  /* Not Mandatory */
  bool   b_arg;
  string arguments(jdl::get_arguments(*m_ad, b_arg));
 
  /* Not Mandatory */
  vector<string> env;
  utilities::EvaluateAttrListOrSingle(*m_ad, jdl::JDL::ENVIRONMENT, env);

  /* Mandatory */
  jdl::set_copy_to_spool(*result, false);
  jdl::set_transfer_executable(*result, true);
  jdl::set_universe(*result, "Globus");
 
  /* Mandatory */
  jdl::set_notification(*result, "never");
  
  /* Not Mandatory */
  bool b_std;
  string stdinput(jdl::get_std_input(*m_ad, b_std));
  string stderror(jdl::get_std_error(*m_ad, b_std));
  string stdoutput(jdl::get_std_output(*m_ad, b_std));
  
  // TEMP patch: forward ce_id
  bool   b_ce_id;
  string ce_id(jdl::get_ce_id(*m_ad, b_ce_id));
  jdl::set_ce_id(*result, ce_id, b_ce_id);
  
  /* Mandatory */
  /* It is renamed in globusscheduler. */
  string globusresourcecontactstring(jdl::get_globus_resource_contact_string(*m_ad));
  if (globusresourcecontactstring.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::GLOBUSRESOURCE,
                                        globusresourcecontactstring,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_globus_scheduler(*result, globusresourcecontactstring);
  
  /* Mandatory */
  /* job id is mandatory to build the globusrsl string and to create the */
  /* job wrapper                                                         */
  string job_id(jdl::get_edg_jobid(*m_ad));
  if (job_id.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::JOBID,
                                        job_id,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_edg_jobid(*result, job_id);

  // keep the dag id if present
  bool b_present = false;
  string dag_id(jdl::get_edg_dagid(*m_ad, b_present));
  if (b_present && !dag_id.empty()) {
    jdl::set_edg_dagid(*result, dag_id);
  }  

  /* Mandatory */
  string requirements("EDG_WL_JDL_REQ");
  requirements.append(" '");
  string reqvalue(jdl::unparse_requirements(*m_ad));
  if (reqvalue.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::REQUIREMENTS,
                                        reqvalue,
                                        "not empty",
                                        helper_id);
  }
  utilities::replace(reqvalue, "\"", "\\\"");
  
  requirements.append(reqvalue);
  requirements.append("'");
  
  /* Mandatory */
  /* queuname is mandatory to build the globusrsl string. */ 
  string queuename(jdl::get_queue_name(*m_ad));
  if (queuename.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::QUEUENAME,
                                        queuename,
                                        "not empty",
                                        helper_id);
  }

  string globusrsl("(queue=");
  globusrsl.append(queuename);
  globusrsl.append(")(jobtype=single)");

  //globusrsl.append("(environment=(");
  //globusrsl.append(requirements);
  //globusrsl.append("))");

  /* Not Mandatory */
  bool b_hlr;
  string hlrlocation("EDG_WL_HLR_LOCATION");
  hlrlocation.append(" '");
  string hlrvalue(jdl::get_hlrlocation(*m_ad, b_hlr));
  if (!hlrvalue.empty()) {
    utilities::replace(hlrvalue, "\"", "\\\"");
    hlrlocation.append(hlrvalue);
    hlrlocation.append("'");
    globusrsl.append("(environment=(");
    globusrsl.append(hlrlocation);
    globusrsl.append(") (");
    string jid("EDG_WL_JOBID");
    jid.append(" '");
    string jidvalue(job_id);
    utilities::replace(jidvalue, "\"", "\\\"");
    jid.append(jidvalue);
    jid.append("'");
    globusrsl.append(jid);
    globusrsl.append("))");
  }
  
  /* Mandatory */
  /* x509 user proxy is mandatory for the condor submit file. */
  string userproxy(jdl::get_x509_user_proxy(*m_ad));
  if (userproxy.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDLPrivate::USERPROXY,
                                        userproxy,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_x509_user_proxy(*result, userproxy);
  
  /* Mandatory */
  string type(jdl::get_type(*m_ad));
  if (type.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::TYPE,
                                        type,
                                        "not empty",
                                        helper_id);
  }
  jdl::set_type(*result, type);
  
  /* Mandatory */
  /* job type is mandatory to build the correct job wrapper file */
  string jobtype(jdl::get_job_type(*m_ad));
  if (jobtype.empty()) {
    throw helper::InvalidAttributeValue(jdl::JDL::JOBTYPE,
                                        jobtype,
                                        "not empty",
                                        helper_id);
  }
  
  /* start preparing JobWrapper file */
  jobwrapper::JobWrapper* jw = 0;
   
  /* convert the jobid into filename */
  string jobid_to_file(jobid::to_filename(job_id));
  /* concert the dagid into filename */
  string dagid_to_file;
  if (!dag_id.empty()) {
    dagid_to_file.append(jobid::to_filename(dag_id));
  }

  /* check if there is '/' in the executable */
  if (executable[0] != '/') {
    executable.insert(0, "./", 2);
  }    
	
  // Figure out the local host name (this should really come from
  // some common entity)
  char   hostname[1024];
  string local_host_name("");
  
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
  
  /* lowercase all jobtype characters */
  string ljobtype(jobtype);
  transform(ljobtype.begin(), ljobtype.end(), ljobtype.begin(), ::tolower); 
  
  if (ljobtype == "mpich") {
    /* Mandatory */	  
    /* lrms type is mandatory for the mpich job */  
    string lrmstype(jdl::get_lrms_type(*m_ad));
    /* lowercase all lrmstype characters */
    string llrmstype(lrmstype);
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
    string nn(boost::lexical_cast<string>(nodenumber));
    
    globusrsl.append("(count=");
    globusrsl.append(nn);
    globusrsl.append(")(hostCount=");
    globusrsl.append(nn);
    globusrsl.append(")");
    
    if (llrmstype == "lsf") {
      jw = new jobwrapper::MpiLsfJobWrapper(executable);
    }
    else if (llrmstype == "pbs") {
      jw = new jobwrapper::MpiPbsJobWrapper(executable);
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
	Beginning(string("BYPASS_SHADOW_HOST="))) == env.end())
    {
      std::ostringstream env_str;
      std::copy(env.begin(), env.end(),
                std::ostream_iterator<std::string>(env_str, ":"));
      throw helper::InvalidAttributeValue(jdl::JDL::ENVIRONMENT,
                                          env_str.str(),
                                          "contains BYPASS_SHADOW_HOST",
                                          helper_id);
    }

    string::size_type pos = executable.find("./");
    if (pos == string::npos) {
      jw = new jobwrapper::InteractiveJobWrapper(executable);
    } else {
      jw = new jobwrapper::InteractiveJobWrapper(executable.substr(pos+2));
    }
  } else {
    jw = new jobwrapper::JobWrapper(executable);
  }
  
  // OutputData is not mandatory
  bool b_od;
  ExprTree* odtree = jdl::get_output_data(*m_ad, b_od);
  string    otptdata("Warning");
  if (b_od) {
    ExprList* odtree_list = dynamic_cast<ExprList*>(odtree);

    if (odtree_list) {
      jw->outputdata(odtree_list);
    } else {
      ClassAd *odtree_classad = dynamic_cast<ClassAd*>(odtree);
      if (odtree_classad) {
	auto_ptr<ExprList>   list(new ExprList);
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
    string vo(jdl::get_virtual_organisation(*m_ad));
    if (vo.empty()) {
      throw helper::InvalidAttributeValue(jdl::JDL::VIRTUAL_ORGANISATION,
                                          vo,
                                          "not empty",
                                          helper_id);
    } else {
       jw->vo(vo);	    
    }
    jw->dsupload(url::URL(job_id));
  }
	
  jdl::set_globus_rsl(*result, globusrsl);
  
  jw->standard_input(stdinput);
  jw->standard_output(stdoutput);
  jw->standard_error(stderror);
  jw->brokerinfo();
  jw->create_subdir();
  jw->arguments(arguments);
  jw->job_Id(job_id);
  jw->job_id_to_filename(jobid_to_file); 
  jw->environment(env);
  
  // Get the gatekeeper hostname
  string::size_type pos = globusresourcecontactstring.find(":");
  if (pos == 0 || pos == string::npos) {
    throw helper::InvalidAttributeValue(jdl::JDL::GLOBUSRESOURCE,
                                        globusresourcecontactstring,
                                        "contains a hostname before a :",
                                        helper_id);
  }
  jw->gatekeeper_hostname(globusresourcecontactstring.substr(0, pos));
  
  //check if there is the protocol in the inputsandbox path. 
  //if no the protocol gsiftp:// is added to the inputsandboxpath.
  if (inputsandboxpath.find("://") == string::npos) {
    string new_inputsandboxpath("gsiftp://");
    new_inputsandboxpath.append(local_host_name);
    new_inputsandboxpath.append(inputsandboxpath);
    jw->input_sandbox(url::URL(new_inputsandboxpath), inputsandbox);    
  } else { 
    jw->input_sandbox(url::URL(inputsandboxpath), inputsandbox);
  }

  //check if there is the protocol in the outputsandbox path. 
  //if no the protocol gsiftp:// is added to the outputsandboxpath.
  if (outputsandboxpath.find("://") == string::npos) {
    string new_outputsandboxpath("gsiftp://");     
    new_outputsandboxpath.append(local_host_name);
    new_outputsandboxpath.append(outputsandboxpath);
    jw->output_sandbox(url::URL(new_outputsandboxpath), outputsandbox);
  } else {  
    jw->output_sandbox(url::URL(outputsandboxpath), outputsandbox);
  }
  
  // end preparation JobWrapper file

  config::LMConfiguration const* logconfig = config::Configuration::instance()->lm();
  
  // Mandatory
  // Maradone file path
  string maradonapr(logconfig->maradona_transport_protocol());

  config::NSConfiguration const* nsconfig = config::Configuration::instance()->ns();
  
//  string maradona_name(jobid_to_file);
//  maradona_name.append("/");
//  maradona_name.append("Maradona.output");
  boost::filesystem::path maradona_path(
    boost::filesystem::normalize_path(nsconfig->sandbox_staging_path()),
    boost::filesystem::system_specific);
  maradona_path <<= jobid::get_reduced_part(job_id);
  maradona_path <<= jobid_to_file;
  maradona_path <<= "Maradona.output";
  jw->maradonaprotocol(maradonapr, maradona_path.file_path());

  config::JCConfiguration const* jcconfig = config::Configuration::instance()->jc();

  // read the submit file path
  string jw_name("JobWrapper.");
  jw_name.append(jobid_to_file);
  jw_name.append(".sh");
  boost::filesystem::path jw_path(
    boost::filesystem::normalize_path(jcconfig->submit_file_dir()),
    boost::filesystem::system_specific);
  if (!dag_id.empty()) {
    jw_path <<= jobid::get_reduced_part(dag_id);
    string jw_dagid_name("dag.");
    jw_dagid_name.append(dagid_to_file);
    jw_path <<= jw_dagid_name;
  } else {  
    jw_path <<= jobid::get_reduced_part(job_id);
  }
  jw_path <<= jw_name;

  try { 
    boost::filesystem::create_parents(jw_path.branch());
  } catch(boost::filesystem::filesystem_error &err) {
    throw CannotCreateJobWrapper(jw_path.file_path());
  }

  // write the JobWrapper in the submit file path
  ofstream ofJW(jw_path.file_path().c_str());
  if (!ofJW) {
    throw CannotCreateJobWrapper(jw_path.file_path());
  }	  
  ofJW << *jw;

  delete jw;
	
  // The jdl::get_output_data function returns a _copy_ of the object.
  delete odtree;
  
  // Mandatory
  // the new executable value is the jobwrapper file path
  jdl::set_executable(*result, jw_path.file_path());

  // Prepare the output file path
  
  string output_file_path(jcconfig->output_file_dir());
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

  // New parameter Mandatory
  // Output file path is mandatory
  string output(output_file_path);
  output.append("StandardOutput");
  jdl::set_output(*result, output);

  // New parameter Mandatory
  // Error file path is mandatory
  string error(output_file_path);
  error.append("StandardError");
  jdl::set_error_(*result, error);

  /* Mandatory */
  jdl::set_stream_output(*result, false);
  jdl::set_stream_error(*result, false);

  // Mandatory
  // Condor Log file path
  string condor_file_path(logconfig->condor_log_dir());
  string log(condor_file_path);
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


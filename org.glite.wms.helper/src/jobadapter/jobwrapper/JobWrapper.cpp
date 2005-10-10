/***************************************************************************
 *  filename  : JobWrapper.cpp
 *  authors   : Alessio Gianelle <alessio.gianelle@pd.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *              Rosario Peluso <rosario.peluso@pd.infn.it>
 *              Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <algorithm>
#include <cassert>
#include <string>
#include <fstream>

#include <classad_distribution.h>

#include "glite/wms/jdl/JobAdManipulation.h"
#include "JobWrapper.h"

namespace url = glite::wms::helper::jobadapter::url;
namespace configuration = glite::wms::common::configuration;
namespace jdl = glite::wms::jdl;

using namespace std;
using namespace classad;

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace jobwrapper {

const string JobWrapper::s_brokerinfo_default = ".BrokerInfo";

JobWrapper::JobWrapper(const string& job)
  : m_job(job), m_create_subdir(false), m_nodes(0), m_wmp_support(false), m_perusal_support(false)
{  
}

JobWrapper::~JobWrapper()
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
JobWrapper::wmp_input_sandbox_support(const url::URL& base_url,
				      const vector<string>& input_base_files)
{
  m_input_base_url = base_url;

  copy(input_base_files.begin(), input_base_files.end(), back_inserter(m_wmp_input_base_files));

  for (vector<string>::const_iterator it = input_base_files.begin();
      it != input_base_files.end(); it++) {

    string::size_type pos = it->find_last_of("/");
    if (pos == string::npos) {
      // throw ExInvalidURL("there is not ://");
      cerr << "ERROR" ;
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

void
JobWrapper::token(std::string const& token_file)
{
  m_token_file = token_file;
}

void
JobWrapper::perusal_support(void)
{
  m_perusal_support = true;
}

void
JobWrapper::perusal_timeinterval(int perusaltimeinterval)
{
  m_perusal_timeinterval = perusaltimeinterval;
}

void
JobWrapper::perusal_filesdesturi(const string& filesdesturi)
{
  m_perusal_filesdesturi = filesdesturi;
}

void 
JobWrapper::perusal_listfileuri(const string& listfileuri)
{
  m_perusal_listfileuri = listfileuri;
}

namespace {

bool 
dump_bool(std::ostream& os, const std::string& var_name, bool var_value)
{
  return os << var_name << '=' << (var_value ? 1 : 0) << '\n';
}

bool 
dump_numeric(std::ostream& os, const std::string& var_name, int var_value)
{
  return os << var_name << '=' << var_value << '\n';
}

bool 
dump_string(std::ostream& os, const std::string& var_name, const std::string& var_value)
{
  os << var_name << '=';
  if (!var_value.empty()) {
    os << '"' << var_value << '"';
  }
  return os << '\n';
}

bool 
dump_url(std::ostream& os, const std::string& var_name, const url::URL& url)
{
  string str_url = url.as_string(); //m_protocol + "://" + m_host + ((m_port != "") ? ":" + m_port : "")+ m_path;
  if (str_url[str_url.size() - 1] != '/') {
    str_url += '/';
  }
  return os << var_name << "=\"" << str_url << "\"\n";
}

bool 
dump_string_vector(std::ostream& os, const std::string& var_names, std::vector<std::string> const& values)
{
  int i = 0;

  if(values.empty()) {
    os << "declare -a " << var_names << '\n';
  }
  else {
    for(std::vector<std::string>::const_iterator it = values.begin(); it != values.end() ; ++it )  {
      os << var_names << '[' << i << "]=\"" << *it << "\"\n";
      ++i;
    }
  }
  return os;
}

bool
dump_classad_exprlist(std::ostream& os,
                      std::string var_output_file,
                      std::string var_output_lfn,
                      std::string var_output_se,
                      boost::shared_ptr<classad::ExprList> const outputdata)
{
  ExprList::const_iterator it;
  bool check;
  int i = 0;

  if( outputdata != 0 ) {
    for (it = outputdata->begin(); it != outputdata->end(); ++it) {
      ClassAd* ad = dynamic_cast<ClassAd*>(*it);
      if (ad != 0) {
        os << var_output_file << '[' << i << "]=\"" << jdl::get_output_file(*ad) << "\"\n";
        os << var_output_lfn << '[' << i << "]=\"" << jdl::get_logical_file_name(*ad, check) << "\"\n";
        os << var_output_se << '[' << i << "]=\"" << jdl::get_storage_element(*ad, check) << "\"\n";
        ++i;
      }
    }
  }
  else {
    os << "declare -a " << var_output_file << '\n';
    os << "declare -a " << var_output_lfn << '\n';
    os << "declare -a " << var_output_se << '\n';
  }
  return os;
}

} // anonymous namespace

bool 
JobWrapper::dump_vars(std::ostream& os) const
{
  return dump_string(os, "__brokerinfo", m_brokerinfo) &&
    dump_bool(os, "__create_subdir", m_create_subdir) &&
    dump_string(os, "__gatekeeper_hostname", m_gatekeeper_hostname) &&
    dump_string(os, "__jobid", m_jobid) &&
    dump_string(os, "__edg_location", "${GLITE_LOCATION:-/opt/glite}") &&
    dump_string(os, "__job", m_job) &&
    dump_string(os, "__standard_input", m_standard_input) &&
    dump_string(os, "__standard_output", m_standard_output) &&
    dump_string(os, "__standard_error", m_standard_error) &&
    dump_string(os, "__arguments", m_arguments) &&
    dump_string(os, "__maradonaprotocol", m_maradonaprotocol) &&
    dump_url(os, "__input_base_url", m_input_base_url) &&
    dump_string_vector(os, "__input_file", m_input_files) &&
    dump_url(os, "__output_base_url", m_output_base_url) &&
    dump_string_vector(os, "__output_file", m_output_files) &&
    dump_string(os, "__jobid_to_filename", m_jobid_to_filename) &&
    dump_string(os, "__globus_resource_contact_string", m_globus_resource_contact_string) &&
    dump_string_vector(os, "__environment", m_environment) &&
    dump_numeric(os, "__nodes", m_nodes) &&
    dump_string(os, "__vo", m_vo) &&
    dump_string(os, "__dsupload", m_dsupload) &&
    dump_bool(os, "__wmp_support", m_wmp_support) &&
    dump_string_vector(os, "__wmp_input_file", m_wmp_input_files) &&
    dump_string_vector(os, "__wmp_input_base_file", m_wmp_input_base_files) &&
    dump_string_vector(os, "__wmp_output_file", m_wmp_output_files) &&
    dump_string_vector(os, "__wmp_output_dest_file", m_wmp_output_dest_files) &&
    dump_string(os, "__token_file", m_token_file) &&
    dump_bool(os, "__perusal_support", m_perusal_support) &&
    dump_numeric(os, "__perusal_timeinterval", m_perusal_timeinterval) &&
    dump_string(os, "__perusal_filesdesturi", m_perusal_filesdesturi) &&
    dump_string(os, "__perusal_listfileuri", m_perusal_listfileuri) &&
    dump_bool(os, "__handle_ouput_data", m_outputdata == 0) && 
    dump_classad_exprlist(os, "__output_file", "__output_lfn", "__output_se", m_outputdata);
}

bool 
JobWrapper::fill_out_script(const std::string& template_file, std::ostream& output_stream) const
{
  std::ifstream fs(template_file.c_str());
  if (!fs) {
   output_stream << "echo \"Cannot open input file " << template_file << "\"\n";
   return false;
  }

  std::string input_line;
  getline(fs, input_line);
  if (input_line.substr(0, 2) != "#!") {
    return false;
  }
  output_stream << input_line << '\n';
  getline(fs, input_line);
  if (!input_line.empty()) {
    return false;
  }
  output_stream << '\n';
  if (!dump_vars(output_stream))
    return false;
  output_stream << '\n';
  output_stream << fs.rdbuf();

  return true;
}

ostream&
JobWrapper::print(std::ostream& os) const
{
  const configuration::WMConfiguration* wm_config
    = configuration::Configuration::instance()->wm();

  if (!fill_out_script( 
                       wm_config->job_wrapper_template_dir() 
                       + 
                       "/template.sh" ,os) ) {
    os << "echo \"Generic error occurred while writing template\"\n";
  }
  
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

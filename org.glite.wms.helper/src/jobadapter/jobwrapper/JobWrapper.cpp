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

template<typename T>
bool
dump(std::ostream& os,
  const std::string& name,
  const T& value)
{
  return os << name << '=' << value << '\n';
}

template<typename T>
bool
dump(std::ostream& os,
  const std::string& name,
  const std::vector<T>& values)
{
  int i = 0;

  for(typename std::vector<T>::const_iterator it = values.begin(); it != values.end() ; ++it )  {
    os << name << '[' << i << "]=" << *it << "\n";
    ++i;
  }

  return os;
}

bool
dump(std::ostream& os,
  const std::string& name,
  const std::vector<std::string>& values)
{
  int i = 0;

  if(values.empty()) {
    os << "declare -a " << name << '\n';
  }
  else {
    for(std::vector<std::string>::const_iterator it = values.begin(); it != values.end() ; ++it )  {
      os << name << '[' << i << "]=\"" << *it << "\"\n";
      ++i;
    }
  }

  return os;
}

bool
dump(std::ostream& os,
  const std::string& name,
  const std::string& value)
{
  os << name << '=';
  if ( !value.empty() ) {
    os << '"' << value << '"';
  }
  return os << '\n';
}

bool
dump(std::ostream& os,
  const std::string& name,
  const url::URL& url)
{
  std::string str_url = url.as_string();
  if (str_url[str_url.size() - 1] != '/') {
    str_url += '/';
  }
  return os << name << "=\"" << str_url << "\"\n";
}

bool
dump(std::ostream& os,
  const std::string& name,
  const char* var)
{
  return dump(os, name, std::string(var));
}
} // anonymous namespace

bool 
JobWrapper::dump_vars(std::ostream& os) const
{
  const configuration::WMConfiguration* const wm_config
    = configuration::Configuration::instance()->wm();

  std::vector< std::string > output_files;
  std::vector< std::string > logical_file_names;
  std::vector< std::string > storage_elements;
  bool check;

  if(m_outputdata != 0) {
    ExprList::const_iterator const it_end = m_outputdata->end();
    for (ExprList::const_iterator it = m_outputdata->begin(); it != it_end; ++it) {
      ClassAd* ad = dynamic_cast< ClassAd* >(*it);
      if (ad != 0) {
        output_files.push_back(jdl::get_output_file(*ad));
        logical_file_names.push_back(jdl::get_logical_file_name(*ad, check));
        storage_elements.push_back(jdl::get_storage_element(*ad, check));
      }
    }
  }

  return dump(os, "__brokerinfo", m_brokerinfo) &&
    dump(os, "__create_subdir", m_create_subdir) &&
    dump(os, "__gatekeeper_hostname", m_gatekeeper_hostname) &&
    dump(os, "__jobid", m_jobid) &&
    dump(os, "__job", m_job) &&
    dump(os, "__standard_input", m_standard_input) &&
    dump(os, "__standard_output", m_standard_output) &&
    dump(os, "__standard_error", m_standard_error) &&
    dump(os, "__arguments", m_arguments) &&
    dump(os, "__maradonaprotocol", m_maradonaprotocol) &&
    dump(os, "__input_base_url", m_input_base_url) &&
    dump(os, "__input_file", m_input_files) &&
    dump(os, "__output_base_url", m_output_base_url) &&
    dump(os, "__output_file", m_output_files) &&
    dump(os, "__jobid_to_filename", m_jobid_to_filename) &&
    dump(os, "__globus_resource_contact_string", 
      m_globus_resource_contact_string
    ) &&
    dump(os, "__environment", m_environment) &&
    dump(os, "__nodes", m_nodes) &&
    dump(os, "__vo", m_vo) &&
    dump(os, "__dsupload", m_dsupload) &&
    dump(os, "__wmp_support", m_wmp_support) &&
    dump(os, "__wmp_input_file", m_wmp_input_files) &&
    dump(os, "__wmp_input_base_file", 
      m_wmp_input_base_files
    ) &&
    dump(os, "__wmp_output_file", m_wmp_output_files) &&
    dump(os, "__wmp_output_dest_file", 
      m_wmp_output_dest_files
    ) &&
    dump(os, "__token_file", m_token_file) &&
    dump(os, "__token_support", m_token_support) &&
    dump(os, "__perusal_support", m_perusal_support) &&
    dump(os, "__perusal_timeinterval", m_perusal_timeinterval) &&
    dump(os, "__perusal_filesdesturi", m_perusal_filesdesturi) &&
    dump(os, "__perusal_listfileuri", m_perusal_listfileuri) &&
    dump(os, "__output_data", m_outputdata != 0) &&
    dump(os, "__output_file", output_files) &&
    dump(os, "__output_lfn", logical_file_names) &&
    dump(os, "__output_se", storage_elements) &&
    dump(os, "__max_osb_size",
      wm_config->job_wrapper_max_output_sandbox_size()
    ) &&
    dump(os, "__file_tx_retry_count", 
      wm_config->job_wrapper_file_tx_retry_count()
    );
}

bool 
JobWrapper::fill_out_script(const std::string& template_file, std::ostream& output_stream) const
{
  std::ifstream fs(template_file.c_str());
  if ( !fs ) {
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
  if ( !input_line.empty() ) {
    return false;
  }
  output_stream << '\n';
  if ( !dump_vars(output_stream) )
    return false;
  output_stream << '\n';
  output_stream << fs.rdbuf();

  return true;
}

ostream&
JobWrapper::print(std::ostream& os) const
{
  const configuration::WMConfiguration* const wm_config
    = configuration::Configuration::instance()->wm();

  if ( !fill_out_script( 
                       wm_config->job_wrapper_template_dir() 
                       + 
                       "/template.sh" ,os) )
  {
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

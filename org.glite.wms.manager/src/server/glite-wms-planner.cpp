// File: edg-wl-planner.cpp
// Author: Francesco Giacomini
// Author: Francesco Prelz
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <iostream>
#include <fstream>
#include <cstring>
#include <dlfcn.h>              // dlopen()
#include <sys/types.h>          // mkfifo()
#include <sys/stat.h>           // mkfifo()
#include <unistd.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>
#include <classad_distribution.h>

#include "signal_handling.h"
#include "CommandAdManipulation.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wms/helper/jobadapter/JobAdapter.h"

namespace manager = glite::wms::manager::server;
namespace utilities = glite::wms::common::utilities;
namespace requestad = glite::wms::jdl;
namespace configuration = glite::wms::common::configuration;
namespace common = glite::wms::manager::common;
namespace jobid = glite::wmsutils::jobid;
namespace fs = boost::filesystem;

namespace {

std::string program_name;

void usage(std::ostream& os)
{
  os
    << "Usage: " << program_name << " --help\n"
    << "   or: " << program_name << " SOURCE DEST\n";
}

std::string get_filelist_name()
{
  configuration::WMConfiguration const& wm_config
    = *configuration::Configuration::instance()->wm();
  return wm_config.input();
}

std::string get_matchpipe_dir()
{
  configuration::NSConfiguration const& ns_config
    = *configuration::Configuration::instance()->ns();
  return ns_config.list_match_root_path();
}

fs::path get_input_sandbox_path(jobid::JobId const& id)
{
  configuration::NSConfiguration const& ns_config
    = *configuration::Configuration::instance()->ns();

  return fs::path(ns_config.sandbox_staging_path(), fs::native)
    / jobid::get_reduced_part(id)
    / jobid::to_filename(id)
    / "input";  
}

class planning_error: public std::exception
{
  std::string m_error;
public:
  planning_error(std::string const& error)
    : m_error(error)
  {
  }
  char const* what() const throw()
  {
    return m_error.c_str();
  }
  ~planning_error() throw()
  {
  }
};

typedef boost::shared_ptr<classad::ClassAd> ClassAdPtr;
typedef boost::tuple<std::string, double, ClassAdPtr> match_type;
typedef std::vector<match_type> matches_type;

bool fill_matches(classad::ClassAd const& response, matches_type& matches)
try {

  std::string reason(utilities::evaluate_attribute(response, "reason"));
  if (reason != "ok") {
    throw planning_error(reason);
  }

  classad::ExprList const* match_result(
    utilities::evaluate_attribute(response, "match_result")
  );

  for (classad::ExprList::const_iterator it = match_result->begin();
       it != match_result->end(); ++it) {
    assert(utilities::is_classad(*it));
    classad::ClassAd const& match(*static_cast<classad::ClassAd const*>(*it));

    std::string ce_id(utilities::evaluate_attribute(match, "ce_id"));
    double rank(utilities::evaluate_attribute(match, "rank"));
    classad::ClassAd const* brokerinfo(
      utilities::evaluate_attribute(match, "brokerinfo")
    );
    matches.push_back(
      boost::make_tuple(
        ce_id,
        rank,
        ClassAdPtr(static_cast<classad::ClassAd*>(brokerinfo->Copy()))
      )
    );
  }

  return !matches.empty();

} catch (utilities::InvalidValue&) {
  throw planning_error(
    "invalid response: " + utilities::unparse_classad(response)
  );
}

std::auto_ptr<classad::ClassAd>
make_planned_jdl(classad::ClassAd const& jdl, match_type const& the_match)
{
  std::auto_ptr<classad::ClassAd> result(new classad::ClassAd(jdl));

  static boost::regex expression("(.+/[^\\-]+-(.+))-(.+)");
  boost::smatch pieces;
  std::string ce_id(the_match.get<0>());

  if (boost::regex_match(ce_id, pieces, expression)) {

    requestad::set_globus_resource_contact_string(
      *result,
      std::string(pieces[1].first, pieces[1].second)
    );

    requestad::set_lrms_type(
      *result,
      std::string(pieces[2].first, pieces[2].second)
    );

    requestad::set_queue_name(
      *result,
      std::string(pieces[3].first, pieces[3].second)
    );

    requestad::set_ce_id(*result, ce_id);

  } else {
    throw planning_error("invalid ce_id: " + ce_id);
  }

  return result;
}

std::string
make_match_request(classad::ClassAd const& jdl, std::string const& output_file)
{
  int const number_of_results = 10;
  bool const include_brokerinfo = true;

  boost::scoped_ptr<classad::ClassAd> cmd(
    common::match_command_create(
      new classad::ClassAd(jdl),
      output_file,
      number_of_results,
      include_brokerinfo
    )
  );

  return utilities::unparse_classad(*cmd);
}

bool get_matches(std::string& s, matches_type& matches)
{
  boost::scoped_ptr<classad::ClassAd> response(utilities::parse_classad(s));
  return response && fill_matches(*response, matches);
}

void add_brokerinfo_to_isb(classad::ClassAd& jdl)
{
  bool isb_exists = false;
  std::vector<std::string> isb;
  requestad::get_input_sandbox(jdl, isb, isb_exists);
  bool isb_base_uri_exists = false;
  std::string isb_base_uri =
    requestad::get_wmpinput_sandbox_base_uri(jdl, isb_base_uri_exists);
  if (isb_base_uri_exists) {
    isb.push_back(isb_base_uri + "/input/.BrokerInfo");
  } else {
    isb.push_back(".BrokerInfo");
  }
  requestad::set_input_sandbox(jdl, isb);
}

// alternative implementation: instead of opening a named pipe, just let the wm
// create a file with a specified name and poll the file existence

std::auto_ptr<classad::ClassAd>
Plan(classad::ClassAd const& jdl)
{
  jobid::JobId job_id(requestad::get_edg_jobid(jdl));

  // generate named pipe name
  std::string output_file(get_matchpipe_dir());
  output_file += '/' + job_id.getUnique();

  // create fifo
  if (::mkfifo(output_file.c_str(), S_IRUSR | S_IWUSR) == -1) {
    throw planning_error(
      "mkfifo " + output_file + ": " + std::strerror(errno)
    );
  }

  // be sure the file is unlinked in any case
  utilities::scope_guard unlink_file(
    boost::bind(::unlink, output_file.c_str())
  );

  // open the pipe for reading, non blocking
  int fd = open(output_file.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd == -1) {
    throw planning_error("cannot open pipe for reading");
  }

  // create a match request and push it in the wm's input fl
  { // lock
    utilities::FileList<std::string> fl(get_filelist_name());
    utilities::FileListMutex mx(fl);
    utilities::FileListLock lock(mx);
    fl.push_back(make_match_request(jdl, output_file));
  }

  std::string match_string("");

  bool read_done = false;

  for (;;) {
    char read_buffer[1024];
    int read_retcod;

    // read response from fifo
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    int retval = select(fd + 1, &read_fds, 0, 0, 0);
    if (retval != 1) {
      close(fd);
      throw planning_error(
        "select failed with errno " + boost::lexical_cast<std::string>(errno)
      );
    }
    if (!FD_ISSET(fd, &read_fds)) {
      close(fd);
      throw planning_error("select returned the wrong file descriptor");
    }
    read_retcod = read(fd, read_buffer, sizeof(read_buffer));
    if (read_retcod < 0) {
      if (errno == EAGAIN) continue;
      else {
        close(fd);
        throw planning_error(
            "error reading the match file with errno " + 
            boost::lexical_cast<std::string>(errno));
      }
    } else if (read_retcod == 0) break;
    match_string.append(read_buffer, read_retcod);
  }
  close(fd);
  matches_type matches;
  if (!get_matches(match_string, matches)) {
    throw planning_error("no matching resource");
  }

  // choose one of the matches, e.g. the first one
  match_type const& the_match = matches[0];

  // create a new jdl based on the chosen match
  std::auto_ptr<classad::ClassAd> planned_jdl(
    make_planned_jdl(jdl, the_match)
  );

  // dump the brokerinfo to file
  ClassAdPtr brokerinfo = the_match.get<2>();
  classad::ExprTree const* DAC = jdl.Lookup("DataAccessProtocol");
  if (DAC) {
    brokerinfo->Insert("DataAccessProtocol", DAC->Copy());
  }

  fs::path brokerinfo_file(get_input_sandbox_path(job_id) / ".BrokerInfo");
  std::ofstream brokerinfo_os(brokerinfo_file.native_file_string().c_str());
  assert(brokerinfo_os);

  brokerinfo_os << utilities::unparse_classad(*brokerinfo) << std::endl;

  add_brokerinfo_to_isb(*planned_jdl);

  // job adapting
  std::auto_ptr<classad::ClassAd> result(
    glite::wms::helper::jobadapter::JobAdapter(planned_jdl.get()).resolve()
  );

  return result;
}

} // {anonymous}

int
main(int argc, char* argv[])
try {
  program_name = argv[0];

  if (argc == 2 && std::string(argv[1]) == "--help") {

    usage(std::cout);
    return EXIT_SUCCESS;

  } else if (argc != 3) {

    usage(std::cerr);
    return EXIT_FAILURE;

  }

  manager::signal_handling_init();

  configuration::Configuration config(
    "glite_wms.conf",
    configuration::ModuleType::workload_manager
  );

  std::string input_file(argv[1]);
  std::ifstream is(input_file.c_str());
  if (!is) {
    std::cerr << "Cannot open input file " << input_file << '\n';
    return EXIT_FAILURE;
  }
  std::string output_file(argv[2]);
  std::ofstream os(output_file.c_str());
  if (!os) {
    std::cerr << "Cannot open output file " << output_file << '\n';
    return EXIT_FAILURE;
  }

  boost::scoped_ptr<classad::ClassAd> input_ad(utilities::parse_classad(is));
  boost::scoped_ptr<classad::ClassAd> output_ad(Plan(*input_ad));

  if (!output_ad) {
    std::cerr << "Planning failed for unknown reason\n";
    return EXIT_FAILURE;
  }

  os << utilities::unparse_classad(*output_ad) << '\n';

  std::cout << requestad::get_ce_id(*output_ad) << '\n';

} catch (std::exception const& e) {

  std::cerr << e.what() << "\n";
  return EXIT_FAILURE;

} catch (...) {

  std::cerr << "unknown exception\n";
  return EXIT_FAILURE;

}

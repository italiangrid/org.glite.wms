#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/program_options.hpp>

#include "lb_utils.h"
#include "purger.h"

#include "glite/jobid/JobId.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMPConfiguration.h"

#include <iostream>                        // for cout
#include <string>
#include <vector>
#include <cmath>
#include <time.h>
#include <fstream>
#include <sys/vfs.h>

namespace fs            = boost::filesystem;
namespace wl	        = glite::wms;
namespace configuration = glite::wms::common::configuration;
namespace jobid         = glite::jobid;
namespace po            = boost::program_options;

namespace {

const configuration::Configuration* f_conf = 0;

bool
find_directories(
  const fs::path & from_path,
  const std::string &prefix,
  std::vector<fs::path>& path_found,
  bool recursive = false
) {
  if (!fs::exists(from_path)) {
    return false;
  }
  fs::directory_iterator end_itr; // default construction yields past-the-end
  for (fs::directory_iterator itr( from_path );
       itr != end_itr;
       ++itr) {
    if (fs::exists(*itr))
    try { 	
      if (fs::is_directory( *itr )) {
	   if (itr->leaf().substr(0,prefix.length()) == prefix) path_found.push_back( *itr );
	   else if (recursive && find_directories( *itr, prefix, path_found )) return true;
      }
    }
    catch( fs::filesystem_error& e) {
	std::cerr << e.what() << std::endl;
    }	
  }
  return false;
}

std::string
get_staging_path()
{
  if (!f_conf) {
    f_conf = configuration::Configuration::instance();
    assert(f_conf);
  }
  static std::string const sandbox_staging_path(
    f_conf->wp()->sandbox_staging_path()
  );
  return sandbox_staging_path;
}
}

int main( int argc, char* argv[])
{
  try {
    po::options_description desc("Usage");
    desc.add_options()
      ("help,h", "display this help and exit")
      (
        "conf-file,c",
        po::value<std::string>(),
        "configuration file"
      )
      (
        "threshold,t",
        po::value<int>(),
        "sets the purging threshold to the specified number of seconds"
      )
      (
        "allocated-limit,a",
        po::value<int>(),
        "defines the percentange of allocated blocks which triggers the"
        " purging"
      )
      (
        "skip-threshold-checking",
        "does not perform the threshold check before purging"
      )
      (
        "skip-status-checking",
        "does not perform any status checking before purging"
      )
      ("force-orphan-node-removal", "force removal of orphan dag nodes")
      ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      return EXIT_SUCCESS;
    }

    configuration::Configuration config(
      vm.count("conf-file")
      ? vm["conf-file"].as<std::string>()
      : "glite_wms.conf",
      configuration::ModuleType::workload_manager
    );

    int const allocated_limit(
      vm.count("allocated-limit") ? vm["allocated-limit"].as<int>(): 0
    );
    if (allocated_limit) {
      struct statfs fs_stat;
      if (!statfs(get_staging_path().c_str(), &fs_stat)) {
        int const allocated(
          static_cast<int>(
            (1.0 - (static_cast<double>(fs_stat.f_bfree) / fs_stat.f_blocks))
            * 100
          )
        );
        if (allocated < allocated_limit) {
          return EXIT_SUCCESS;
	}
      }
    }

    wl::purger::Purger thePurger;

    thePurger.threshold(
      vm.count("threshold") ? vm["threshold"].as<int>() : 604800
    ).
    skip_threshold_checking(
      vm.count("skip-threshold-checking")
    ).
    skip_status_checking(
      vm.count("skip-status-checking")
    ).
    force_orphan_node_removal(
      vm.count("force-orphan-node-removal")
    ).
    log_using(
      edg_wll_LogClearTIMEOUT
    );

   std::vector<fs::path> found_path;
   fs::path from_path( get_staging_path(), fs::native);
   find_directories(from_path, "https", found_path, true);

   std::vector<fs::path>::const_iterator i = found_path.begin();
   std::vector<fs::path>::const_iterator const e = found_path.end();

   for( ; i != e ; ++i) {
#warning FIXME extract the id from somewhere
     jobid::JobId id;//(jobid::from_filename( i->leaf()));
     thePurger( id );
   }
  }
  catch (boost::program_options::unknown_option const& e) {
    std::cerr<< e.what() << '\n';
  } catch (std::exception const& e) {
    std::cerr << e.what() << '\n';
  } catch (...) {
    std::cerr << "unknown exception\n";
  }
  return EXIT_FAILURE;
}

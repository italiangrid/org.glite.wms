// File: submission_utils.cpp
// Author: Francesco Giacomini
// Copyright (c) Members of the EGEE Collaboration 2004
// For license conditions see http://public.eu-egee.org/license/license.html

// $Id$

#include "submission_utils.h"
#include <classad_distribution.h>
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/ManipulationExceptions.h"

namespace fs = boost::filesystem;
namespace configuration = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

int get_max_shallow_count()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  return config.wm()->max_shallow_retry_count();
}

int get_max_retry_count()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  return config.wm()->max_retry_count();
}

std::string get_token_file()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  return config.wm()->token_file();
}

fs::path sandbox_dir()
{
  configuration::Configuration const& config(
    *configuration::Configuration::instance()
  );

  std::string const path_str = config.ns()->sandbox_staging_path();

  return fs::path(path_str, fs::native);
}

}

fs::path get_reallyrunning_token(jobid::JobId const& id)
{
  fs::path result(sandbox_dir());
  result /= fs::path(jobid::get_reduced_part(id), fs::native);
  result /= fs::path(jobid::to_filename(id), fs::native);
  result /= fs::path(get_token_file(), fs::native);

  return result;
}

// in practice the actual retry limit for the deep count is
// max(0, min(job_retry_count, max_retry_count))
// and similarly for the shallow count
// max(0, min(job_shallow_count, max_shallow_count))

void check_shallow_count(classad::ClassAd const& jdl, int count)
{
  // check against the job max shallow count
  bool valid = false;
  int job_shallow_count(
    jdl::get_shallow_retry_count(jdl, valid)
  );
  if (!valid || job_shallow_count < 0) {
    job_shallow_count = 0;
  }
  if (count >= job_shallow_count) {
    throw HitJobShallowCount(job_shallow_count);
  }

  // check against the system max shallow count
  int max_shallow_count = get_max_shallow_count();
  if (max_shallow_count < 0) {
    max_shallow_count = 0;
  }
  if (count >= max_shallow_count) {
    throw HitMaxShallowCount(max_shallow_count);
  }
}

void check_deep_count(classad::ClassAd const& jdl, int count)
{
  // check against the job max retry count
  bool valid = false;
  int job_retry_count(jdl::get_retry_count(jdl, valid));
  if (!valid || job_retry_count < 0) {
    job_retry_count = 0;
  }
  if (count >= job_retry_count) {
    throw HitJobRetryCount(job_retry_count);
  }

  // check against the system max retry count
  int max_retry_count = get_max_retry_count();
  if (count >= max_retry_count) {
    throw HitMaxRetryCount(max_retry_count);
  }
}

int get_job_shallow_count(classad::ClassAd const& jdl)
{
  bool valid = false;
  int result = jdl::get_shallow_retry_count(jdl, valid);
  if (!valid) {
    result = 0;
  }
  return result;
}

bool shallow_resubmission_is_disabled(classad::ClassAd const& jdl)
{
  return get_job_shallow_count(jdl) == -1;
}

void create_token(fs::path const& p)
{
  fs::ofstream _(p);
}

}}}} // glite::wms::manager::server

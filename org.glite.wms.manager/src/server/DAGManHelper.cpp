// File: DAGManHelper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2003 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "DAGManHelper.h"
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "glite/wms/common/utilities/boost_fs_add.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <classad_distribution.h>
#include "glite/wms/helper/HelperFactory.h"
#include "glite/wms/helper/exceptions.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/PrivateAdManipulation.h"
#include "glite/wms/jdl/DAGAd.h"
#include "glite/wms/jdl/DAGAdManipulation.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/scope_guard.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/JCConfiguration.h"
#include "glite/wms/common/configuration/LMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/logger/logger_utils.h"

namespace fs = boost::filesystem;
namespace utilities = glite::wms::common::utilities;
namespace configuration = glite::wms::common::configuration;
namespace jobid = glite::wmsutils::jobid;
namespace jdl = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

std::string helper_id("DAGManHelper");

helper::HelperImpl* create_helper()
{
  return new DAGManHelper;
}

struct Register
{
  Register()
  {
    helper::HelperFactory::instance()->register_helper(helper_id, create_helper);
  }
  ~Register()
  {
    helper::HelperFactory::instance()->unregister_helper(helper_id);
  }
};

Register r;

std::string f_output_file_suffix(".dmh");

classad::ClassAd const*
get_description(classad::ExprTree const* node)
try {
  classad::ClassAd const* result = 0;

  if (utilities::is_classad(node)) {
    result = utilities::evaluate_attribute(
      *dynamic_cast<classad::ClassAd const*>(node),
      "description"
    );
  }

  return result;

} catch (utilities::InvalidValue&) {
  return 0;
}

fs::path
jc_submit_file_dir()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::JCConfiguration const* const jc_config = config->jc();
  assert(jc_config);

  std::string path_str = jc_config->submit_file_dir();

  return fs::path(path_str, fs::system_specific);
}

fs::path
jc_output_file_dir()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::JCConfiguration const* const jc_config = config->jc();
  assert(jc_config);

  std::string path_str = jc_config->output_file_dir();

  return fs::path(path_str, fs::system_specific);
}

fs::path
lm_condor_log_dir()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::LMConfiguration const* const lm_config = config->lm();
  assert(lm_config);

  std::string path_str = lm_config->condor_log_dir();

  return fs::path(path_str, fs::system_specific);
}

fs::path
sandbox_dir()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::NSConfiguration const* const ns_config = config->ns();
  assert(ns_config);

  std::string path_str = ns_config->sandbox_staging_path();

  return fs::path(path_str, fs::system_specific);
}

std::string
get_condor_dagman()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::JCConfiguration const* const jc_config = config->jc();
  assert(jc_config);

  return jc_config->condor_dagman();
}

int
get_dagman_log_level()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::JCConfiguration const* const jc_config = config->jc();
  assert(jc_config);

  return jc_config->dagman_log_level();
}

int
get_dagman_log_rotate()
{
  configuration::Configuration const* const config
    = configuration::Configuration::instance();
  assert(config);

  configuration::JCConfiguration const* const jc_config = config->jc();
  assert(jc_config);

  return jc_config->dagman_log_rotate();
}

std::string const dag_description_file("dag_description.con");

class Paths
{
  jobid::JobId m_dag_id;
  fs::path m_base_submit_dir;
  fs::path m_base_output_dir;
  fs::path m_dag_log;
  fs::path m_sandbox_dir;

public:
  Paths(jobid::JobId const& dag_id)
    : m_dag_id(dag_id),
      m_base_submit_dir(jc_submit_file_dir() << jobid::get_reduced_part(dag_id) << ("dag." + jobid::to_filename(dag_id))),
      m_base_output_dir(jc_output_file_dir() << jobid::get_reduced_part(dag_id) << jobid::to_filename(dag_id)),
      m_dag_log(lm_condor_log_dir() << ("dag." + jobid::to_filename(dag_id) + ".log")),
      m_sandbox_dir(sandbox_dir())
  {}

  jobid::JobId dag_id() const
  {
    return m_dag_id;
  }
  fs::path base_submit_dir() const
  {
    return m_base_submit_dir;
  }
  fs::path base_output_dir() const
  {
    return m_base_output_dir;
  }
  fs::path dag_description() const
  {
    return m_base_submit_dir << dag_description_file;
  }
  std::string lock_file() const
  {
    return dag_description_file + ".lock"; // it must be this way
  }
  fs::path lib_log() const
  {
    return m_base_output_dir << "dag.lib.out";
  }
  fs::path debug_log() const
  {
    return m_base_output_dir << "dag.dagman.out";
  }
  fs::path dag_log() const
  {
    return m_dag_log;
  }
  fs::path rescue() const
  {
    return m_base_submit_dir << dag_description_file + ".rescue";
  }
  fs::path ad(jobid::JobId const& node_id) const
  {
    return m_base_submit_dir << "ad." + jobid::to_filename(node_id);
  }
  fs::path submit(jobid::JobId const& node_id) const
  {
    return m_base_submit_dir << "Condor." + jobid::to_filename(node_id) + ".submit";
  }
  fs::path pre(jobid::JobId const& node_id) const
  {
    std::string const edg_wl_location = std::getenv("EDG_WL_LOCATION");
    return fs::path(edg_wl_location, fs::system_specific) << "libexec/edg-wl-planner.sh";
  }
  fs::path post(jobid::JobId const& node_id) const
  {
    std::string const edg_wl_location = std::getenv("EDG_WL_LOCATION");
    return fs::path(edg_wl_location, fs::system_specific) << "libexec/edg-wl-dag-post.sh";
  }
  fs::path standard_output(jobid::JobId const& node_id) const
  {
    //    return m_base_output_dir << jobid::get_reduced_part(node_id) << jobid::to_filename(node_id) << "StandardOutput";
    return m_base_output_dir << jobid::to_filename(node_id) << "StandardOutput";
  }
  fs::path maradona(jobid::JobId const& node_id) const
  {
    return m_sandbox_dir << jobid::get_reduced_part(node_id) << jobid::to_filename(node_id) << "Maradona.output";
  }

};

class DescriptionAdToSubmitFile
{
  jdl::DAGAd* m_dagad;
  Paths const* m_paths;

public:
  DescriptionAdToSubmitFile(jdl::DAGAd* dagad, Paths const* paths)
    : m_dagad(dagad), m_paths(paths)
  {
  }
  void operator()(jdl::DAGAd::node_value_type const& node) const
  {
    std::string const& name      = node.first;
    jdl::DAGNodeInfo const& node_info = node.second;

    classad::ClassAd const* ad = node_info.description_ad();
    assert(ad);

    jobid::JobId node_id = jdl::get_edg_jobid(*ad);

    fs::path submit_file = m_paths->submit(node_id);
    fs::ofstream os(submit_file);
    assert(os);
    os << "log = " << m_paths->dag_log().file_path() << "\n";

    jdl::DAGNodeInfo new_node_info = node_info;
    new_node_info.description_file_for_ad(submit_file.file_path());
    // TODO should change node type too
    m_dagad->replace_node(name, new_node_info);
  }
};

// <node>.<file>
boost::regex node_file_regex("([a-zA-Z0-9_]+)\\.(.*)");

std::pair<std::string,std::string>
get_node_file(classad::ExprTree const* et)
{
  std::string s(utilities::unparse(et));
  boost::smatch m;
  std::string node;
  std::string file;
  if (boost::regex_match(s, m, node_file_regex)) {
    node.assign(m[1].first, m[1].second);
    file.assign(m[2].first, m[2].second);
  }

  return std::make_pair(node, file);
}

// consider the ISB; replace references to files in other node OSBs with actual
// paths
class NodeAdTransformation
{
  jdl::DAGAd const* m_dagad;
public:
  NodeAdTransformation(jdl::DAGAd const* dagad)
    : m_dagad(dagad)
  {
  }
  classad::ClassAd* operator()(classad::ClassAd const& ad) const
  {
    std::auto_ptr<classad::ClassAd> result(static_cast<classad::ClassAd*>(ad.Copy()));

    jdl::set_x509_user_proxy(*result, jdl::get_x509_user_proxy(*m_dagad));
    jdl::set_certificate_subject(*result, jdl::get_certificate_subject(*m_dagad));
    jdl::set_edg_dagid(*result, jdl::get_edg_jobid(*m_dagad));

    // manage isb
    boost::scoped_ptr<classad::ExprTree const> et(result->Remove(jdl::JDL::INPUTSB));
    if (!et) {
      return result.release();
    }

    // can it be a single value or is it always a list?
    // assume the latter for the moment
    classad::ExprList const* el = dynamic_cast<classad::ExprList const*>(et.get());    

    std::auto_ptr<classad::ExprList> new_isb(new classad::ExprList);

    for (classad::ExprList::const_iterator it = el->begin(); it != el->end(); ++it) {

      if (utilities::is_attribute_reference(*it)) {
        std::string node;
        std::string file;
        boost::tie(node, file) = get_node_file(*it);
        if (!node.empty() && !file.empty()) {
          jdl::DAGAd::node_iterator node_it = m_dagad->find(node);
          if (node_it != m_dagad->nodes().second) {
            classad::Value v;
            jdl::DAGNodeInfo const& node_info = node_it->second;
            classad::ClassAd const* ad = node_info.description_ad();
            assert(ad);

            bool exists = false;
            std::string osb_path(jdl::get_output_sandbox_path(*ad, exists));
            if (exists && !osb_path.empty()) {
              v.SetStringValue(osb_path + "/" + file);
              new_isb->push_back(classad::Literal::MakeLiteral(v));
            }
          }
        }
      } else {
        new_isb->push_back((*it)->Copy());
      }

    }

    result->Insert(jdl::JDL::INPUTSB, new_isb.release());

    return result.release();
  }

};

class DumpAdToFile
{
  typedef boost::function<classad::ClassAd*(classad::ClassAd)> transform_function;

  transform_function m_transform;
  Paths const* m_paths;

public:
  DumpAdToFile(transform_function transform, Paths const* paths)
    : m_transform(transform), m_paths(paths)
  {
  }
  void operator()(jdl::DAGAd::node_value_type const& node) const
  {
    jdl::DAGNodeInfo const& node_info = node.second;

    classad::ClassAd const* ad = node_info.description_ad();
    assert(ad);

    fs::path ad_file = m_paths->ad(jdl::get_edg_jobid(*ad));
    fs::ofstream os(ad_file);
    assert(os);

    boost::scoped_ptr<classad::ClassAd> transformed_ad(m_transform(*ad));
    assert(transformed_ad);
    os << utilities::unparse_classad(*transformed_ad) << "\n";
  }
};

class GeneratePrePost
{
  jdl::DAGAd* m_dagad;
  Paths const* m_paths;

public:
  GeneratePrePost(jdl::DAGAd* dagad, Paths const* paths)
    : m_dagad(dagad), m_paths(paths)
  {
  }
  void operator()(jdl::DAGAd::node_value_type const& node) const
  {
    std::string const& name      = node.first;
    jdl::DAGNodeInfo const& node_info = node.second;

    jdl::DAGNodeInfo new_node_info = node_info;
    classad::ClassAd const* ad = node_info.description_ad();
    assert(ad);
    jobid::JobId node_id = jdl::get_edg_jobid(*ad);

    std::string pre_file = m_paths->pre(node_id).file_path();
    std::string pre_args = m_paths->ad(node_id).file_path()
      + " " + m_paths->submit(node_id).file_path();
    new_node_info.pre(pre_file, pre_args);

    std::string post_file = m_paths->post(node_id).file_path();
    std::string post_args = m_paths->standard_output(node_id).file_path() + " "
      + m_paths->maradona(node_id).file_path();
    new_node_info.post(post_file, post_args);

    m_dagad->replace_node(name, new_node_info);
  }
};

struct InsertJobInSubmitFile
{
  std::ostream* operator()(std::ostream* osp, const jdl::DAGAd::node_value_type& node) const
  {
    std::ostream& os = *osp;

    std::string const& node_name = node.first;
    jdl::DAGNodeInfo const& node_info = node.second;

    os << "JOB " << node_name << " " << node_info.description_file() << "\n";

    std::string pre;
    std::string pre_args;
    boost::tie(pre, pre_args) = node_info.pre();
    if (!pre.empty()) {
      os << "SCRIPT PRE " << node_name << " " << pre;
      if (!pre_args.empty()) {
        os << " " << pre_args;
      }
      os << "\n";
    }

    std::string post;
    std::string post_args;
    boost::tie(post, post_args) = node_info.post();
    if (!post.empty()) {
      os << "SCRIPT POST " << node_name << " " << post;
      if (!post_args.empty()) {
        os << " " << post_args;
      }
      os << "\n";
    }

    int retry_count = node_info.retry_count();
    if (retry_count > 0) {
      os << "RETRY " << node_name << " " << node_info.retry_count() << "\n";
    }

    return osp;
  }
};

struct InsertDependencyInSubmitFile
{
  std::ostream* operator()(
    std::ostream* osp,
    const jdl::DAGAd::dependency_value_type& dependency
  ) const
  {
    std::ostream& os = *osp;

    os << "PARENT " << dependency.first->first
       << " CHILD " << dependency.second->first
       << "\n";

    return osp;
  }
};

std::ostream&
to_dag_description(std::ostream& os, const jdl::DAGAd& ad)
{
  jdl::DAGAd::node_iterator node_b;
  jdl::DAGAd::node_iterator node_e;
  boost::tie(node_b, node_e) = ad.nodes();
  std::accumulate(node_b, node_e, &os, InsertJobInSubmitFile());

  std::accumulate(ad.dependencies().first, ad.dependencies().second, &os,
                  InsertDependencyInSubmitFile());

  return os;
}

void create_dagman_job_ad(classad::ClassAd& result, Paths const& paths)
{
  jdl::set_type(result, "dag");
  jdl::set_universe(result, "scheduler");
  std::string condor_dagman(get_condor_dagman());
  jdl::set_executable(result, condor_dagman);
  jdl::set_getenv(result, "true");

  jdl::set_output(result, paths.lib_log().file_path());
  jdl::set_error_(result, paths.lib_log().file_path());
  jdl::set_remove_kill_sig(result, "SIGUSR1");

  int dagman_log_level = get_dagman_log_level();
  if (dagman_log_level < 0) {
    dagman_log_level = 0;
  } else if (dagman_log_level > 5) {
    dagman_log_level = 5;
  }
  std::ostringstream arguments;
  arguments << "-f"
            << " -l " << paths.base_submit_dir().file_path()
            << " -Debug " << dagman_log_level
            << " -Lockfile " << paths.lock_file()
            << " -Dag " << paths.dag_description().file_path()
            << " -Rescue " << paths.rescue().file_path()
            << " -Condorlog " << paths.dag_log().file_path();
  jdl::set_arguments(result, arguments.str());
  jdl::set_ce_id(result, "dagman");

  int dagman_log_rotate = get_dagman_log_rotate();
  if (dagman_log_rotate < 0) {
    dagman_log_rotate = 0;
  }
  std::ostringstream environment;
  environment << "_CONDOR_DAGMAN_LOG=" << paths.debug_log().file_path()
              << ";_CONDOR_MAX_DAGMAN_LOG=" << dagman_log_rotate;
  result.InsertAttr("environment", environment.str());
}

classad::ClassAd* f_resolve(classad::ClassAd const& input_ad)
try {

  std::auto_ptr<classad::ClassAd> result(new classad::ClassAd);

  jdl::DAGAd dagad(input_ad);

  std::string const dag_id_str = jdl::get_edg_jobid(dagad);
  jobid::JobId const dag_id(dag_id_str);

  jdl::set_edg_jobid(*result, dag_id_str);

  jdl::set_x509_user_proxy(*result, jdl::get_x509_user_proxy(dagad));

  Paths paths(dag_id);

  fs::create_parents(paths.base_submit_dir());
  utilities::scope_guard base_submit_dir_undo(
    boost::bind(fs::remove_all, paths.base_submit_dir())
  );

  jdl::DAGAd::node_iterator node_b;
  jdl::DAGAd::node_iterator node_e;

  // in the following assume for the moment that all node infos contain a
  // description ad rather than a description file

  // dump description ads to file, applying a transformation first
  boost::tie(node_b, node_e) = dagad.nodes();
  NodeAdTransformation node_ad_transformation(&dagad);
  std::for_each(node_b, node_e, DumpAdToFile(node_ad_transformation, &paths));

  // generate pre and post scripts
  boost::tie(node_b, node_e) = dagad.nodes();
  std::for_each(node_b, node_e, GeneratePrePost(&dagad, &paths));

  // replace description ads with (submit) files
  // generate a preliminary submit file containing just the log attribute to
  // make dagman happy. The submit file will be overwritten (better: completed)
  // by the Big Helper
  boost::tie(node_b, node_e) = dagad.nodes(); // node_iterator is an input iterator
  std::for_each(node_b, node_e, DescriptionAdToSubmitFile(&dagad, &paths));

  // TODO: what to do if an operation fails, in particular the generation of a
  // file? undo the previous calls?

  // generate dag description
  fs::ofstream dag_description(paths.dag_description());
  assert(dag_description);
  to_dag_description(dag_description, dagad) << "\n";

  // create the job ad, to be then converted by the JC to a submit file
  create_dagman_job_ad(*result, paths);

  // touch the lock file to force dagman to start in recovery mode. This is
  // needed because currently dagman (in non-recovery mode) removes the log file
  // at start up if it exists. But this causes the dagman submission event to be
  // lost because we use the same log file both for the dagman job and for its
  // subjobs
  fs::ofstream lf(paths.base_submit_dir() << paths.lock_file());
  assert(lf);

  base_submit_dir_undo.dismiss();

  return result.release();

} catch (jdl::InvalidDAG const& e) {
  throw DAGManHelperError();
} catch (fs::filesystem_error const& ex) {
  throw helper::FileSystemError("DAGManHelper", ex);
} catch (utilities::ClassAdError& ex) {
  Error(ex.what());
  return 0;
}

} // {anonymous}

std::string
DAGManHelper::id() const
{
  return helper_id;
}

std::string
DAGManHelper::output_file_suffix() const
{
  return f_output_file_suffix;
}

classad::ClassAd*
DAGManHelper::resolve(classad::ClassAd const* input_ad) const
{
  // not yet
  // assert(input_ad != 0);

  return input_ad ? f_resolve(*input_ad) : 0;

  // not yet
  //  assert(result != 0);
}

} // server
} // manager
} // wms
} // glite 

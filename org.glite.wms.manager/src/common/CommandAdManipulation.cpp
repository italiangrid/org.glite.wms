// File: CommandAdManipulation.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "CommandAdManipulation.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/PrivateAttributes.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include <algorithm>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <classad_distribution.h>

namespace utilities = glite::wms::common::utilities;
namespace requestad = glite::wms::jdl;

namespace glite {
namespace wms {
namespace manager {
namespace common {

namespace {
std::string const command_requirements(
  "[requirements="
  "  other.version==\"1.0.0\""
  "  && isString(other.command)"
  "  && isClassad(other.arguments)"
  "  && (other.command == \"jobsubmit\""
  "      && isClassad(other.arguments.ad)"
  "      && isString(other.arguments.ad.edg_jobid)"
  "      && isString(other.arguments.ad.lb_sequence_code)"
  "      && isString(other.arguments.ad.X509UserProxy)"
  "      || other.command == \"jobresubmit\""
  "      && isString(other.arguments.id)"
  "      && isString(other.arguments.lb_sequence_code)"
  "      || other.command == \"jobcancel\""
  "      && isString(other.arguments.id)"
  "      && isString(other.arguments.lb_sequence_code)"
  "      || other.command == \"match\""
  "      && isClassad(other.arguments.ad)"
  "      && isString(other.arguments.ad.CertificateSubject)"
  "      && isString(other.arguments.file)"
  "      && (isUndefined(other.arguments.number_of_results)"
  "          || isInteger(other.arguments.number_of_results))"
  "      && (isUndefined(other.arguments.include_brokerinfo)"
  "          || isBoolean(other.arguments.include_brokerinfo))"
  "     )"
  "]"
);

} // {anonymous}

bool
command_is_valid(classad::ClassAd const& command_ad)
{
  static classad::ClassAd const* command_ad_requirements(
    utilities::parse_classad(command_requirements)
  );
  return utilities::left_matches_right(command_ad, *command_ad_requirements);
}

std::string
command_get_command(classad::ClassAd const& command_ad)
{
  std::string result = utilities::evaluate_attribute(command_ad, "command");

  std::transform(result.begin(), result.end(), result.begin(), ::tolower);

  return result;
}

std::auto_ptr<classad::ClassAd>
submit_command_create(classad::ClassAd* job_ad)
{
  std::auto_ptr<classad::ClassAd> result;
  std::string jobid = requestad::get_edg_jobid(*job_ad);

  if (!jobid.empty())
  {
    result.reset(new classad::ClassAd);
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobsubmit"));
    std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
    args->Insert("ad", job_ad);
    result->Insert("arguments", args.get());
    args.release();
  }

  return result;
}

classad::ClassAd const*
submit_command_get_ad(classad::ClassAd const& submit_command_ad)
{
  return utilities::evaluate_expression(submit_command_ad, "arguments.ad");
}

classad::ClassAd*
submit_command_remove_ad(classad::ClassAd& submit_command_ad)
{
  return
    static_cast<classad::ClassAd*>(
      static_cast<classad::ClassAd*>(
        submit_command_ad.Lookup("arguments")
      )->Remove("ad")
    );
}

std::auto_ptr<classad::ClassAd>
resubmit_command_create(std::string const& job_id, std::string const& sequence_code)
{
  std::auto_ptr<classad::ClassAd> result;

  if (!job_id.empty()) {
    result.reset(new classad::ClassAd);
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobresubmit"));
    std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
    args->InsertAttr("id", job_id);
    args->InsertAttr("lb_sequence_code", sequence_code);
    result->InsertAttr("arguments", args.get());
    args.release();
  }

  return result;
}

std::string
resubmit_command_get_id(classad::ClassAd const& command_ad)
{
  return utilities::evaluate_expression(command_ad, "arguments.id");
}

std::string
resubmit_command_get_lb_sequence_code(classad::ClassAd const& command_ad)
{
  return utilities::evaluate_expression(command_ad, "arguments.lb_sequence_code");
}

std::auto_ptr<classad::ClassAd>
cancel_command_create(std::string const& job_id,
  std::string const& sequence_code,
  std::string const& user_x509_proxy)
{
  std::auto_ptr<classad::ClassAd> result;

  if (!job_id.empty()) {
    result.reset(new classad::ClassAd);
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobcancel"));
    std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
    args->InsertAttr(requestad::JDL::JOBID, job_id);
    args->InsertAttr(requestad::JDL::LB_SEQUENCE_CODE, sequence_code);
    args->InsertAttr(JDLPrivate::USERPROXY, user_x509_proxy);
    
    result->InsertAttr("arguments", args.get());
  }

  return result;
}

std::string
cancel_command_get_id(classad::ClassAd const& cancel_command_ad)
{
  return utilities::evaluate_expression(cancel_command_ad, "arguments.id");
}

std::string
cancel_command_get_lb_sequence_code(classad::ClassAd const& command_ad)
{
  return utilities::evaluate_expression(command_ad, "arguments.lb_sequence_code");
}

classad::ClassAd*
match_command_create(
  classad::ClassAd* jdl,
  std::string const& file,
  int number_of_results,
  bool include_brokerinfo
)
{
  std::auto_ptr<classad::ClassAd> result(new classad::ClassAd);

  result->InsertAttr("version", std::string("1.0.0"));
  result->InsertAttr("command", std::string("match"));
  std::auto_ptr<classad::ClassAd> args(new classad::ClassAd);
  args->Insert("ad", jdl);
  args->InsertAttr("file", file);
  args->InsertAttr("number_of_results", number_of_results);
  args->InsertAttr("include_brokerinfo", include_brokerinfo);
  result->Insert("arguments", args.release());

  return result.release();
}

classad::ClassAd const*
match_command_get_ad(classad::ClassAd const& match_command_ad)
{
  return utilities::evaluate_expression(match_command_ad, "arguments.ad");
}

classad::ClassAd*
match_command_remove_ad(classad::ClassAd& match_command_ad)
{
  return
    static_cast<classad::ClassAd*>(
      static_cast<classad::ClassAd*>(
        match_command_ad.Lookup("arguments")
      )->Remove("ad")
    );
}

std::string
match_command_get_file(classad::ClassAd const& match_command_ad)
{
  return utilities::evaluate_expression(match_command_ad, "arguments.file");
}

int match_command_get_number_of_results(classad::ClassAd const& match_command_ad)
try {
  return utilities::evaluate_expression(match_command_ad, "arguments.number_of_results");
} catch (utilities::InvalidValue&) {
  return -1;
}

bool match_command_get_include_brokerinfo(classad::ClassAd const& match_command_ad)
try {
  return utilities::evaluate_expression(match_command_ad, "arguments.include_brokerinfo");
} catch (utilities::InvalidValue&) {
  return false;
}

}}}} // glite::wms::manager::common

// File: CommandAdManipulation.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "CommandAdManipulation.h"
#include  "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include <algorithm>
#include <cctype>
#include <boost/scoped_ptr.hpp>
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
  "      && isString(other.arguments.ad.CertSubject)"
  "      && isString(other.arguments.file)"
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

classad::ClassAd*
submit_command_create(classad::ClassAd* job_ad)
{
  classad::ClassAd* result = 0;

  std::string jobid = requestad::get_edg_jobid(*job_ad);
  if (!jobid.empty()) {
    result = new classad::ClassAd;
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobsubmit"));
    classad::ClassAd* args = new classad::ClassAd;
    args->Insert("ad", job_ad);
    result->Insert("arguments", args);
  }

  return result;
}



classad::ClassAd const*
submit_command_get_ad(classad::ClassAd const& submit_command_ad)
{
  return utilities::evaluate_expression(submit_command_ad, "arguments.ad");
}

classad::ClassAd*
resubmit_command_create(std::string const& job_id, std::string const& sequence_code)
{
  classad::ClassAd* result = 0;

  if (!job_id.empty()) {
    result = new classad::ClassAd;
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobresubmit"));
    classad::ClassAd* args = new classad::ClassAd;
    args->InsertAttr("id", job_id);
    args->InsertAttr("lb_sequence_code", sequence_code);
    result->InsertAttr("arguments", args);
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

classad::ClassAd*
cancel_command_create(std::string const& job_id)
{
  classad::ClassAd* result = 0;

  if (!job_id.empty()) {
    result = new classad::ClassAd;
    result->InsertAttr("version", std::string("1.0.0"));
    result->InsertAttr("command", std::string("jobcancel"));
    classad::ClassAd* args = new classad::ClassAd;
    args->InsertAttr("id", job_id);
    result->InsertAttr("arguments", args);
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

classad::ClassAd const*
match_command_get_ad(classad::ClassAd const& match_command_ad)
{
  return utilities::evaluate_expression(match_command_ad, "arguments.ad");
}

std::string
match_command_get_file(classad::ClassAd const& match_command_ad)
{
  return utilities::evaluate_expression(match_command_ad, "arguments.file");
}

}}}} // glite::wms::manager::common


// File: CommandAdManipulation.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_COMMON_COMMANDADMANIPULATION_H
#define GLITE_WMS_MANAGER_COMMON_COMMANDADMANIPULATION_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace manager {
namespace common {

bool
command_is_valid(classad::ClassAd const& command_ad);

std::string
command_get_command(classad::ClassAd const& command_ad);

classad::ClassAd*
submit_command_create(classad::ClassAd* job_ad);

classad::ClassAd const*
submit_command_get_ad(classad::ClassAd const& submit_command_ad);

classad::ClassAd*
submit_command_remove_ad(classad::ClassAd& submit_command_ad);

classad::ClassAd*
cancel_command_create(std::string const& job_id);

std::string
cancel_command_get_id(classad::ClassAd const& cancel_command_ad);

std::string
cancel_command_get_lb_sequence_code(classad::ClassAd const& command_ad);

classad::ClassAd*
resubmit_command_create(std::string const& job_id, std::string const& sequence_code);

std::string
resubmit_command_get_id(classad::ClassAd const& command_ad);

std::string
resubmit_command_get_lb_sequence_code(classad::ClassAd const& command_ad);

classad::ClassAd*
match_command_create(
  classad::ClassAd* job_ad,
  std::string const& file,
  int number_of_result = -1,    // unlimited
  bool include_brokerinfo = false
);

classad::ClassAd const*
match_command_get_ad(classad::ClassAd const& match_command_ad);

classad::ClassAd*
match_command_remove_ad(classad::ClassAd& match_command_ad);

std::string
match_command_get_file(classad::ClassAd const& match_command_ad);

int match_command_get_number_of_results(classad::ClassAd const& match_command_ad);

bool match_command_get_include_brokerinfo(classad::ClassAd const& match_command_ad);

}}}} // glite::wms::manager::common

#endif

// Local Variables:
// mode: c++
// End:

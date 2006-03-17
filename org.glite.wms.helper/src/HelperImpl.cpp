// File: HelperImpl.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "glite/wms/helper/HelperImpl.h"
#include <fstream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>
#include "glite/wmsutils/classads/classad_utils.h"

namespace utils = glite::wmsutils::classads;

namespace glite {
namespace wms {
namespace helper {

HelperImpl::HelperImpl()
{
}

HelperImpl::~HelperImpl()
{
}

std::string
HelperImpl::resolve(std::string const& input_file) const
{
  assert(!input_file.empty());

  std::string result = input_file + output_file_suffix(); // output file

  std::ifstream fin(input_file.c_str());
  assert(fin);

  std::ofstream fout(result.c_str());
  assert(fout);

  boost::scoped_ptr<classad::ClassAd> ad(utils::parse_classad(fin));
  boost::scoped_ptr<classad::ClassAd> resolved_ad(resolve(&*ad));

  if (resolved_ad.get() == 0) {
    throw std::logic_error(id() + " - cannot resolve");
  }

  fout << utils::unparse_classad(*resolved_ad) << '\n';

  return result;
}

}}} // glite::wms:helper

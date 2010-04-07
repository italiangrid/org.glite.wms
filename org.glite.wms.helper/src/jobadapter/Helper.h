/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://project.eu-egee.org/partners for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// File: Helper.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.

// $Id$

#ifndef GLITE_WMS_HELPER_JOBADAPTER_HELPER_H
#define GLITE_WMS_HELPER_JOBADAPTER_HELPER_H

#include <boost/shared_ptr.hpp>
#include "glite/wms/helper/HelperImpl.h"

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

class Helper: public helper::HelperImpl
{
public:
  std::string id() const;
  std::string output_file_suffix() const;
  classad::ClassAd* resolve(
    classad::ClassAd const* input_ad,
    boost::shared_ptr<std::string> jw_template
  ) const;
};

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite 

#endif

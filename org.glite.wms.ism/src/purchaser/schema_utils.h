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

// File: schema_utils.h
// Author: Salvatore Monforte
// $Id$

#ifndef GLITE_WMS_II_PURCHASER_SCHEMA_UTILS_H
#define GLITE_WMS_II_PURCHASER_SCHEMA_UTILS_H

#include <string>
#include <utility>
#include <vector>

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class bdii_schema_info_type
{
  bdii_schema_info_type();
  bdii_schema_info_type(const bdii_schema_info_type&);
public:
  std::pair<
    std::vector<std::string>::const_iterator, 
    std::vector<std::string>::const_iterator
   > multi_valued(void);

   std::pair<
    std::vector<std::string>::const_iterator,
    std::vector<std::string>::const_iterator
   > number_valued(void);

  friend bdii_schema_info_type& bdii_schema_info();
};

bdii_schema_info_type&
bdii_schema_info();

}}}}

#endif



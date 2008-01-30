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



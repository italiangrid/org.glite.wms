// File: ii_attr_utils.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html
// $Id$

#ifndef _GLITE_WMS_COMMON_UTILITIES_II_ATTR_UTILS_
#define _GLITE_WMS_COMMON_UTILITIES_II_ATTR_UTILS_

#include <string>
#include <vector>

#include <boost/utility.hpp>

namespace glite {
namespace wms {
namespace common {
namespace utilities {
namespace ii_attributes {

typedef std::string type;	
typedef std::vector<type> container_type;
typedef container_type::const_iterator const_iterator;
typedef const_iterator iterator;

extern std::pair<const_iterator, const_iterator> multiValued();
extern bool isGlueSchema();

} // namespace ii_attributes
} // namespace utilities
} // namespace common
} // namespace wms
} // namespace glite

#endif



// File: ism-file-purchaser.h
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_ISM_PURCHASER_ISM_FILE_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_FILE_PURCHASER_H

#include <string>
#include <vector>
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"
#include "glite/wms/ism/purchaser/ism-ii-purchaser.h"
#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class ism_file_purchaser : public ism_purchaser
{
public:
                
  ism_file_purchaser(
    std::string const& file,
    exec_mode_t mode = once,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

  void operator()();

private:                
  std::string              m_filename;
};

namespace file {
// the types of the class factories
typedef ism_file_purchaser* create_t(
    std::string const& file,
    exec_mode_t mode = once,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
);

typedef void destroy_t(ism_file_purchaser*);

// the type of the set entry update factory function
typedef void set_purchaser_entry_update_fns_t(
  ii::create_entry_update_fn_t*,
  cemon::create_entry_update_fn_t*,
  rgma::create_entry_update_fn_t*
);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif

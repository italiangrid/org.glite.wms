// File: FLExtractor.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_COMMON_UTILITIES_FLEXTRACTOR_H
#define EDG_WORKLOAD_COMMON_UTILITIES_FLEXTRACTOR_H

#ifndef EDG_WORKLOAD_X_STRING
#define EDG_WORKLOAD_X_STRING
#include <string>
#endif
#ifndef EDG_WORKLOAD_X_UTILITY
#define EDG_WORKLOAD_X_UTILITY
#include <utility>
#endif
#ifndef EDG_WORKLOAD_X_VECTOR
#define EDG_WORKLOAD_X_VECTOR
#include <vector>
#endif
#ifndef EDG_WORKLOAD_COMMON_UTILITIES_FILELIST_H
#include "FileList.h"
#endif
#ifndef EDG_WORKLOAD_COMMON_UTILITIES_FILELISTLOCK_H
#include "FileListLock.h"
#endif
#ifndef EDG_WORKLOAD_COMMON_UTILITIES_EXTRACTOR_H
#include "Extractor.h"
#endif

namespace edg {
namespace workload {
namespace common {
namespace utilities {

template<typename T>
class FLExtractor
{
  typedef typename edg::workload::common::utilities::FileList<T> FL;
  typedef edg::workload::common::utilities::FileListMutex FLM;
  typedef edg::workload::common::utilities::ForwardExtractor<FL> FE;

  FL m_filelist;
  FLM m_mutex;
  FE m_extractor;

public:
  typedef typename FE::iterator iterator;
  typedef typename FE::value_type value_type;

public:
  FLExtractor(const std::string& file);
  ~FLExtractor();

  std::pair<iterator, bool> try_get_one();
  std::vector<iterator> get_all_available();
  void erase(const iterator& it);

};

} // namespace utilities
} // namespace common
} // namespace workload
} // namespace edg

#include "edg/workload/common/utilities/FLExtractor.cpp"

#endif

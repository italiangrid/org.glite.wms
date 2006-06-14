// File: DispatcherFromFileList.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H
#define GLITE_WMS_MANAGER_SERVER_DISPATCHERFROMFILELIST_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include "pipedefs.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

class Submit { };
class Cancel { };
class Match { };

typedef boost::variant<Submit, Cancel, Match> JobRequest;
typedef boost::shared_ptr<JobRequest> JobRequestPtr;

class Request_processor: public boost::static_visitor<>
{
public:
  void operator()(Submit const& submit) const;
  void operator()(Cancel const& cancel) const;
  void operator()(Match const& match) const;
  Request_processor();
  Request_processor(JobRequestPtr const&);

  ~Request_processor();
private:
  JobRequestPtr m_request;
};

class DispatcherFromFileList:
    public glite::wms::common::task::PipeWriter<RequestPtr, queue_type>
{
  class Impl;
  boost::shared_ptr<Impl> m_impl;

public:
  DispatcherFromFileList(std::string const& input_file, TaskQueue& tq);
  void operator()();
};

}}}} // glite::wms::manager::server

#endif

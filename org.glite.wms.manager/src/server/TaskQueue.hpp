#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <map>
#include <boost/shared_ptr.hpp>
#include "Request.hpp"

namespace glite {
namespace wms {
namespace manager {
namespace server {

typedef boost::shared_ptr<Request> RequestPtr;
typedef boost::shared_ptr<Request const> RequestConstPtr;

// should be JobId, but JobId is not safe
typedef std::map<std::string, RequestPtr> TaskQueue;

TaskQueue& the_task_queue();

glite::wms::manager::common::ContextPtr get_context(glite::wmsutils::jobid::JobId const& id);

}}}}

#endif

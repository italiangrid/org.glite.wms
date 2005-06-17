#ifndef GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H
#define GLITE_WMS_MANAGER_SERVER_PIPEDEFS_H

#include <queue>
#include "glite/wms/common/task/Task.h"
#include "TaskQueue.hpp"

namespace glite {
namespace wms {
namespace manager {
namespace server {

inline bool operator<(RequestPtr const& rhs, RequestPtr const& lhs)
{
  if (!rhs->marked_match() && lhs->marked_match()) {
    return true;
  } else {
    return rhs->last_processed() < lhs->last_processed();
  }
}

template<typename Q> class queue_adaptor;

template<typename T>
class queue_adaptor<std::priority_queue<T> >
{
  std::priority_queue<T> m_q;
public:
  queue_adaptor() {}
  T front() const
  {
    return m_q.top();
  }
  void push(T const& t)
  {
    m_q.push(t);
  }
  bool empty() const
  {
    return m_q.empty();
  }
  typename std::priority_queue<T>::size_type size() const
  {
    return m_q.size();
  }
  void pop()
  {
    m_q.pop();
  }
};

template<typename T>
struct adapted_priority_queue
{
  typedef queue_adaptor<std::priority_queue<T> > type;
};

typedef RequestPtr pipe_value;
typedef adapted_priority_queue<RequestPtr>::type queue_type;
typedef glite::wms::common::task::Pipe<RequestPtr, queue_type> pipe_type;

}}}}

#endif

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <ctime>
#include <classad_distribution.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include "../common/lb_utils.h"
#include <string>
#include <vector>
#include "glite/wmsutils/jobid/JobId.h"
#include <boost/utility.hpp>

typedef boost::shared_ptr<classad::ClassAd> ClassAdPtr;

namespace glite {
namespace wms {
namespace manager {
namespace server {

class InvalidRequest
{
  std::string m_str;
public:
  InvalidRequest(std::string const& str)
    : m_str(str)
  {
  }
  std::string str() const { return m_str; }
};

class Request: boost::noncopyable
{
public:
  enum State {
    WAITING       = 0x01,
    READY         = 0x02,
    PROCESSING    = 0x04,
    RECOVERABLE   = 0x08,
    UNRECOVERABLE = 0x10,
    DELIVERED     = 0x20,
    CANCELLED     = 0x40
  };

public:
  Request(
    classad::ClassAd const& command_ad,
    boost::function<void()> const& cleanup,
    glite::wmsutils::jobid::JobId const& id
  );
  ~Request();
  glite::wmsutils::jobid::JobId id() const { return m_id; }
  void state(State s, std::string const& message = std::string())
  {
    m_state = s;
    m_message = message;
  }
  State state() const { return m_state; }
  std::string message() const { return m_message; }
  void last_processed(time_t t) { m_last_processed = t; }
  std::time_t last_processed() const { return m_last_processed; }
  glite::wms::manager::common::ContextPtr lb_context() const { return m_lb_context; }

  classad::ClassAd const* jdl() const { return m_jdl.get(); }
  void jdl(classad::ClassAd* jdl);
  void clear_jdl() { m_jdl.reset(); }

  void mark_cancelled() { m_cancelled = true; }
  bool marked_cancelled() const { return m_cancelled; }

  void mark_resubmitted() { m_resubmitted = true; }
  bool marked_resubmitted() const { return m_resubmitted; }

  bool marked_match() const { return !m_match_parameters.get<0>().empty(); }
  boost::tuple<std::string, int, bool> match_parameters() const
  {
    return m_match_parameters;
  }

  void add_cleanup(boost::function<void()> const& cleanup)
  {
    m_input_cleaners.push_back(cleanup);
  }

  std::time_t expiry_time() const
  {
    return m_expiry_time;
  }

private:
  boost::scoped_ptr<classad::ClassAd> m_jdl;
  glite::wmsutils::jobid::JobId m_id;
  typedef std::vector<boost::function<void()> > input_cleaners_type;
  input_cleaners_type m_input_cleaners;
  State m_state;
  std::string m_message;
  std::time_t m_last_processed;
  glite::wms::manager::common::ContextPtr m_lb_context;
  bool m_cancelled;
  bool m_resubmitted;

  //match options: file name, number of results and include or not brokerinfo
  boost::tuple<std::string, int, bool> m_match_parameters;
  std::time_t m_expiry_time;

};

}}}}

#endif

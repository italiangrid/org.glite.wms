#ifndef GLITE_GPBOX_PEP_REQUEST_H
#define GLITE_GPBOX_PEP_REQUEST_H

#include "attribute.h"

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

namespace glite {
namespace gpbox {
namespace pep {

typedef std::vector<std::string> Resources;
typedef std::string Action;
typedef std::string Subject;

class Request
{
public:
  enum Scope {
    SUBJECT,
    RESOURCE,
    ACTION
  };
  
  Request(
    Resources const& resources,
    Action const& action,
    Subject const& subject
  );
  Request& attributes(Attributes const& attributes, Scope scope);
  Resources const& resources() const;
  Subject const& subject() const;
  Action const& action() const;
  Attributes const& attributes(Scope scope) const;
  
private:
  class Impl;
  boost::shared_ptr<Impl> m_impl;
};

}}}

#endif

#include "pep_request.h"

namespace glite {
namespace gpbox {
namespace pep {

struct Request::Impl
{
  Resources resources;
  Action action;
  Subject subject;
  Attributes attributes[3];
};

Request::Request(
  Resources const& resources,
  Action const& action,
  Subject const& subject
)
  : m_impl(new Impl)
{
  m_impl->resources = resources;
  m_impl->action = action;
  m_impl->subject = subject;
}

Request&
Request::attributes(Attributes const& attributes, Scope scope)
{
  m_impl->attributes[scope] = attributes;
  return *this;
}

Resources const&
Request::resources() const
{
  return m_impl->resources;
}

Subject const&
Request::subject() const
{
  return m_impl->subject;
}

Action const&
Request::action() const
{
  return m_impl->action;
}

Attributes const&
Request::attributes(Scope scope) const
{
  return m_impl->attributes[scope];
}

}}}

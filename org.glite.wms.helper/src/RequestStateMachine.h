// File: RequestStateMachine.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_HELPER_REQUESTSTATEMACHINE_H
#define GLITE_WMS_HELPER_REQUESTSTATEMACHINE_H

#include <map>
#include <set>
#include <string>

#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace helper {

class NoValidState: public std::exception
{
  char const* what() const throw()
  {
    return "No valid state transition for the job";
  }
};

class RequestStateMachine: boost::noncopyable
{
public:
  typedef std::string state_type;
  typedef std::string expression_type;
  typedef std::map<state_type, expression_type> initial_states_type;
  typedef std::map<state_type, boost::tuple<std::string, state_type> > transition_table_type;
  typedef std::set<state_type> final_states_type;

  struct AdNotValid {};

private:
  initial_states_type m_initial_states;
  transition_table_type m_transition_table;
  final_states_type m_final_states;
  state_type m_current_state;

public:
  // throws ADNotValid if state_machine_ad does not represent a valid state machine
  RequestStateMachine(classad::ClassAd const* state_machine_ad);
  ~RequestStateMachine();

  void start(classad::ClassAd const* ad);
  classad::ClassAd* next_step(classad::ClassAd const* ad);
  operator void*() const;
  int operator!() const;

};

}}} // glite::wms::helper

#endif

// Local Variables:
// mode: c++
// End:

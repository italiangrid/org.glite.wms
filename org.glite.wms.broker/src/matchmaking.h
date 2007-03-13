// $Id$

#ifndef GLITE_WMS_MATCHMAKING_MATCHMAKER_H
#define GLITE_WMS_MATCHMAKING_MATCHMAKER_H

#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <deque>
#include <set>
#include <vector>

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

typedef boost::tuple<
  std::string,                        // ceid
  double,                             // the rank information 
  boost::shared_ptr<classad::ClassAd> // the ce classad representation
> matchinfo;

enum { Id, Rank, Ad }; 

//typedef std::deque<matchinfo> matchtable;
typedef std::vector<matchinfo> matchtable;

void
match(
  classad::ClassAd& requestAd,
  matchtable& suitableCEs
);

void
match(
  classad::ClassAd& requestAd,
  std::set<std::string> const& CEids,
  matchtable& suitableCEs
);

struct rank_less_than_comparator :
  std::binary_function< matchinfo&, matchinfo&, bool >
{
  bool operator()(
    const matchinfo& a,
    const matchinfo& b)
  {
    return boost::tuples::get<Rank>(a) <
      boost::tuples::get<Rank>(b);
  }
};

struct rank_greater_than_comparator :
  std::binary_function< matchinfo&, matchinfo&, bool >
{
  bool operator()(
    const matchinfo& a,
    const matchinfo& b)
  {
    return boost::tuples::get<Rank>(a) > 
      boost::tuples::get<Rank>(b);
  }
};

}}}

#endif

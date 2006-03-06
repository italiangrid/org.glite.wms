// File: MatchMaker.h
// Author: Monforte Salvatore
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MATCHMAKING_MATCHMAKER_H
#define GLITE_WMS_MATCHMAKING_MATCHMAKER_H

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <map>
#include <functional>

namespace classad
{
  class ClassAd;
}

namespace glite {
namespace wms {
namespace matchmaking {

class match_info
{
public:
  match_info()
    : m_rank(0), m_undefined(true)
  {
  }
  match_info(boost::shared_ptr<classad::ClassAd>& a)
    : m_rank(0), m_CEAd(a), m_undefined(true)
  {
  }
  ~match_info() {}
  // Default copy ctor and operator= are ok
  bool isRankUndefined() const { return m_undefined; }
  double getRank() const { return m_rank; }
  void setRank(double r) { m_rank = r; m_undefined = false; }  
  const classad::ClassAd* getAd() const { return m_CEAd.get(); }
  
private:
  double m_rank;
  boost::shared_ptr<classad::ClassAd> m_CEAd; 
  bool m_undefined;
};

/** Type definition for match_table. */
typedef std::map<std::string, match_info> match_table_t;
typedef match_table_t::const_iterator match_const_iterator;
typedef match_const_iterator match_iterator;

/**
 * Returns CEid attribute of the pointed element a match_table const iterator.
 * @param it the costant iterator.
 * @return the pointed element CEid.
 */
inline std::string CEid(match_iterator& it) { return it -> first; }

class MatchMakerImpl : boost::noncopyable
{
 public:
  MatchMakerImpl() {}
  // TODO: no op if requestAd=0
  virtual void prefetchCEInfo(
    const classad::ClassAd* requestAd,
    match_table_t& suitableCEs
  ) = 0;
  virtual void checkRequirement(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ce_info
  ) = 0;
  virtual void checkRank(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ce_info
  ) = 0;
  virtual ~MatchMakerImpl() {}
}; 

template<class implementation>
class MatchMaker : boost::noncopyable
{ 
 public:
  MatchMaker() : m_impl( new implementation ) 
  {
  } 
  ~MatchMaker() {}
  
  void prefetchCEInfo(
    const classad::ClassAd* requestAd,
    match_table_t& suitableCEs
  )
  {
    m_impl->prefetchCEInfo( requestAd, suitableCEs);
  }
  void checkRequirement(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ce_info = false
  )
  {
    m_impl->checkRequirement(requestAd, suitableCEs, use_prefetched_ce_info);
  }
  void checkRank(
    classad::ClassAd& requestAd,
    match_table_t& suitableCEs,
    bool use_prefetched_ce_info = false
  )
  {
    m_impl->checkRank(requestAd, suitableCEs, use_prefetched_ce_info);
  }
  
private:
  boost::scoped_ptr<MatchMakerImpl>      m_impl;
};

}}}

#endif

// File: match.h
// Author: Francesco Giacomini
// Author: Salvatore Monforte

// $Id$

#ifndef GLITE_WMS_BROKER_MATCH_H
#define GLITE_WMS_BROKER_MATCH_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace broker {

struct MatchInfo
{
  std::string ce_id;
  double rank;
  boost::shared_ptr<classad::ClassAd> ce_ad;
  MatchInfo(
    std::string const& id,
    double r, 
    boost::shared_ptr<classad::ClassAd> ad
  ) : ce_id(id), rank(r), ce_ad(ad) {}   
};

typedef std::vector<MatchInfo> MatchTable;

typedef std::map<std::string, std::vector<std::string> > FileMapping;

struct StorageInfo
{
  typedef std::vector<std::pair<std::string, int> > Protocols;
  typedef std::vector<FileMapping::const_iterator> Links;
  typedef std::vector<std::pair<std::string, std::string> > CE_Mounts;

  Protocols protocols;
  Links links;
  CE_Mounts ce_mounts;
};

typedef std::map<std::string, StorageInfo> StorageMapping;

struct DataInfo
{
  FileMapping fm;
  StorageMapping sm;
};

void
match(
  classad::ClassAd& ad,
  MatchTable& matches,
  std::vector<std::string> const& skipping_ces = std::vector<std::string>()
);

void
match(
  classad::ClassAd& ad,
  MatchTable& matches,
  std::set<std::string> const& candidate_ces,
  std::vector<std::string> const& skipping_ces = std::vector<std::string>()
);

void
match(
  classad::ClassAd& ad,
  MatchTable& matches,
  DataInfo& data_info,
  std::vector<std::string> const& skipping_ces = std::vector<std::string>()
);

struct MaxRankSelector
{
  MatchTable::iterator operator()(MatchTable&) const;
};

class FuzzySelector
{
  double const m_factor;

public:
  explicit FuzzySelector(double factor = 1.0);

  MatchTable::iterator operator()(MatchTable&) const;
};

}}}

#endif

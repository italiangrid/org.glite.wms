// File: storage_utils.cpp
// Author: Salvatore Monforte, INFN
// $Id$
#include <numeric>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/regex.hpp>

#include <classad_distribution.h>

#include "glite/wms/common/logger/logger_utils.h"
#include "brokerinfo.h"
#include "glite/wms/ism/ism.h"
#include "storage_utils.h"

using namespace std;

namespace glite {
namespace wms {
namespace logger        = common::logger;
namespace broker {

namespace {

bool evaluate(
  classad::ClassAd const& ad, 
  std::string const& name, 
  vector<classad::ExprTree*>& v
) {
  bool result = false;
  classad::Value value;
  const classad::ExprList *l;
  if (ad.EvaluateAttr(name, value) &&
    value.IsListValue(l)) {
      l->GetComponents(v);
      result = true;
  }
  return result;
}

string
resolve_storage_name(string const& sfn)
{
  static boost::regex e("^\\s*([^:]*):[\\s/]*([^\\s:/]+)(:[0-9]+)?/.*");
  boost::smatch p;
  string name;

  if(boost::regex_match(sfn, p, e)) {
    name.assign(p[2].first, p[2].second);
  }
  else if (int p=sfn.find("://",0) != string::npos) {
      name.assign(sfn.substr(p+3));
  }
  return name;
}

std::pair<std::string, int>
make_storage_info(classad::ClassAd const&ad)
{
  std::string protocol;
  int port;
  ad.EvaluateAttrString("GlueSEAccessProtocolType", protocol);
  ad.EvaluateAttrInt("GlueSEAccessProtocolPort", port);
  return std::make_pair(protocol, port);
}

std::pair<std::string, std::string>
make_ce_bind_info(classad::ClassAd const&ad)
{
  std::string id;
  std::string mount;
  ad.EvaluateAttrString("GlueCESEBindCEUniqueID", id);
  ad.EvaluateAttrString("GlueCESEBindCEAccesspoint", mount);
  return std::make_pair(
    id, mount
  );
}

struct matches_any_protocol_in
{
  matches_any_protocol_in(vector<pair<string,int> >const& protocols) :
    m_protocols(protocols) {}

  bool operator()(string const& p) {
    vector<pair<string,int> >::const_iterator it(
      m_protocols.begin()
    );
    vector<pair<string,int> >::const_iterator const e(
      m_protocols.end()
    );
    for( ; it!=e; ++it ) {
      if(it->first == p) return true;
    }
    return false;
  }
  vector<pair<string,int> > const& m_protocols;
};

bool is_mount_defined(pair<string,string>const& p)
{
    return !p.second.empty();
}

struct extract_mapped_file_names : binary_function<
  std::set<string>*,
  broker::storagemapping::const_iterator,
  std::set<string>*
>
{
  std::set<string>*
  operator()(
    std::set<string>* f,
    broker::storagemapping::const_iterator si
  )
  {
    vector<filemapping::const_iterator> const& info(
      boost::tuples::get<tag::filemapping_link>(si->second)
    );
    vector<filemapping::const_iterator>::const_iterator it(info.begin());
    vector<filemapping::const_iterator>::const_iterator const e(info.end());
    for( ; it != e ; ++it ) {
       f->insert((*it)->first);
    }
    return f;
  }
};

} // anonymous namespace
 
boost::shared_ptr<storagemapping>
resolve_storagemapping_info(
  boost::shared_ptr<filemapping> fm
)
{
  boost::shared_ptr<storagemapping> sm(new storagemapping);

  filemapping::const_iterator fi = fm->begin(); 
  filemapping::const_iterator fe = fm->end();

  ism::Ism const& the_ism = *ism::get_ism();
 for( ; fi != fe; ++fi ) {
    vector<string> const& sfns(
      fi->second
    );
    vector<string>::const_iterator sfni = sfns.begin();
    vector<string>::const_iterator const sfne = sfns.end();

    for( ; sfni != sfne; ++sfni ) {
       string name = resolve_storage_name(*sfni);
       if(name.empty()) continue;

       ism::Ism::MutexSliceContainer::const_iterator
       slice_it(the_ism.storage.begin());
       ism::Ism::MutexSliceContainer::const_iterator const
       last_slice(the_ism.storage.end());
       
       for( ; slice_it != last_slice; ++slice_it ) {

         ism::OrderedSliceIndex& ordered_index(
           (*slice_it)->slice->get<ism::SliceIndex::Ordered>()
         );
 
         ism::Mutex::scoped_lock l((*slice_it)->mutex);
         ism::OrderedSliceIndex::iterator const slice_end(ordered_index.end());
         ism::OrderedSliceIndex::iterator it = ordered_index.find(name);

         if ( it!=slice_end ) {
           boost::shared_ptr<classad::ClassAd> se_ad(
             boost::tuples::get<ism::Ad>(*it)
           );
           storagemapping::iterator i;
           bool ib;

           boost::tie(i,ib) = sm->insert(
             std::make_pair(
               name,   
               boost::tuples::make_tuple(
                 vector<pair<string, int > >(),
                 vector<filemapping::const_iterator>(),
                 vector<pair<string, string > >()
               )
             )
           );
           boost::tuples::get<tag::filemapping_link>(i->second).push_back(fi);
           {
             vector<classad::ExprTree*> ads;
             if (evaluate(*se_ad, "GlueSEAccessProtocol", ads)) {

               vector<classad::ExprTree*>::const_iterator 
               expr_it(ads.begin());
               vector<classad::ExprTree*>::const_iterator const 
               expr_e(ads.end());

               for (; expr_it != expr_e; ++expr_it) {
                 classad::ClassAd const& ad(
                   *static_cast<classad::ClassAd*>(*expr_it)
                 );
                 boost::tuples::get<tag::storage_info>(i->second).push_back(
                   make_storage_info(ad)
                 );
               }
             }
           }  
           {
             vector<classad::ExprTree*> ads;
             if (evaluate(*se_ad, "CloseComputingElements", ads)) {

               vector<classad::ExprTree*>::const_iterator 
               expr_it(ads.begin());
               vector<classad::ExprTree*>::const_iterator const 
               expr_e(ads.end());

               for (; expr_it != expr_e; ++expr_it) {
                 classad::ClassAd const& ad(
                   *static_cast<classad::ClassAd*>(*expr_it)
                 );
                 boost::tuples::get<tag::ce_bind_info>(i->second).push_back(
                   make_ce_bind_info(ad)
                 );
               }
             }
           }  
         break; // there is no need to continue the search in
                // the remaining slices
         }
       }
    }
  }
  return sm;
}

vector<storagemapping::const_iterator>
select_compatible_storage(
  storagemapping const& sm,
  vector<string>const& dap
)
{
  vector<storagemapping::const_iterator> result;
  storagemapping::const_iterator storage_it(sm.begin());
  storagemapping::const_iterator const storage_end(sm.end());

  for( ; storage_it != storage_end; ++storage_it ) {

    vector<pair<string,int> > const& supported_protocols(
      boost::tuples::get<tag::storage_info>(storage_it->second)
    );
    vector<string>::const_iterator it = find_if(
      dap.begin(), dap.end(),
      matches_any_protocol_in(supported_protocols)
    );
    if (it!=dap.end()) {
      vector<pair<string,string> > const& close_ce_binds(
        boost::tuples::get<tag::ce_bind_info>(storage_it->second)
      );
      if(*it=="file" &&
         find_if(
           close_ce_binds.begin(),
           close_ce_binds.end(),
           is_mount_defined
         ) == close_ce_binds.end()) {

          continue;
      }
      result.push_back(storage_it);
    }
  }
  return result;
}

size_t
count_unique_logical_files(
  vector<storagemapping::const_iterator>::const_iterator first,
  vector<storagemapping::const_iterator>::const_iterator last)
{
  std::set<string> unique_logical_file_names;
  std::accumulate(
    first,
    last,
    &unique_logical_file_names,
    extract_mapped_file_names()
  );
  return unique_logical_file_names.size();
}

std::set<std::string>*
extract_computing_elements_id::operator()(
  std::set<std::string>* s,
  storagemapping::const_iterator si
)
{
  std::vector<std::pair<std::string,std::string> > const& info(
    boost::tuples::get<tag::ce_bind_info>(si->second)
  );
  std::vector<std::pair<std::string,std::string> >::const_iterator it(info.begin());
  std::vector<std::pair<std::string,std::string> >::const_iterator const e(info.end());
  for( ; it != e ; ++it ) {
     
     s->insert(it->first);
  }
  return s;
}

is_storage_close_to::is_storage_close_to(std::string const& id) 
  : m_ceid(id) 
{
}

bool is_storage_close_to::operator()(storagemapping::const_iterator const& v)
{
 std::vector<std::pair<std::string,std::string> > const& i(
      boost::tuples::get<tag::ce_bind_info>(v->second)
 );
 std::vector<std::pair<std::string,std::string> >::const_iterator it(i.begin());
 std::vector<std::pair<std::string,std::string> >::const_iterator const e(i.end());
 for( ; it != e ; ++it) {
   if(boost::starts_with(m_ceid, it->first)) return true;
 }
 return false;
}

boost::shared_ptr<filemapping>
resolve_filemapping_info(
  const classad::ClassAd& requestAd
)
{
  return glite::wms::rls::resolve_filemapping_info(requestAd);
}

} // namespace broker
} // namespace wms
} // namespace glite

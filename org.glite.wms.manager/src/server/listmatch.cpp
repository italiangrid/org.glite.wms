/*
 * File: listmatch.cpp
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Author: Cinzia Di Giusto <Cinzia.DiGiusto@cnaf.infn.it>
 */

#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/exception.hpp>
#include "listmatch.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/broker/RBSimpleImpl.h"
#include "glite/wms/broker/RBMaximizeFilesImpl.h"
#include "glite/wms/broker/RBMinimizeAccessCostImpl.h"
#include "glite/wms/matchmaking/matchmaker.h"
#include "glite/wms/matchmaking/utility.h"
#include "glite/wms/matchmaking/exceptions.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

using namespace std;

namespace broker        = glite::wms::broker;
namespace brokerinfo    = glite::wms::brokerinfo;
namespace matchmaking   = glite::wms::matchmaking;
namespace configuration = glite::wms::common::configuration;
namespace utilities     = glite::wms::common::utilities;
namespace requestad     = glite::wms::jdl;
namespace jobid         = glite::wmsutils::jobid;

typedef brokerinfo::BrokerInfo<brokerinfo::brokerinfoGlueImpl> BrokerInfo;

namespace {
 
bool use_cached_info_for_gris()
{
  return configuration::Configuration::instance()->common()->use_cache_instead_of_gris();
}

   
classad::ClassAd listmatch(classad::ClassAd const& jdl)
{
  classad::ClassAd result;
  result.InsertAttr("reason", std::string("no matching resources found"));
  result.InsertAttr("match_result", new classad::ExprList);
    
  broker::ResourceBroker rb(new broker::RBSimpleImpl(use_cached_info_for_gris()));
  bool input_data_exists = false;
  std::vector<std::string> input_data;
  requestad::get_input_data(jdl, input_data, input_data_exists);

  BrokerInfo BI;
  
  if (input_data_exists) {
    // Here we have to check if the rank expression in the request
    // is rank = other.dataAccessCost and change the implementation
    // of the broker (RBMinimizeAccessCost)
    classad::ExprTree* rank_expr = jdl.Lookup("rank");
    if (rank_expr) {
      std::vector<std::string> rankAttributes;
      utilities::insertAttributeInVector(&rankAttributes, rank_expr, utilities::is_reference_to("other"));
      if (rankAttributes.size() == 1 
          && rankAttributes.front() == "DataAccessCost") {
      // RBMinimizeAccessCostImpl doesn't rank based on the
      // Info Service. So the use_cached_info_for_gris 
      // flag isn't needed.
        rb.changeImplementation(new broker::RBMinimizeAccessCostImpl(&BI));
      } else {
        rb.changeImplementation(new broker::RBMaximizeFilesImpl(&BI, use_cached_info_for_gris()));
      }
    }
  }

  boost::scoped_ptr<matchmaking::match_table_t> suitableCEs;
  
  std::string mm_error;
  
  try {
    suitableCEs.reset(rb.findSuitableCEs(&jdl));
    
    std::vector<classad::ExprTree*> hosts;
    if (!suitableCEs->empty() ) {
            
      matchmaking::match_vector_t suitableCEs_vector(suitableCEs->begin(), suitableCEs->end());
      
      std::stable_sort(suitableCEs_vector.begin(), suitableCEs_vector.end(), matchmaking::rank_greater_than_comparator());
      
      for (matchmaking::match_vector_t::const_iterator it = suitableCEs_vector.begin(); 
           it != suitableCEs_vector.end(); 
           ++it) {
      
        //  "host,rank"
        classad::Value value;
        value.SetStringValue(
          it->first + ',' + boost::lexical_cast<std::string>(it->second.getRank())
        );

       hosts.push_back(classad::Literal::MakeLiteral(value));
     
    /* classad::ClassAd* ceinfo = new classad::ClassAd;
      if (!ceinfo->InsertAttr("host", it->first)) return false;
      if (!ceinfo->InsertAttr("rank", it -> second.getRank())) return false;
      
      hosts.push_back(ceinfo->Copy());*/

      }
      
      result.InsertAttr("reason", std::string("ok"));    
      result.Insert("match_result", classad::ExprList::MakeExprList(hosts));
            
    }
    
  } catch (matchmaking::ISNoResultError const& e) {
    result.InsertAttr(
      "reason",
      "The user is not authorized on any resource currently registered in "
      + e.host()
    );

  } catch (matchmaking::InformationServiceError const& e) {
    result.InsertAttr("reason", std::string(e.what()));  

  } catch (matchmaking::RankingError const& e) {
    result.InsertAttr("reason", std::string(e.what()));  
 
  } catch (requestad::CannotGetAttribute const& e) {
    result.InsertAttr(
      "reason", 
      "Attribute " + e.parameter() + " does not exist or has the wrong type"
    ); 

  } catch (requestad::CannotSetAttribute const& e) {
    result.InsertAttr("reason", "Cannot set attribute " + e.parameter());

  } catch (jobid::JobIdException const& e) {
    result.InsertAttr("reason", std::string(e.what()));  

  } catch (boost::filesystem::filesystem_error const& e) {
    result.InsertAttr("reason", std::string(e.what()));  
  }
 
  return result;
}
    
}

namespace glite {
namespace wms {
namespace manager {
namespace server {

bool match(classad::ClassAd const& jdl, std::string const& result_file)
{
  classad::ClassAd result(listmatch(jdl));
  
  if (result_file.empty() || result_file[0] != '/') {
    return false;
  }
  
  std::ofstream out(result_file.c_str());
  
  out << result << '\n';
      
  return true;
}

} // namespace server
} // namespace manager
} // namespace wms
} // namespace glite

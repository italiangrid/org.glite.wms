// File: Helper.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <fstream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/regex.hpp>

#include <classad_distribution.h>

#include "Helper_matcher.h"

#include "glite/wms/helper/broker/exceptions.h"
#include "glite/wms/helper/exceptions.h"
#include "glite/wms/helper/HelperFactory.h"

#include "glite/wms/brokerinfo/brokerinfo.h"

#ifdef MATCHER_HELPER_USE_ISM
#include "glite/wms/brokerinfo/brokerinfoISMImpl.h"
#include "glite/wms/broker/RBSimpleISMImpl.h"
#include "glite/wms/broker/RBMaximizeFilesISMImpl.h"
#else
#include "glite/wms/brokerinfo/brokerinfoGlueImpl.h"
#include "glite/wms/broker/RBSimpleImpl.h"
#include "glite/wms/broker/RBMaximizeFilesImpl.h"
#include "glite/wms/broker/RBMinimizeAccessCostImpl.h"
#endif

#include "glite/wms/matchmaking/exceptions.h"
#include "glite/wms/matchmaking/utility.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wmsutils/classads/classad_utils.h"

#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/JobAdManipulation.h"
#include "glite/jdl/PrivateAdManipulation.h"
#include "glite/jdl/PrivateAttributes.h"
#include "glite/jdl/ManipulationExceptions.h"

#ifndef GLITE_WMS_DONT_HAVE_GPBOX
#include "gpbox_utils.h"
#endif

namespace fs            = boost::filesystem;
namespace jobid         = glite::wmsutils::jobid;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace requestad     = glite::jdl;
namespace utils		= glite::wmsutils::classads;
namespace matchmaking   = glite::wms::matchmaking;

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

namespace glite {
namespace wms {
namespace helper {
namespace matcher {

namespace {

#ifdef MATCHER_HELPER_USE_ISM
typedef glite::wms::brokerinfo::BrokerInfo<glite::wms::brokerinfo::brokerinfoISMImpl> BrokerInfo;
#else
typedef glite::wms::brokerinfo::BrokerInfo<glite::wms::brokerinfo::brokerinfoGlueImpl> BrokerInfo;
#endif
std::string const helper_id("MatcherHelper");

helper::HelperImpl* create_helper()
{
  return new Helper;
}

struct Register
{
  Register()
  {
    helper::HelperFactory::instance()->register_helper(helper_id, create_helper);
  }
  ~Register()
  {
    helper::HelperFactory::instance()->unregister_helper(helper_id);
  }
};

Register const r;

std::string const f_output_file_suffix(".rbh");

/* Answer format:
 * 1) if no error occurs
 * [
 *   reason =ok;
 *   match_result={
 *                  [
 *                    ce_id=...;
 *                    rank=...;
 *                    brokerinfo=...;
 *                  ],
 *                  [...],
 *                  ...
 *                }
 * ]
 * 2) if error occurs
 * [
 *   reason = error reason;
 *   match_result={}
 * ]
 */

std::auto_ptr<classad::ClassAd>
f_resolve_do_match(classad::ClassAd const& input_ad)
{
  std::auto_ptr<classad::ClassAd> result;
  result.reset(new classad::ClassAd);

  result->InsertAttr("reason", std::string("no matching resources found"));
  result->Insert("match_result", new classad::ExprList);
    
#ifdef MATCHER_HELPER_USE_ISM
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleISMImpl());
#elif MATCHER_HELPER_PREFETCH_INFO_FROM_II
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleImpl(true));
#else
  glite::wms::broker::ResourceBroker rb(new glite::wms::broker::RBSimpleImpl());
#endif

  bool input_data_exists = false;
  std::vector<std::string> input_data;
  requestad::get_input_data(input_ad, input_data, input_data_exists);

  boost::scoped_ptr<BrokerInfo> BI(new BrokerInfo);
  
  if (input_data_exists) {
    // Here we have to check if the rank expression in the request
    // is rank = other.dataAccessCost and change the implementation
    // of the broker (RBMinimizeAccessCost)
    classad::ExprTree* rank_expr = input_ad.Lookup("rank");
    if (rank_expr) {
      std::vector<std::string> rankAttributes;
      utils::insertAttributeInVector(&rankAttributes, rank_expr, utils::is_reference_to("other"));
      if (rankAttributes.size() == 1 
          && rankAttributes.front() == "DataAccessCost") {
      // RBMinimizeAccessCostImpl doesn't rank based on the
      // Info Service. So the use_cached_info_for_gris 
      // flag isn't needed.
#ifndef MATCHER_HELPER_USE_ISM
        rb.changeImplementation(new glite::wms::broker::RBMinimizeAccessCostImpl(BI.get()));
#endif
      } else {
#ifdef MATCHER_HELPER_USE_ISM
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesISMImpl(BI.get(),true));
#elif MATCHER_HELPER_PREFETCH_INFO_FROM_II
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesImpl(BI.get(),true));
#else
        rb.changeImplementation(new glite::wms::broker::RBMaximizeFilesImpl(BI.get()));
#endif
      }
    }
  }

  boost::scoped_ptr<matchmaking::match_table_t> suitableCEs;
  std::string mm_error;
  
  try {
    suitableCEs.reset(rb.findSuitableCEs(&input_ad));

    std::string vo(requestad::get_virtual_organisation(input_ad));    
    std::vector<classad::ExprTree*> hosts;
    bool include_brokerinfo = false;
    input_ad.EvaluateAttrBool("include_brokerinfo", include_brokerinfo);
    int number_of_results = -1;
    input_ad.EvaluateAttrInt("number_of_results", number_of_results);

#ifndef GLITE_WMS_DONT_HAVE_GPBOX
    std::string x509_user_proxy_file_name(requestad::get_x509_user_proxy(input_ad)); 
    if (!suitableCEs->empty()) { 
      configuration::Configuration const* const config(
        configuration::Configuration::instance()
      );
      assert(config);
 
      std::string PBOX_host_name(config->wm()->pbox_host_name());
      if (!PBOX_host_name.empty())
      {
        if (!glite::wms::helper::broker::gpbox::interact(
          *config,
          x509_user_proxy_file_name,
          PBOX_host_name,
          *suitableCEs
        ))
          Info("Error during gpbox interaction");
      }
    }
#endif

    if (!suitableCEs->empty() ) {
      matchmaking::match_vector_t suitableCEs_vector(
        suitableCEs->begin(),
        suitableCEs->end()
      );

      std::stable_sort(
        suitableCEs_vector.begin(),
        suitableCEs_vector.end(),
        matchmaking::rank_greater_than_comparator()
      );

      matchmaking::match_vector_t::const_iterator it(
        suitableCEs_vector.begin()
      );
      matchmaking::match_vector_t::const_iterator const end(
        suitableCEs_vector.end()
      );
      for (int i = 0;
           it != end && (number_of_results == -1 || i < number_of_results);
           ++it, ++i) {

        std::auto_ptr<classad::ClassAd> ceinfo(new classad::ClassAd);
        string const ce_id(
          utils::evaluate_attribute(*it->second.getAd(), "GlueCEUniqueID")
        );
        ceinfo->InsertAttr("ce_id", ce_id);
        ceinfo->InsertAttr("rank", it->second.getRank());
        
        if (include_brokerinfo) {
           std::string const ce_id(
             utils::evaluate_attribute(*it->second.getAd(),"GlueCEUniqueID")
           );
           BI->retrieveCloseSEsInfo(*it->second.getAd());
           BI->retrieveCloseSAsInfo(vo); // Retrieve only GlueSAAvailableVOSpace
           ceinfo->Insert("brokerinfo", BI->asClassAd());
        }
        hosts.push_back(ceinfo.get());
        ceinfo.release();
      }
      
      result->InsertAttr("reason", std::string("ok"));    
      result->Insert("match_result", classad::ExprList::MakeExprList(hosts));           
    }
#ifndef GLITE_WMS_DONT_HAVE_GPBOX
    else {
      Info("Empty CE list after G-Pbox screening");
    }
#endif
  } catch (matchmaking::ISNoResultError const& e) {
    result->InsertAttr(
      "reason",
      "The user is not authorized on any resource currently registered in "
      + e.host()
    );

  } catch (matchmaking::InformationServiceError const& e) {
    result->InsertAttr("reason", std::string(e.what()));  

  } catch (matchmaking::RankingError const& e) {
    result->InsertAttr("reason", std::string(e.what()));  
 
  } catch (requestad::CannotGetAttribute const& e) {
    result->InsertAttr(
      "reason", 
      "Attribute " + e.parameter() + " does not exist or has the wrong type"
    ); 

  } catch (requestad::CannotSetAttribute const& e) {
    result->InsertAttr("reason", "Cannot set attribute " + e.parameter());

  } catch (jobid::JobIdException const& e) {
    result->InsertAttr("reason", std::string(e.what()));  

  } catch (boost::filesystem::filesystem_error const& e) {
    result->InsertAttr("reason", std::string(e.what()));  
  }
  return result;
}

} // {anonymous}

std::string
Helper::id() const
{
  return helper_id;
}

std::string
Helper::output_file_suffix() const
{
  return f_output_file_suffix;
}

classad::ClassAd*
Helper::resolve(classad::ClassAd const* input_ad) const
{
  std::auto_ptr<classad::ClassAd> result = f_resolve_do_match(*input_ad);

  return result.release();
}

}}}} // glite::wms::helper::matcher

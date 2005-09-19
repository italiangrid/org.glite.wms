/*
 * File: listmatch.cpp
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Author: Cinzia Di Giusto <Cinzia.DiGiusto@cnaf.infn.it>
 * Author: Francesco Giacomini
 * Author: Francesco Prelz
 */

#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include "listmatch.h"
#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/helper/Helper.h"
#include "glite/wms/helper/exceptions.h"

namespace utilities = glite::wms::common::utilities;

namespace glite {
namespace wms {
namespace manager {
namespace server {

bool match(
  classad::ClassAd const& jdl,
  std::string const& result_file,
  int number_of_results,
  bool include_brokerinfo
)
{
  if (result_file.empty() || result_file[0] != '/') {
    return false;
  }

  std::auto_ptr<classad::ClassAd> result;

  try {
    result.reset(glite::wms::helper::Helper("MatcherHelper").resolve(&jdl));

    classad::ExprTree *et = result->Lookup("match_result");
    assert(utilities::is_expression_list(et));
    classad::ExprList& match_result = *static_cast<classad::ExprList*>(et);

    if (match_result.size() != 0) {
      std::vector<classad::ExprTree*> list;
      match_result.GetComponents(list);

      std::vector<classad::ExprTree*> newlist;
      if (number_of_results == -1) {
        number_of_results = match_result.size();
      }

      for (int i = 0;
           i < std::min(number_of_results, match_result.size());
           ++i) {
        assert(utilities::is_classad(list[i]));
        std::auto_ptr<classad::ExprTree> element(list[i]->Copy());

        if (!include_brokerinfo) {

          //translate the answer in this format
          /* [
           *   reason = ok;
           *   match_result = {"host1,rank1", "host2,rank2", ...}
           * ]
           */
          classad::ClassAd* ca = static_cast<classad::ClassAd*>(element.get());
          std::string host = utilities::evaluate_attribute(*ca, "ce_id");
          double rank = utilities::evaluate_attribute(*ca, "rank");

          classad::Value value;
          value.SetStringValue(
            host + ',' + boost::lexical_cast<std::string>(rank)
          );


          element.reset(classad::Literal::MakeLiteral(value));
        }
        newlist.push_back(element.release());
      }

      result->Insert("match_result", classad::ExprList::MakeExprList(newlist));
    }
  } catch (glite::wms::helper::HelperError const& e) {
    // We need to chime into the communication pipe in this case.
    result.reset(new classad::ClassAd);
    result->InsertAttr("reason", std::string("error accessing broker helper"));
    result->InsertAttr("match_result", new classad::ExprList);
  }

  classad::PrettyPrint result_unparser;
  std::string          result_string;
  result_unparser.Unparse(result_string, result.get());

  int const fd = open(result_file.c_str(), O_WRONLY | O_NONBLOCK);
  if (fd == -1) {
    return false;
  }

  int retry_count, backoff_time;
  int write_size = result_string.size();
  int written_size = 0;

  for (retry_count = 7; retry_count > 0; backoff_time *= 2, --retry_count) {
      int write_retcod
        = write(fd, result_string.c_str() + written_size, write_size);
      if (write_retcod < 0) {
        if (errno == EAGAIN) {
          sleep(backoff_time);
        } else { 
          break;
        }
      } else if (write_retcod < write_size) {
        written_size += write_retcod;
        write_size   -= write_retcod;
        backoff_time = 1;
        sleep(backoff_time);
      } else {
        break;
      }
  }

  close(fd);

  return retry_count > 0;
}

}}}} // glite::wms::manager::server

#ifndef GLITE_GPBOX_PEP_XACMLHELPER_H
#define GLITE_GPBOX_PEP_XACMLHELPER_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

namespace glite {
namespace gpbox {
namespace pep {

class Request;
class Response;

typedef boost::shared_array<std::vector<char> > Buffer;

std::string make_request(Request const& request);
boost::shared_ptr<Responses> get_responses(Buffer const& pdp_answer);

}}}

#endif

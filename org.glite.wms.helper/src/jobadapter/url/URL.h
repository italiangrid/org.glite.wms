/***************************************************************************
 *  Filename  : URL.h
 *  Authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
                Marco Cecchi <marco.cecchi@cnaf.infn.it>
 *  Copyright : (C) 2006 by INFN
 ***************************************************************************/

#ifndef GLITE_WMS_HELPER_JOBADAPTER_URL_H
#define GLITE_WMS_HELPER_JOBADAPTER_URL_H

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace url {

class ExInvalidURL
{
public:
  ExInvalidURL(std::string const& msg);
  const std::string& parameter(void) const;

private:
  std::string m_invalidurl_parameter;
};

class URL
{
public:
  URL(std::string url);
  ~URL(void);

  std::string protocol(void) const;
  std::string host(void) const;
  std::string port(void) const;
  std::string path(void) const;
  std::string as_string(void) const;

private:
  std::string m_protocol;
  std::string m_host;
  std::string m_path;
  std::string m_port;

  void parse(std::string url);
};

} // namespace url
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_URL_H

/***************************************************************************
 *  filename  : URL.h
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *              Francesco Giacomini <francesco.giacomini@cnaf.infn.it>
 *  copyright : (C) 2001 by INFN
 ***************************************************************************/

#ifndef GLITE_WMS_HELPER_JOBADAPTER_URL_H
#define GLITE_WMS_HELPER_JOBADAPTER_URL_H

/**
 *  \defgroup Common
 */

/**
 *  \file URL.h
 *  Define the URL and InvalidURL classes.
 *  This header defines the URL and InvalidURL classes, used in the
 *  ReplicaCatalog submodule of the Broker, in the JobWrapper of the
 *  JobSubmission and  test.
 *  \date Tue 16 Oct 2001
 *  \author Francesco Giacomini
 *  \author Ronchieri Elisabetta
 *  \ingroup Common
 */

#ifndef GLITE_WMS_X_STRING
#define GLITE_WMS_X_STRING
#include <string>
#endif

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {
namespace url {

/**
  * The InvalidURL class.
  * Exception thrown when the URL is not well formed
  * \ingroup Common
  */

class ExInvalidURL
{
public:

  ExInvalidURL(const char *par);

  const std::string& parameter(void) const;

private:

  std::string m_invalidurl_parameter;
};

/**
 * The URL class
 * OO representation for a Uniform Resource Locator
 * \ingroup Common
 */

class URL
{
public:

  /**
   * Constructor.
   * \ingroup Common
   */
  URL(void);

  /**
   * build a URL from a plain string
   * \ingroup Common
   */
  URL(std::string url);

  /**
   * Destructor.
   * \ingroup Common
   */
  ~URL(void);

public:

  bool is_empty(void) const;

public:

  /**
   * return the URL protocol
   * \ingroup Common
   */
  std::string protocol(void) const;

  /**
   * return the URL host
   * \ingroup Common
   */
  std::string host    (void) const;

  /**
   * return the URL port
   * \ingroup Common
   */
  std::string port    (void) const;

  /**
   * return the URL path
   * \ingroup Common
   */
  std::string path    (void) const;

public:

  /**
   * convert a URL in a plain string form
   * \ingroup Common
   */
  std::string as_string(void) const;

private:
  bool m_is_empty;

private:
  std::string m_protocol;
  std::string m_host;
  std::string m_path;
  std::string m_port;

private:

  /**
   * parse the string and initialize the member variables
   * \param url  The url.
   * \ingroup Common
   */
  void parse(std::string url);

};

} // namespace url
} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_URL_H

/***************************************************************************
 *  filename  : JobAdapter.h
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

// $Id$

#ifndef GLITE_WMS_HELPER_JOBADAPTER_JOBADAPTER_H
#define GLITE_WMS_HELPER_JOBADAPTER_JOBADAPTER_H

#ifndef GLITE_WMS_X_BOOST_UTILITY_HPP
#define GLITE_WMS_X_BOOST_UTILITY_HPP
#include <boost/utility.hpp>
#endif

namespace classad {
class ClassAd;
}

namespace glite {
namespace wms {
namespace helper {
namespace jobadapter {

void
replace(std::string& where, const std::string& what, const std::string& with);

class JobAdapter: boost::noncopyable
{
public:
  /**
   * Constructor.
   */
  JobAdapter(classad::ClassAd* ad);
  /**
   * Destructor.
   */
  ~JobAdapter();

public:
  /**
   * Create a new ClassAd using a selection of input ClassAd's ones.
   */
  classad::ClassAd* resolve(void);
  
private: 
  classad::ClassAd* m_ad;
};

} // namespace jobadapter
} // namespace helper
} // namespace wms
} // namespace glite

#endif // GLITE_WMS_HELPER_JOBADAPTER_JOBADAPTER_H

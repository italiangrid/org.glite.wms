/***************************************************************************
 *  filename  : ReplicaServiceException.cpp
 *  authors   : Elisabetta Ronchieri <elisbetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

#include <string>

#include "ReplicaServiceException.h"

using namespace std;

namespace glite {
namespace wms {
namespace rls {

ReplicaServiceException::ReplicaServiceException(void) 
  : exception()
{
}

ReplicaServiceException::~ReplicaServiceException(void) throw()
{
}

const char* 
ReplicaServiceException::what(void) const throw()
{
  return reason().c_str();
}

VoException::VoException(const string& par)
  : m_voex_parameter(par)
{
}

VoException::~VoException(void) throw()
{
}

string 
VoException::reason(void) const
{
  string error("Vo is wrong!");
  error.append(m_voex_parameter);
  return error;
}

const string& 
VoException::parameter(void) const
{
  return m_voex_parameter;
}

LfnException::LfnException(const string& par)
  : m_lfnex_parameter(par)
{
}

LfnException::~LfnException(void) throw()
{
}

string
LfnException::reason(void) const
{
  string error("Lfn is wrong!");
  error.append(m_lfnex_parameter);
  return error;
}

const string&
LfnException::parameter(void) const
{
  return m_lfnex_parameter;
}

InvalidRLS::InvalidRLS(const string& par)
  : m_rlsex_parameter(par)
{
}

InvalidRLS::~InvalidRLS(void) throw()
{
}

string 
InvalidRLS::reason(void) const
{
  string error("Cannot : ");
  error.append(m_rlsex_parameter);
  return error;
}

const string& 
InvalidRLS::parameter(void) const
{
  return m_rlsex_parameter;
}

} // namespace rls
} // namespace wms
} // namespace glite

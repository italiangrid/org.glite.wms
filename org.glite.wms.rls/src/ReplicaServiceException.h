/***************************************************************************
 *  filename  : ReplicaServiceException.h
 *  authors   : Elisabetta Ronchieri <elisabetta.ronchieri@cnaf.infn.it>
 *  Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
 *  For license conditions see LICENSE file or
 *  http://www.edg.org/license.html
 ***************************************************************************/

// $Id$

#ifndef GLIDE_WMS_RLS_REPLICASERVICEEXCEPTION_H
#define GLIDE_WMS_RLS_REPLICASERVICEEXCEPTION_H

#include <exception>
#include <string>

namespace glite {
namespace wms {
namespace rls {

class ReplicaServiceException : public std::exception {
public:
  ReplicaServiceException(void);
  virtual ~ReplicaServiceException(void) throw();

public:
  virtual std::string reason(void) const = 0;
  virtual const char* what(void) const throw();
};

class VoException : public ReplicaServiceException {
public:
  VoException(const std::string& par);
  virtual ~VoException(void) throw();

public:
  virtual std::string reason(void) const;
  const std::string& parameter(void) const;

private:
  std::string m_voex_parameter;
};

class LfnException : public ReplicaServiceException {
public:
  LfnException(const std::string& par);
  virtual ~LfnException(void) throw();

public:
  virtual std::string reason(void) const;
  const std::string& parameter(void) const;

private:
  std::string m_lfnex_parameter;
};

class InvalidRLS : public ReplicaServiceException {
public:
  InvalidRLS(const std::string& par);
  virtual ~InvalidRLS(void) throw();

public:
  virtual std::string reason(void) const;
  const std::string& parameter(void) const;

private:
  std::string m_rlsex_parameter;
};

} // namespace rls
} // namespace wms
} // namespace glite

#endif

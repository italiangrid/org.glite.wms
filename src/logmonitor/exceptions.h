#ifndef EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_EXCEPTIONS_H
#define EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_EXCEPTIONS_H

#include <exception>
#include <iostream>
#include <string>

#include "jobcontrol_namespace.h"

JOBCONTROL_NAMESPACE_BEGIN {

namespace logmonitor {

class MonitorException : public std::exception {
public:
  MonitorException( void );
  virtual ~MonitorException( void ) throw();

  virtual std::string reason( void ) const = 0;
  virtual const char *what( void ) const throw();

private:
  mutable std::string    me_what;
};

class CannotOpenFile : public MonitorException {
public:
  CannotOpenFile( const std::string &filename, int error = 0 );
  virtual ~CannotOpenFile( void ) throw();

  virtual std::string reason( void ) const;

private:
  int            cof_errno;
  std::string    cof_name;
};

class FileSystemError : public MonitorException {
public:
  FileSystemError( const std::string &error );
  virtual ~FileSystemError( void ) throw();

  virtual std::string reason( void ) const;

private:
  std::string    fse_error;
};

class CannotExecute : public MonitorException {
public:
  CannotExecute( const std::string &reason );
  virtual ~CannotExecute( void ) throw();

  virtual std::string reason( void ) const;

private:
  std::string   ce_reason;
};

class InvalidJobId : public MonitorException {
public:
  InvalidJobId( const std::string &id );
  virtual ~InvalidJobId( void ) throw();

  virtual std::string reason( void ) const;

private:
  std::string    iji_id;
};

class InvalidLogFile : public MonitorException {
public:
  InvalidLogFile( const std::string &reason );
  virtual ~InvalidLogFile( void ) throw();

  virtual std::string reason( void ) const;

private:
  std::string   ilf_reason;
};

class InvalidFileName : public MonitorException {
public:
  InvalidFileName( const std::string &filename );
  virtual ~InvalidFileName( void ) throw();

  virtual std::string reason( void ) const;

private:
  std::string   ifn_filename;
};

} // Namespace logmonitor

} JOBCONTROL_NAMESPACE_END

inline std::ostream &operator<<( std::ostream &os, const glite::wms::jobsubmission::logmonitor::MonitorException &me )
{ os << me.what(); return os; }

#endif /* EDG_WORKLOAD_JOBCONTROL_LOGMONITOR_EXCEPTIONS_H */

// Local Variables:
// mode: c++
// End:

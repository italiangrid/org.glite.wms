#ifndef EDG_WORKLOAD_COMMON_UTILITIES_LINEPARSEREXCEPTIONS_H
#define EDG_WORKLOAD_COMMON_UTILITIES_LINEPARSEREXCEPTIONS_H

#include <exception>

COMMON_NAMESPACE_BEGIN {

namespace utilities {

class LineParsingError : public std::exception {
public:
  LineParsingError( const ParserData &d, int code );
  LineParsingError( const LineParsingError &lpe );
  virtual ~LineParsingError( void ) throw();

  virtual void usage( std::ostream &os ) const = 0;
  virtual const char *what( void ) const throw();

  inline int return_code( void ) { return this->lpe_retcode; }

private:
  LineParsingError &operator=( const LineParsingError & ); // Not implemented

protected:
  int                  lpe_retcode;
  const ParserData    &lpe_data;

  static const char   *lpe_s_what;
};

class ShowHelp : public LineParsingError {
public:
  ShowHelp( const ParserData &d );
  ShowHelp( const ShowHelp &sh );
  virtual ~ShowHelp( void ) throw();

  virtual void usage( std::ostream &os ) const;
};

class InvalidOption : public LineParsingError {
public:
  InvalidOption( const ParserData &d, int opt );
  InvalidOption( const InvalidOption &io );
  virtual ~InvalidOption( void ) throw();

  virtual void usage( std::ostream &os ) const;

private:
  int   io_opt;
};

class InvalidArgNumber : public LineParsingError {
public:
  InvalidArgNumber( const ParserData &d, int argn );
  InvalidArgNumber( const InvalidArgNumber &ian );
  virtual ~InvalidArgNumber( void ) throw();

  virtual void usage( std::ostream &os ) const;

private:
  int    ian_argn;
};

}; // Namespace closure

} COMMON_NAMESPACE_END;

inline std::ostream &operator<<( std::ostream &os, const edg::workload::common::utilities::LineParsingError &lpe )
{ lpe.usage( os ); return os; }

#endif /* EDG_WORKLOAD_COMMON_UTILITIES_LINEPARSEREXCEPTIONS_H */

// Local Variables:
// mode: c++
// End:

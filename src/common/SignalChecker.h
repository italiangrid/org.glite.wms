#ifndef EDG_WORKLOAD_JOBCONTROL_COMMON_SIGNALCHECKER_H
#define EDG_WORKLOAD_JOBCONTROL_COMMON_SIGNALCHECKER_H

#include <exception>
#include <string>
#include <vector>
#include <list>

#include <boost/shared_ptr.hpp>

JOBCONTROL_NAMESPACE_BEGIN {

namespace jccommon {

class SignalChecker {
public:
  ~SignalChecker( void );

  void deactivate_signal_trapping( void );
  void reactivate_signal_trapping( void );
  void reset_all_signals( void );
  void throw_on_signal( void );
  bool add_signal( int signum );
  bool reset_signal( int signum );
  bool ignore_signal( int signum );
  int check_signal( void );
  std::vector<bool> add_signals( const std::vector<int> &signums );

  static SignalChecker *instance( void );

  class Exception : std::exception {
  public:
    Exception( int signal );
    virtual ~Exception( void ) throw();

    virtual const char *what( void ) const throw();

    inline int signal( void ) const { return this->e_signal; }
    inline const std::string &reason( void ) const { return *this->e_reason; }

  private:
    int                              e_signal;
    boost::shared_ptr<std::string>   e_reason;
  };

private:
  SignalChecker( void ); // Only the initializer must have such things...

  static SignalChecker     *sh_s_instance;

  std::list<int>            sh_signals;
};

}; // Namespace jccommon

} JOBCONTROL_NAMESPACE_END;

#endif /* EDG_WORKLOAD_JOBCONTROL_COMMON_SIGNALCHECKER_H */

// Local Variables:
// mode: c++
// End:

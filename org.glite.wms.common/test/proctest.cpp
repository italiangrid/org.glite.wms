#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>

#include <iostream>

#include "process/process.h"
#include "process/subprocess.h"

namespace process = glite::wms::common::process;

using namespace std;

class Under : public process::Functor {
public:
  Under( unsigned int max, time_t every );

  virtual ~Under( void );

  virtual int run( void );

private:
  unsigned int    u_max;
  time_t          u_interval;
};

Under::Under( unsigned int max, time_t every ) : u_max( max ), u_interval( every )
{}

Under::~Under( void ) {}

int Under::run( void )
{
  int          res = 0;
  unsigned int i;

  cout << "Starting subprocess in pid: " << process::Process::self()->pid() << endl;

  for( i = 0; i < this->u_max; ++i ) {
    cout << "Subprocess, Iterazione n." << i << endl;

    cout << "Sleeping " << this->u_interval << " seconds..." << endl;
    sleep( this->u_interval );
  }

  cout << "Ending subprocess in pid: " << process::Process::self()->pid() << endl;

  return res;
}

int main( void )
{
  process::Subprocess   *son;
  Under                  under( 10, 2 );

  srand( time(NULL) );

  cout << "Starting subprocess test in pid: " << process::Process::self()->pid() << endl;

  cout << "Created subprocess..." << endl;
  son = process::Process::self()->fork( under );

  cout << "Waiting 5 seconds (" << son->is_ended() << "," << son->is_signaled() << ")..." << endl;
  sleep( 5 );

  if( rand() %2 ) {
    cout << "Killing son..." << endl;

    son->signal( SIGKILL );
  }

  cout << "Waiting son to exit..." << endl;
  process::Process::self()->wait_one( son );

  cout << "proc->is_ended() = " << son->is_ended() << endl;
  cout << "proc->is_signaled() = " << son->is_signaled() << endl;

  return 0;
}

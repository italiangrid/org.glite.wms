
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/wait.h>

using namespace std;

int main( int argc, char *argv[]) {
  if( argc>=3 ) {
    char buf[1024];
    memset((void*)buf, 0, 1024);
    
    sprintf(buf, "%s --conf %s", 
	    "/opt/glite/bin/glite-wms-ice",
	    argv[2]);

    
    
    while(true) {
      cout << "Starting real ICE..." << endl;
      cout << "executing [" << buf << "]"<<endl;
      int ret = ::system( buf );
      int wret = WEXITSTATUS(ret);
      cout << "ret=["<< wret << "]" << endl;
      if( 2 == wret) { sleep(2); continue; }
      exit(wret);
    }
  }
}

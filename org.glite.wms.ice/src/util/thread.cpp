
#include "thread.h"
#include <unistd.h>
#include <iostream>

static int  res         = 0;

using namespace glite::wms::ice::util;

//______________________________________________________________________________
extern "C" 
{
  void* coroutine(void* obj) {
    ((runnable*)obj)->run();
    return &res;
  }
}

//______________________________________________________________________________
thread::thread(runnable& object) : _object(&object)
{
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}

//______________________________________________________________________________
void thread::start() throw(thread_start_ex&)
{
  int ret = pthread_create(&t, &attr, coroutine, (void*)_object);
  if(ret == EAGAIN) {
    throw thread_start_ex("not enough system resources to create a new thread or more than PTHREAD_THREADS_MAX threads are already active");
  }
}

//______________________________________________________________________________
void thread::stop()
{
  _object->stop();
}

//______________________________________________________________________________
void thread::join()
{
}

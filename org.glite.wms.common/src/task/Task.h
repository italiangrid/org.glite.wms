// File: Task.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2001 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef EDG_WORKLOAD_COMMON_TASK_TASK_H
#define EDG_WORKLOAD_COMMON_TASK_TASK_H

#include <deque>
#include <cassert>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

namespace edg {
namespace workload {
namespace common {
namespace task {

// exceptions
struct Eof {};
struct SigPipe {};
struct Empty {};
struct Full {};

template<typename T> class Pipe;

template<typename T>
class PipeReadEnd {

  friend class Pipe<T>;

  Pipe<T>* m_parent;

public:
  PipeReadEnd(Pipe<T>* parent = 0): m_parent(parent) {}
  PipeReadEnd(const PipeReadEnd<T>& rhs) : m_parent(rhs.m_parent) {}
  PipeReadEnd<T>& operator=(const PipeReadEnd<T>& rhs)
  {
    m_parent = rhs.m_parent;
    return *this;
  }
  ~PipeReadEnd() {}
  T read() { return m_parent->read(); }
  T try_read() { return m_parent->try_read(); }
  void open() { m_parent->open_read_end(); }
  void close() { m_parent->close_read_end(); }
};

template<typename T>
class PipeWriteEnd {

  friend class Pipe<T>;

  Pipe<T>* m_parent;

public:
  PipeWriteEnd(Pipe<T>* parent = 0): m_parent(parent) {}
  PipeWriteEnd(const PipeWriteEnd& rhs): m_parent(rhs.m_parent) {}
  PipeWriteEnd& operator=(const PipeWriteEnd& rhs)
  {
    m_parent = rhs.m_parent;
    return *this;
  }
  ~PipeWriteEnd() {}

  void write(T obj) { m_parent->write(obj); }
  void try_write(T obj) { m_parent->try_write(obj); }
  void open() { m_parent->open_write_end(); }
  void close() { m_parent->close_write_end(); }
};

// when a pipe is created it is open both for reading and for writing,
// although there are no readers and writers at that moment
// a pipe end (read or write) becomes closed when all the threads who had
// previously opened it have then closed it
template<typename T>
class Pipe {

  boost::mutex m_mutex;       // control any access to the pipe
  std::deque<T> m_queue;
  size_t m_max_size;
  boost::condition m_not_full; // sync access to the queue
  boost::condition m_not_empty; // sync access to the queue

  int m_num_writers;
  int m_num_readers;

  bool m_write_end_is_closed;
  bool m_read_end_is_closed;

  // non-copyable
  Pipe(const Pipe& rhs);
  Pipe& operator=(const Pipe& rhs);

public:
  Pipe(size_t max_size = 10)
    : m_max_size(max_size), m_num_writers(0), m_num_readers(0),
      m_write_end_is_closed(false), m_read_end_is_closed(false)
  {
  }
  ~Pipe() {}

  PipeReadEnd<T> read_end() { return PipeReadEnd<T>(this); }
  PipeWriteEnd<T> write_end() { return PipeWriteEnd<T>(this); }

  // before the first read is done m_num_writers > 0
  T read()
  {
    boost::mutex::scoped_lock lock(m_mutex);

    while (m_queue.empty()) {
      if (m_write_end_is_closed) { // see comment in close_write_end()
        throw Eof();
      }
      m_not_empty.wait(lock);
    }
    T result = m_queue.front();
    m_queue.pop_front();
    m_not_full.notify_one();

    return result;
  }

  T try_read()
  {
    boost::mutex::scoped_lock lock(m_mutex);

    if (m_queue.empty()) {
      if (m_write_end_is_closed) { // see comment in close_write_end()
        throw Eof();
      } else {
        throw Empty();
      }
    }

    T result = m_queue.front();
    m_queue.pop_front();
    m_not_full.notify_one();

    return result;
  }

  void write(const T& obj)
  {
    boost::mutex::scoped_lock lock(m_mutex);

    if (m_read_end_is_closed) {
      throw SigPipe();
    }

    while (m_queue.size() == m_max_size) {
      m_not_full.wait(lock);
      if (m_read_end_is_closed) { // see comment in close_read()
        throw SigPipe();
      }
    }
    m_queue.push_back(obj);
    m_not_empty.notify_one();
  }

  void try_write(const T& obj)
  {
    boost::mutex::scoped_lock lock(m_mutex);

    if (m_read_end_is_closed) {
      throw SigPipe();
    } else if (m_queue.size() == m_max_size) {
      throw Full();
    }

    m_queue.push_back(obj);
    m_not_empty.notify_one();
  }

  bool empty()
  {
    boost::mutex::scoped_lock lock(m_mutex);

    return m_queue.empty();
  }

  void open_read_end(int num = 1)
  {
    boost::mutex::scoped_lock lock(m_mutex);

    m_num_readers += num;
  }

  void open_write_end(int num = 1)
  {
    boost::mutex::scoped_lock lock(m_mutex);

    m_num_writers += num;
  }

  void close_read_end()
  {
    boost::mutex::scoped_lock lock(m_mutex);

    --m_num_readers;
    if (m_num_readers == 0) {
      // be careful when writing... (see comment in write())
      m_read_end_is_closed = true;
      // wake up all writers so that they can realize that they will never
      // be able to push data again into the pipe
      m_not_full.notify_all();
    }
  }

  void close_write_end()
  {
    boost::mutex::scoped_lock lock(m_mutex);

    --m_num_writers;
    if (m_num_writers == 0) {
      // be careful when reading... (see comment in read())
      m_write_end_is_closed = true;
      // wake up all readers so that they can realize that no other data
      // will ever appear on the pipe
      m_not_empty.notify_all();
    }
  }

};

template<typename T_IN>
class PipeReader
{
  PipeReadEnd<T_IN> m_read_end;

  friend class Task;
  void read_from(const PipeReadEnd<T_IN>& from) { m_read_end = from; }

protected:
  PipeReader() {}
  PipeReadEnd<T_IN>& read_end() { return m_read_end; }

public:
  virtual ~PipeReader() {}
  virtual void run() = 0;
};


template<typename T_OUT>
class PipeWriter
{
  PipeWriteEnd<T_OUT> m_write_end;

  friend class Task;
  void write_to(const PipeWriteEnd<T_OUT>& to) { m_write_end = to; }

protected:
  PipeWriter() {}
  PipeWriteEnd<T_OUT>& write_end() { return m_write_end; }

public:
  virtual ~PipeWriter() {}
  virtual void run() = 0;
};

template<typename T_IN, typename T_OUT>
class PipeForwarder
{
  PipeReadEnd<T_IN> m_read_end;
  PipeWriteEnd<T_OUT> m_write_end;

  friend class Task;
  void read_from(const PipeReadEnd<T_IN>& from) { m_read_end = from; }
  void write_to(const PipeWriteEnd<T_OUT>& to) { m_write_end = to; }

protected:
  PipeForwarder() {}
  PipeReadEnd<T_IN>& read_end() { return m_read_end; }
  PipeWriteEnd<T_OUT>& write_end() { return m_write_end; }

public:
  virtual ~PipeForwarder() {}
  virtual void run() = 0;
};

// aux class for "Resource acquisition is initialization"
template<typename E>
struct Access
{
  E& m_e;
  Access(E& e): m_e(e) { m_e.open(); }
  ~Access() { m_e.close(); }
};
 
// aux class to guarantee to close a pipe end at the end of the thread function
template<typename E>
struct CloseOnExit
{
  E& m_e;
  CloseOnExit(E& e): m_e(e) {}
  ~CloseOnExit() { m_e.close(); }
};
 
template<typename T_IN>
class ReaderFunctor
{
  PipeReader<T_IN>& m_runner;
  PipeReadEnd<T_IN> m_read_end;

public:
  ReaderFunctor(PipeReader<T_IN>& runner, PipeReadEnd<T_IN> end)
    : m_runner(runner), m_read_end(end)
  {}

  void operator()()
  {
    CloseOnExit< PipeReadEnd<T_IN> > a(m_read_end);

    m_runner.run();
  }
};

template<typename T_OUT>
class WriterFunctor
{
  PipeWriter<T_OUT>& m_runner;
  PipeWriteEnd<T_OUT> m_write_end;

public:
  WriterFunctor(PipeWriter<T_OUT>& runner, PipeWriteEnd<T_OUT> end)
    : m_runner(runner), m_write_end(end)
  {}

  void operator()()
  {
    CloseOnExit< PipeWriteEnd<T_OUT> > a(m_write_end);

    m_runner.run();
  }
};

template<typename T_IN, typename T_OUT>
class ForwarderFunctor
{
  PipeForwarder<T_IN, T_OUT>& m_runner;
  PipeReadEnd<T_IN> m_read_end;
  PipeWriteEnd<T_OUT> m_write_end;

public:
  ForwarderFunctor(PipeForwarder<T_IN, T_OUT>& runner,
                   PipeReadEnd<T_IN> read_end,
                   PipeWriteEnd<T_OUT> write_end)
    : m_runner(runner), m_read_end(read_end), m_write_end(write_end)
  {}

  void operator()()
  {
    CloseOnExit< PipeReadEnd<T_IN> > r(m_read_end);
    CloseOnExit< PipeWriteEnd<T_OUT> > w(m_write_end);

    m_runner.run();
  }
};

class Task
{
  boost::thread_group* m_group;

  // non-copyable
  Task(const Task& rhs);
  Task& operator=(const Task& rhs);

public:

  Task(const boost::function0<void>& fun, int num_threads = 1)
    : m_group(new boost::thread_group)
  {
    assert(num_threads > 0);

    while (num_threads--) {
      m_group->create_thread(fun);
    }
  }

  template<typename T>
  Task(PipeReader<T>& runner, Pipe<T>& pipe, int num_threads = 1)
    : m_group(new boost::thread_group)
  {
    assert(num_threads > 0);

    runner.read_from(pipe.read_end());
    ReaderFunctor<T> f(runner, pipe.read_end());
    pipe.open_read_end(num_threads);
    while (num_threads--) {
      m_group->create_thread(f);
    }
  }

  template<typename T>
  Task(PipeWriter<T>& runner, Pipe<T>& pipe, int num_threads = 1)
    : m_group(new boost::thread_group)
  {
    assert(num_threads > 0);

    runner.write_to(pipe.write_end());
    WriterFunctor<T> f(runner, pipe.write_end());
    pipe.open_write_end(num_threads);
    while (num_threads--) {
      m_group->create_thread(f);
    }
  }

  template<typename T_IN, typename T_OUT>
  Task(PipeForwarder<T_IN, T_OUT>& runner,
       Pipe<T_IN>& pipe_in, Pipe<T_OUT>& pipe_out,
       int num_threads = 1)
    : m_group(new boost::thread_group)
  {
    assert(num_threads > 0);

    runner.read_from(pipe_in.read_end());
    pipe_in.open_read_end(num_threads);
    runner.write_to(pipe_out.write_end());
    pipe_out.open_write_end(num_threads);
    ForwarderFunctor<T_IN, T_OUT> f(runner, pipe_in.read_end(), pipe_out.write_end());
    while (num_threads--) {
      m_group->create_thread(f);
    }      
  }

  ~Task()
  {
    m_group->join_all();
    delete m_group;
  }

};

} // namespace task
} // namespace common
} // namespace workload
} // namespace edg

#endif // EDG_WORKLOAD_COMMON_TASK_TASK_H

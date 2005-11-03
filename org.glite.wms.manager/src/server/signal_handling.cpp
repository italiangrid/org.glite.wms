// File: signal_handling.c
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "signal_handling.h"
#include <iostream>
#include <csignal>
#include <pthread.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace {

bool f_received_quit_signal;
boost::mutex::mutex f_mx;
sigset_t f_mask;

void signal_handler()
{
  while (!f_received_quit_signal) {
    int signal_number;
    int err = sigwait(&f_mask, &signal_number);
    boost::mutex::scoped_lock l(f_mx);
    if (err) {
      f_received_quit_signal = true;
    } else {
      switch (signal_number) {
      case SIGINT:
        std::cerr << "SIGINT\n";
        f_received_quit_signal = true;
        break;
      case SIGTERM:
        std::cerr << "SIGTERM\n";
        f_received_quit_signal = true;
        break;
      case SIGQUIT:
        std::cerr << "SIGQUIT\n";
        f_received_quit_signal = true;
        break;
      case SIGPIPE:
        std::cerr << "SIGPIPE\n";
        break;
      default:
        std::cerr << "caught signal " << signal_number << '\n';
        break;
      }
    }
  }
}

}

namespace glite {
namespace wms {
namespace manager {
namespace server {

namespace {

void join_and_delete(boost::thread* t)
{
  if (t) {
    t->join();
  }
  delete t;
}

}

boost::shared_ptr<boost::thread> signal_handling_init()
{
  sigemptyset(&f_mask);
  sigaddset(&f_mask, SIGPIPE);
  sigaddset(&f_mask, SIGINT);
  sigaddset(&f_mask, SIGTERM);
  sigaddset(&f_mask, SIGQUIT);

  sigset_t* const oldmask = 0;        // don't care about the oldmask
  if (pthread_sigmask(SIG_BLOCK, &f_mask, oldmask)) {
    return boost::shared_ptr<boost::thread>();
  }

  return boost::shared_ptr<boost::thread::thread>(
    new boost::thread::thread(signal_handler),
    join_and_delete
  );
}

bool received_quit_signal()
{
  boost::mutex::scoped_lock l(f_mx);
  return f_received_quit_signal;
}

}}}} // glite::wms::manager::server

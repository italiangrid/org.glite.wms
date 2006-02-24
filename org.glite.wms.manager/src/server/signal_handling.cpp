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
#include "glite/wms/common/logger/logger_utils.h"

namespace {

bool f_received_quit_signal;
boost::mutex::mutex f_mx;
sigset_t f_mask;

void signal_handler()
{
  while (!f_received_quit_signal) {
    int signal_number;
    sigwait(&f_mask, &signal_number);
    Info("caught signal " << signal_number);
    boost::mutex::scoped_lock l(f_mx);
    switch (signal_number) {
      case SIGINT:
      case SIGTERM:
      case SIGQUIT:
        f_received_quit_signal = true;
        break;
      case SIGPIPE:
        break;
      default:
        break;
    }
  }
}

}

namespace glite {
namespace wms {
namespace manager {
namespace server {

bool signal_handling()
{
  sigemptyset(&f_mask);
  sigaddset(&f_mask, SIGPIPE);
  sigaddset(&f_mask, SIGINT);
  sigaddset(&f_mask, SIGTERM);
  sigaddset(&f_mask, SIGQUIT);

  sigset_t* const oldmask = 0;        // don't care about the oldmask
  int sigmask_error = pthread_sigmask(SIG_BLOCK, &f_mask, oldmask);
  if (sigmask_error) {
    Error("pthread_sigmask failure (" << sigmask_error << ")\n");
    return false;
  }

  boost::thread::thread t(signal_handler); // run detached

  return true;
}

bool received_quit_signal()
{
  boost::mutex::scoped_lock l(f_mx);
  return f_received_quit_signal;
}

}}}} // glite::wms::manager::server

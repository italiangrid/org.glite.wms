/*
 * File: logging.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#ifndef _GLITE_WMS_WMPROXY_COMMANDS_LOGGING_H_
#define _GLITE_WMS_WMPROXY_COMMANDS_LOGGING_H_

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level) 
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name)
#define glitelogTag(level) logger::threadsafe::edglog << logger::setlevel(logger::level) << "*********"
#define glitelogHead(level)logger::threadsafe::edglog << logger::setlevel(logger::level) << "* Error *"
#define glitelogBody(level)logger::threadsafe::edglog << logger::setlevel(logger::level) << "*       *"

#endif

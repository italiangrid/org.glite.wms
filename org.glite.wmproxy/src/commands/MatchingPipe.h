/*
 * File: MatchingPipe.h
 * Author: MarcoP <marco.pappalardo@ct.infn.it>
 * Copyright (c) 2004 EGEE.
 */
 
// $Id

#ifndef GLITE_WMS_WMPROXY_COMMANDS_MATCHINGPIPE_H
#define GLITE_WMS_WMPROXY_COMMANDS_MATCHINGPIPE_H

#include <classad_distribution.h>
#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> /* for unlink, read, write, and close */
#include <stdio.h>  /* for perror */


namespace classad {
  class ClassAd;
};

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {


/**
 * A PipeWrapper for listmatch results retrieval.
 *
 * @version
 * @date November 18 2004
 * @author Marco Pappalardo
 */
class MatchingPipe
{
 public:

  int pipefd;
  bool ispipeopen;
  std::string pipepath;

 public:
  /**
   * Constructor.
   */
  MatchingPipe(std::string &path);
 
  /**
   * Destructor
   */
  virtual ~MatchingPipe();

  /**
   * Opens the pipe.
   * @return a boolean indicating whether the command was successful or not.
   */
  bool open();
  
  /**
   * Closes the pipe.
   * @return a boolean indicating whether the command was successful or not.
   */
  bool close();

  /**
   * Read the pipe containt.
   * @return a string representation of return values taken from the pipe.
   */
  std::string read();
};
  
} // namespace commands
} // namespace wmproxy
} // namespace wms
} // namespace glite

#endif

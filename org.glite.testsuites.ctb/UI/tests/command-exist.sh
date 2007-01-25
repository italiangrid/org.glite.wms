#!/bin/bash

# function command_exist takes single argument $1
# and returns 0 if $1 is a valid command name and not an alias
# otherwise returns 1

# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version info: $Id$
# Release: $Name$

function command_exist() {

  echo "testing if $1 exists in the system ..."
  if /usr/bin/which $1; then
    echo "... Yes"
  elif alias $1; then
    echo "... No! But there is an alias called $1!"
    return 1
  else
    echo "... Error! $1 not found! Check your installation"
    return 1
  fi

  return 0
}

#!/usr/bin/python
##############################################################################
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################
#
# AUTHORS: Dimitar Shiyachki <Dimitar.Shiyachki@cern.ch>
#
##############################################################################

import os
import sys
import traceback
import dpm2 as dpm

argv = os.sys.argv
argc = len(argv)

filename = argv[1]

try:

   replicas = dpm.dpns_getreplicax (filename, "", "")

   for rep in replicas :

      print "%d:%s:%s:%s" % ( rep.fileid, rep.r_type, rep.f_type, rep.setname )

except ValueError:

   sys.exit(1)


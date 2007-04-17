#!/bin/sh
#
# Config file for the SRM tests
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

# location of basic SRM 2.2 s2 tests
export S2_TESTS_LOCATION=/afs/cern.ch/user/z/zaborov/public/s2-20070402/testing/scripts/protos/srm/2.2/basic

# location of s2 binary
export PATH=$PATH:/afs/cern.ch/user/z/zaborov/public/s2-20070402/bin

# test only SEs in this list (empty list = no restriction)
export RESTRICT_TESTS_TO="lxb2036.cern.ch lxb1921.cern.ch"

# VO to use in SAPath query (empty = use VO from voms proxy)
export VO=dteam

# location of SRM 2.2 tests from LCG-DM package (Gilbert's tests)
export LCGDM_SRM22_TEST_LOCATION=/afs/cern.ch/user/z/zaborov/public/srm2.2-tests

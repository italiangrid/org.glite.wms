#!/usr/bin/env python
#
# Copyright (c) Members of the EGEE Collaboration. 2009
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Author: Ricardo Mendes
# Author: Sofia Sayzhenkova

import unittest, optparse, sys, os,errno
from util import environment
from util import ldap_util
from util import voms_util
try:
        import gfal
except ImportError:
        print "Import of GFAL module failed. Please install it"
        sys.exit(1)

env=environment()
ldap=ldap_util()
voms=voms_util()

#################################
# Environment needed for testing#
#################################
testEnv={}
#############################
# Data to use during testing#
############################
testData={}
testData['err_code'] = None
testData['gfal_obj'] = None
testData['err_msg'] = None
testData['surl'] = None
testData['fd'] = None

gfal_dict = {}

####################################################################
#Class to test GFAL Errors. EPROTONOSUPPORT Failures are expected! #
####################################################################
class TestGFALErrors(unittest.TestCase):

        def setUp(self):
                pass

        def tearDown(self):
                pass

        def setSurlToSrm(self):
                testData['surl'] = "srm://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlToLfn(self):
                testData['surl'] = "lfn:/grid/" + testEnv['vo_name'] + "/sa3tmp" + str(os.getpid())

        def setSurlToSfn(self):
                testData['surl'] = "sfn://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlToFake(self):
                testData['surl'] = "fake://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlDirname(self):
                testData['surl'] = os.path.dirname(testData['surl'])
                         
        def testInit(self):
                gfal_dict['defaultsetype'] = 'srmv2'
                gfal_dict['surls'] = [testData['surl']]
                gfal_dict['srmv2_desiredpintime'] = 60
                gfal_dict['srmv2_lslevels'] = 1
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_init(gfal_dict)

       
        def testClean(self):
                gfal.gfal_internal_free(testData['gfal_obj'])

 
        def testRmdir(self):
                testData['err_code'] = gfal.gfal_rmdir(testData['surl'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testMkdir(self):
                testData['err_code'] = gfal.gfal_mkdir(testData['surl'],0755)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testAbortfiles(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_abortfiles(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                pass

        def testAccess(self):
                testData['err_code'] = gfal.gfal_access(testData['surl'],os.R_OK)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                pass

        def testBringonline(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg']= gfal.gfal_bringonline(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testCreat(self):
                testData['err_code'] = gfal.gfal_creat(testData['surl'],0755)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                testData['fd'] = testData['err_code']

        def testCreatLfn(self):
                testData['err_code'] = gfal.gfal_creat(testData['surl'],0755)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                testData['fd'] = testData['err_code']
               
        def testGet(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_get(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testUnlink(self):
                testData['err_code'] = gfal.gfal_unlink(testData['surl'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testChmod(self):
                testData['err_code'] = gfal.gfal_chmod(testData['surl'],0777)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
        
        def testOpendir(self):
                testData['err_code'] = gfal.gfal_opendir(os.path.dirname(testData['surl']))
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                testData['fd']=testData['err_code']

        def testClosedir(self):
                testData['err_code'] = gfal.gfal_closedir(testData['fd'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testReaddir(self):
                testData['err_code'] = gfal.gfal_readdir(testData['fd'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testDeletesurls(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_deletesurls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testGetstatus(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_get(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testLs(self):
                testData['err_code'], buffer, testData['err_msg'] = gfal.gfal_ls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testOpen(self):
                testData['err_code'] = gfal.gfal_open(testData['surl'],os.O_RDWR, 0644)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)
                testData['fd'] = testData['err_code'] 

        def testLseek(self):
                testData['err_code'] = gfal.gfal_lseek(testData['fd'],1,gfal.SEEK_SET)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testPin(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_pin(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testPrestage(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_prestage(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testPrestagestatus(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_prestagestatus(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testRead(self):
                testData['err_code'], buffer = gfal.gfal_read(testData['fd'], 255)
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testRelease(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_release(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testRename(self):
                testData['err_code'] = gfal.gfal_rename(testData['surl'],testData['surl']+"_new")
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testSetxferdone(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_set_xfer_done(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testSetxferrunning(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_set_xfer_running(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testStat(self):
                testData['err_code'], buffer = gfal.gfal_stat(os.path.dirname(testData['surl']))
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testTurlsfromsurls(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_turlsfromsurls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)

        def testWrite(self):
                testData['err_code'] = gfal.gfal_write(testData['fd'],"Testing GFAL write")
                self.assertEqual(testData['err_code'],-1)
                self.assertEqual(gfal.gfal_get_errno(),errno.EPROTONOSUPPORT)


################################################################
#Class to test GFAL Basic functions. Failures are not expected!#
################################################################
class TestGFALFunctions(unittest.TestCase):

        def setSurlToSrm(self):
                testData['surl'] = "srm://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlToLfn(self):
                testData['surl'] = "lfn:/grid/" + testEnv['vo_name'] + "/sa3tmp" + str(os.getpid())

        def setSurlToSfn(self):
                testData['surl'] = "sfn://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlToFake(self):
                testData['surl'] = "fake://" + testEnv['se'] + testEnv['sa_path'] + "/sa3temp" + str(os.getpid())

        def setSurlDirname(self):
                testData['surl'] = os.path.dirname(testData['surl'])

        def testInit(self):
                gfal_dict['defaultsetype'] = 'srmv2'
                gfal_dict['surls'] = [testData['surl']]
                gfal_dict['srmv2_desiredpintime'] = 60
                gfal_dict['srmv2_lslevels'] = 1
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_init(gfal_dict)

       
        def testClean(self):
                gfal.gfal_internal_free(testData['gfal_obj'])

 
        def testRmdir(self):
                testData['err_code'] = gfal.gfal_rmdir(testData['surl'])
                self.assertEqual(testData['err_code'],0)

        def testMkdir(self):
                testData['err_code'] = gfal.gfal_mkdir(testData['surl'],0755)
                self.assertEqual(testData['err_code'],0)
                pass

        def testAbortfiles(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_abortfiles(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)
                pass

        def testAccess(self):
                testData['err_code'] = gfal.gfal_access(testData['surl'],os.R_OK)
                self.assertEqual(testData['err_code'],0)
                pass

        def testBringonline(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg']= gfal.gfal_bringonline(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testCreat(self):
                testData['err_code'] = gfal.gfal_creat(testData['surl'],0755)
                self.assertNotEqual(testData['err_code'],-1)
                testData['fd'] = testData['err_code']

        def testCreatLfn(self):
                testData['err_code'] = gfal.gfal_creat(testData['surl'],0755)
                self.assertNotEqual(testData['err_code'],-1)
                testData['fd'] = testData['err_code']
               
        def testClose(self):
                testData['err_code'] = gfal.gfal_close(testData['fd'])
                self.assertEqual(testData['err_code'],0)

        def testCloseLfn(self):
                testData['err_code'] = gfal.gfal_close(testData['fd'])
                self.assertEqual(testData['err_code'],0)

        def testGet(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_get(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testUnlink(self):
                testData['err_code'] = gfal.gfal_unlink(testData['surl'])
                self.assertEqual(testData['err_code'],0)

        def testChmod(self):
                testData['err_code'] = gfal.gfal_chmod(testData['surl'],0777)
                self.assertEqual(testData['err_code'],0)
        
        def testOpendir(self):
                testData['err_code'] = gfal.gfal_opendir(os.path.dirname(testData['surl']))
                self.assertNotEqual(testData['err_code'],-1)
                testData['fd']=testData['err_code']

        def testClosedir(self):
                testData['err_code'] = gfal.gfal_closedir(testData['fd'])
                self.assertEqual(testData['err_code'],0)

        def testReaddir(self):
                testData['err_code'] = gfal.gfal_readdir(testData['fd'])
                self.assertNotEqual(testData['err_code'],-1)

        def testDeletesurls(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_deletesurls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testGetstatus(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_get(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testLs(self):
                testData['err_code'], buffer, testData['err_msg'] = gfal.gfal_ls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testOpen(self):
                testData['err_code'] = gfal.gfal_open(testData['surl'],os.O_RDWR, 0644)
                self.assertNotEqual(testData['err_code'],-1)
                testData['fd'] = testData['err_code'] 

        def testLseek(self):
                testData['err_code'] = gfal.gfal_lseek(testData['fd'],1,gfal.SEEK_SET)
                self.assertNotEqual(testData['err_code'],-1)

        def testPin(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_pin(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testPrestage(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_prestage(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testPrestagestatus(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_prestagestatus(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testRead(self):
                testData['err_code'], buffer = gfal.gfal_read(testData['fd'], 255)
                self.assertEqual(testData['err_code'],0)

        def testRelease(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_release(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testRename(self):
                testData['err_code'] = gfal.gfal_rename(testData['surl'],testData['surl']+"_new")
                self.assertEqual(testData['err_code'],0)

        def testSetxferdone(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_set_xfer_done(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testSetxferrunning(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_set_xfer_running(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testStat(self):
                testData['err_code'], buffer = gfal.gfal_stat(os.path.dirname(testData['surl']))
                self.assertEqual(testData['err_code'],0)

        def testTurlsfromsurls(self):
                testData['err_code'], testData['gfal_obj'], testData['err_msg'] = gfal.gfal_turlsfromsurls(testData['gfal_obj'])
                self.assertEqual(testData['err_code'],0)

        def testWrite(self):
                testData['err_code'] = gfal.gfal_write(testData['fd'],"Testing GFAL write")
                self.assertNotEqual(testData['err_code'],-1)


##############################################################
#Parse options and verify that a required option has been set#
##############################################################
class option_parser(optparse.OptionParser):

        """Check for mandatory options"""
        def check_required(self, opt):
                option = self.get_option(opt)
                if getattr(self.values,option.dest) is None:
                        self.error("%s option not supplied" % option)

def usage():
        usage = "usage: %prog --se <host>"
        parser = option_parser(usage=usage)
        parser.add_option("--se",
                          action="store",
                          dest="se",
                          default=None,
                          help="Storage Element to use for testing")
        (options, args) = parser.parse_args()
        parser.check_required("--se")
        return (options, args)

#################################################################
#Get and set all the necessary information from the environment #
#needed to run the testsuites                                   #
#################################################################
def prepareEnvironment(options):
        try:
                voms.check_valid_proxy()
                infosys = env.get_infosys()
                ldap.parse_infosys(infosys)
                vo_name = voms.get_vo()
                se_type=ldap.find_se_type(options.se)
                sa_path=ldap.find_sa_path(options.se,vo_name)
                try:
                        lfc = env.get_lfc()
                except:
                        lfc = ldap.find_lfc(vo_name)[0][0]
                testEnv['infosys'] = infosys
                testEnv['vo_name'] = vo_name
                testEnv['se_type'] = se_type
                testEnv['sa_path'] = sa_path
                testEnv['lfc'] = lfc
                testEnv['se'] = options.se
                env.set_lfc(lfc)
                env.set_dpm_host(options.se)
                env.set_dpns_host(options.se)
                env.set_vo_default_se(vo_name,options.se)
        except:
                print "Failed to run tests. Possible reasons:"
                print "- Proxy expired"
                print "- Infosys is not defined"
                print "- SE is not in the BDII"
                print "- LFC is not defined and could not found one in BDII. Try exporting one"
                sys.exit(1)

#########################################################
#Preparing the tests order and setting the surl and lfn #
#Note that some methods depend on other so the order of #
#the tests is very important                            #
#########################################################
def suite(testcase = "SUCCESS"):

        if testcase == "SUCCESS":
                tests = [
                          'setSurlToSrm', 
                          'testInit', 
                          'testCreat',
                          'testClose',
                          'testGet',
                          'testGetstatus',
                          'testStat',
                          'testTurlsfromsurls',
                          'testAbortfiles',
                          'testBringonline',
                          'testRelease',
                          'testPin',
                          'testSetxferrunning',
                          'testSetxferdone',
                          'testPrestage',
                          'testPrestagestatus',
                          'testAccess',
                          'testOpen',
                          'testWrite',
                          'testRead',
                          'testLseek',
                          'testClose',
                          'setSurlToLfn',
                          'testOpendir',
                          'testReaddir',
                          'testClosedir',
                          'testMkdir', 
                          'testRmdir',
                          'setSurlToSrm',
                          'testUnlink',
                          'testDeletesurls',
                          'setSurlToLfn',
                          'testCreat',
                          'testChmod',
                          'testRename',
                          'testClose',
                          'testClean',
                          'setSurlToSrm',
                          'setSurlDirname',
                          'testInit',
                          'testLs',
                          'testClean'
                        ]

                return unittest.TestSuite(map(TestGFALFunctions, tests))

        elif testcase == errno.EPROTONOSUPPORT:

                tests = [ 
                          'setSurlToFake',
                          'testCreat',
                          'testOpen',
                          'setSurlToSfn',
                          'testInit', 
                          'testGet',
                          'testGetstatus',
                          #'testTurlsfromsurls',
                          'testAbortfiles',
                          'testBringonline',
                          #'testRelease', #Error code returns 0 should be -1
                          #'testPin',
                          #'testSetxferrunning',
                          #'testSetxferdone',
                          'testPrestage',
                          'testPrestagestatus',
                          #'testAccess',
                          #'testWrite',
                          #'testRead',
                          #'testLseek',
                          #'testClose',
                          #'testOpendir',
                          #'testReaddir',
                          #'testClosedir',
                          #'testMkdir', 
                          #'testRmdir',
                          #'testUnlink',
                          #'testDeletesurls',
                          #'testCreatLfn',
                          #'testChmod',
                          #'testRename',
                          #'testCloseLfn',
                          #'testClean',
                          #'testInitDir',
                          #'testLs',
                          'testClean'
                        ]

                return unittest.TestSuite(map(TestGFALErrors, tests))
        else:
                print "No testcase defined"
                sys.exit(1)

#############
#Entry Point#
#############
def main():
        options, args=usage()
        prepareEnvironment(options)
        print "-"*70
#       unittest.TestLoader().sortTestMethodsUsing = None
        print "BASIC FUNCTIONALITY TEST"
        print "-"*70
        testsuite = suite()
        unittest.TextTestRunner(verbosity=2).run(testsuite)
        print "-"*70
        print "EPROTONOSUPPORT TEST"
        print "-"*70
        testsuite = suite(errno.EPROTONOSUPPORT)
        unittest.TextTestRunner(verbosity=2).run(testsuite)




if __name__ == "__main__":
        sys.exit(main())

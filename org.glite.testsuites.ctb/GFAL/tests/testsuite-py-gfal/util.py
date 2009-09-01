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


import sys, os, string, popen2, re
try:
        import ldaphelper
	import ldap
except ImportError:
	print "Loading python-ldap module failed. Please install it"
	sys.exit(1)

#####################################
# Set and Get environment variables #
#####################################
class environment:

	def __init__(self):
		pass

	def __del__(self):
		pass

        def set(self, variable, value):
                try:
                        os.environ[variable]=value
                except:
                        raise

        def get(self, variable):
                try:
                        return os.environ[variable]
                except:
                        raise

	def get_infosys(self):
		try:
			return os.environ["LCG_GFAL_INFOSYS"]
		except:
			raise

        def get_lfc(self):
                try:
                        return os.environ["LFC_HOST"]
                except:
                        raise

        def set_lfc(self, lfc):
                try:
                        os.environ["LFC_HOST"]=lfc
                except:
                        raise

        def get_dpm_host(self):
                try:
                        return os.environ["DPM_HOST"]
                except:
                        raise

        def set_dpm_host(self, dpm):
                try:
                        os.environ["DPM_HOST"]=dpm
                except:
                        raise

        def get_dpns_host(self):
                try:
                        return os.environ["DPNS_HOST"]
                except:
                        raise

        def set_dpns_host(self, dpns):
                try:
                        os.environ["DPNS_HOST"]=dpns
                except:
                        raise

        def get_vo_default(self, vo):
                try:
                        return os.environ["VO_" + vo.upper() + "_DEFAULT_SE"]
                except:
                        raise
 
        def set_vo_default_se(self, vo, se):
                try:
                        os.environ["VO_" + vo.upper() + "_DEFAULT_SE"]=se
                except:
                        raise

#################################################
# Get ldap SE information and parse the results #
#################################################
class ldap_util:

	def __init__(self):
		#ldap setup
		self.con = None
                # information needed to do the queries or to retrieve from the queries
                self.bdii = None
                self.se_type = []
                self.sa_path = None
                self.lfc = []

	def __del__(self):
		pass
	
	def parse_infosys(self, infosys):
		infosys_list=infosys.split(',')
		if str(len(infosys_list)) !=0:
                        self.bdii = infosys_list[0]
                        if re.search("/",self.bdii):
                                self.bdii=self.bdii.split('/')[0]
		else:
                        raise

        def _get_results_ldap(self,filter,attr):
		self.con = ldap.initialize('ldap://' + self.bdii)
		try:
                        self.con.bind_s('','')
                        raw_res = self.con.search_s("Mds-vo-name=local, o=Grid",ldap.SCOPE_SUBTREE,filter,attr)
                        self.con.unbind()
                        return raw_res
                except ldap.LDAPError, e:
                        raise

	def find_se_type(self,se):
                filter="(&(objectClass=GlueSEControlProtocol)(GlueChunkKey=GlueSEUniqueid=" + se +"))"
                attr_str = "GlueSEControlProtocolType"
                attr=[].append(attr_str)
		result=self._get_results_ldap(filter,attr)
                res = ldaphelper.get_search_results( result )
                for record in res:
                        if record.has_attribute(attr_str.lower()):
                                map(self._iter_attr_not_in_se_list,record.get_attr_values(attr_str.lower()))
                se_type_sz = len(self.se_type)
                if se_type_sz == 0:
                        raise
                return self.se_type

        def _iter_attr_not_in_se_list(self,attr):
                if attr not in self.se_type:
                        self.se_type.append(attr)

	def find_sa_path(self,se,vo):
                filter="(&(GlueSALocalID:dn:="+ vo +")(GlueSEUniqueID:dn:="+ se +"))"
                attr_str = "GlueSAPath"
                attr=[].append(attr_str)
		result=self._get_results_ldap(filter,attr)
                res = ldaphelper.get_search_results( result )
                for record in res:
                        if record.has_attribute(attr_str.lower()):
                                self.sa_path=record.get_attr_values(attr_str.lower())[0]
                if self.sa_path == "" and self.sa_path == None:
                        raise
                return self.sa_path

        def find_lfc(self,vo):
                filter="(&(GlueServiceType=lcg-file-catalog)(GlueServiceAccessControlRule="+ vo +"))"
                attr_str = "GlueServiceEndpoint"
                attr=[].append(attr_str)
		result=self._get_results_ldap(filter,attr)
                res = ldaphelper.get_search_results( result )
                for record in res:
                        if record.has_attribute(attr_str.lower()):
                                self.lfc.append(record.get_attr_values(attr_str.lower()))
                lfc_sz=len(self.lfc)
                if lfc_sz == 0:
                        raise
                return self.lfc

############################
# Verify proxy information #
############################
class voms_util:
	
	def __init__(self):
                pass

	def __del__(self):
		pass

	def __run(self, options):
                result = []
		p_obj = popen2.Popen3('`which voms-proxy-info` ' + options,1,1)

                for line in p_obj.fromchild:
                   result.append(line)
                for line in p_obj.childerr:
                   result.append(line)

                ret = p_obj.wait()
		return (ret,result)

	def check_valid_proxy(self):
		options = "-exists"
		if self.__run(options)[0] != 0:
                        raise

	def get_vo(self):
		options = "--vo "
		run_obj = self.__run(options)    
                if run_obj[0] != 0:
                        raise
                return run_obj[1][0].strip()


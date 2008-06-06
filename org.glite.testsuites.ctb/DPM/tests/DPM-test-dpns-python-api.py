#!/usr/bin/python

import sys
import dpm
	
name = "/dpm/cern.ch/home/org.glite.voms-test/";

dir = dpm.dpns_opendirg(name,"")
if (dir == None) or (dir == 0):
	err_num = dpm.cvar.serrno
	err_string = dpm.sstrerror(err_num)
	print "Error while looking for " + name + ": Error " + str(err_num) + " (" + err_string + ")"
	sys.exit(1)

while 1:
	read_pt = dpm.dpns_readdirxr(dir,"")
	if (read_pt == None) or (read_pt == 0):
		break
	entry, list = read_pt
	print entry.d_name
	try:
		for i in range(len(list)):
			print " ==> %s" % list[i].sfn
	except TypeError, x:
		print " ==> None"

dpm.dpns_closedir(dir)

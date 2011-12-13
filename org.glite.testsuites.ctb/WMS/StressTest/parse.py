#!/usr/bin/python

import sys
import re
import string
import getopt

# Regex expressions
# extract jobid
jobidRE = re.compile("Status info for the Job :[ \t\n\r\f\v]*(.+)")
# extract status
statusRE = re.compile("Current Status:[ \t\n\r\f\v]*(.+)[ \t\n\r\f\v]*")
# extract status reason
statusreasonRE = re.compile("Status Reason:[ \t\n\r\f\v]*(.+)[ \t\n\r\f\v]*")
# extract destination ce
destinationRE = re.compile("Destination:[ \t\n\r\f\v]*(.+)")
# check if it is a parent or not
childRE = re.compile("Children num[ \t\n\r\f\v]*=[ \t\n\r\f\v]*(\d+)")
# check if it has been resubmitted
resubmitRE = re.compile("Resubmitted[ \t\n\r\f\v]*=[ \t\n\r\f\v]*(\d+)")
# job's separator: \n====== [string] =======\n 
sep = re.compile("[\n]+[\=]+.*[\=]+[\n]+")
# extract failure reasons
failreasRE=re.compile("- Failure reasons[ \t\n\r\f\v]*=[ \t\n\r\f\v]*(.+)[ \t\n\r\f\v]*- Jobtype", re.DOTALL)
# cename (use also as separator for the failure reasons)
cenameRE=re.compile("\[(.*):[0-9]*\/.*\]\n")

# most common failure reason
failreason = ["BLAH error", "Cannot move ISB", "Cannot move OSB", "Cannot download", 
						"Lease expired", "blparser service", "Transfer to CREAM failed", 
						"Proxy is expired", "Cannot take token", "lsf_reason", "pbs_reason", 
						"Epilogue failed", "Prologue failed", "hit job shallow retry count", 
						"hit job retry count", "Got a job held event", 
						"Job got an error while in the CondorG queue"]

# some file extension
abortedstr = ".abort"
failstr = ".fail"
donestr = ".done"
cancelstr = ".canc"
notdonestr = ".notdone"

# look for most common failure reasons and update counters["fail"]
# string is the error line, counters must be already initialized
def checkfail(string, counters):

	if ( not string ):
		# if string is empty return
		return 1
	for item in failreason:
		if ( re.compile(item, re.IGNORECASE).search(string) <> None ):
			counters["fail"][item] += 1
	counters["fail"]["Total"] += 1

	return 0

# Analyze job extracting all the relevant information
def analyze(job, testname, counters):

	if ( not job.split()):
		return 2

	# look if it is a collection
	child = childRE.search(job)
	if ( child <> None ):
		if ( child.group(1) <> "0" ):
			# this is a collection, return
			return 1

	jobid = jobidRE.search(job)
	if ( jobid<>None ):
		status = statusRE.search(job)
		if ( status<>None ):
			dest = destinationRE.search(job)
			if (dest<>None):
				ce = dest.group(1)
				# ce host name
				cename = dest.group(1).split(":")[0]
			else:
				ce = "CeUnknown"
				cename = "CeUnknown"
			# update status counter
			if not counters.has_key(cename):
				counters[cename] = {}
				counters[cename]["resubmitted"] = 0
			if not counters[cename].has_key(status.group(1)):
				counters[cename][status.group(1)] = 1
			else:
				counters[cename][status.group(1)] += 1
		
      # check if the job has been resubmitted
			reason=""
			rs = resubmitRE.search(job)
			if ( rs.group(1) == "1" ):
				resub = "1"
				# look for failure reason(s)
				fr=failreasRE.search(job)
				if ( fr <> None ):
					# lf should contains a list with tha failure reason 
					# alternate to the cename where the job fails
					lf=re.split(cenameRE, fr.group(1))
					i=0
					while i < len(lf)-1:
						checkfail(lf[i], counters)
						# save the reason
						reason=reason + " --> " + lf[i] + "\n"
						i += 1
						# every failure reason is associate with its ce
						if not counters.has_key(lf[i]):
							counters[lf[i]] = {}
							counters[lf[i]]["resubmitted"] = 1
						else:
							counters[lf[i]]["resubmitted"] += 1
						i += 1
				# update resubmit counter
				counters["resubmitted"] += 1
				# save failure reasons in a file
				fail = open(testname + failstr, "a")
				fail.write(jobid.group(1) + "\n")
				fail.write(reason + "\n")
				fail.close()
			else:
				resub = "0"

      # put toghether Done Ok and "Exit Code != 0" jobs with the "Cleared" ones 
			if ( status.group(1) == "Done (Success)" or status.group(1) == "Done (Exit Code !=0)" or status.group(1) == "Cleared "):
				done = open(testname + donestr, "a")
				done.write(jobid.group(1) + "\t" + "Resubmitted=" + resub + "\t\t" + ce + "\n")		
				done.close()

			# Job Aborted
			elif ( status.group(1) == "Aborted " ):
				sr = statusreasonRE.search(job)
				if ( sr<>None ):
					statreas = sr.group(1)
					# Add aborted reason to failure reasons
					checkfail(statreas, counters)
				else:
					statreas = "N/A"

				aborted = open(testname + abortedstr, "a")
				aborted.write(jobid.group(1) + "\t" + ce + "\t" + statreas + "\n")
				aborted.close()
				        
			# Job Cancelled
			elif ( status.group(1) == "Cancelled "):
				canc = open(testname + cancelstr, "a")
				canc.write(jobid.group(1) + "\n")
				canc.close()

			# job not terminated
			else:
				notdone = open(testname + notdonestr, "a")
				notdone.write(jobid.group(1) + "\n")
				notdone.close()
	
		else:
			# we are not able to extract the Status
			print("The job " + jobid + " is in an unknown status")
			return 4

	else:
		# we are not able to extract the JobID
		print("Error for job " + job)
		return 3

	return 0

def print_result(counters):

	# set some funny tags
	bold = "\033[1m"
	red = "\033[1;31m"
	green = "\033[1;32m"
	yellow = "\033[1;33m"
	reset = "\033[0;0m"

	total = {}
	# Labels	
	name = ["Submit", "Ready", "Waiting", "Schedul", "Running", "DoneOK", "Done!=0", "DoneFail", "Aborted", "Cancel", "Cleared", "Resub"]
	for status in name:
		print "%s%8s%s" % (yellow, status[:8], reset),
	print yellow + "  Tot.      Ce Name " + reset
	print ""

	# initialization
	stat = ["Submitted ", "Ready ", "Waiting ", "Scheduled ", "Running ", "Done (Success)", "Done (Exit Code !=0)", "Done (Failed)", "Aborted ", "Cancelled ", "Cleared "]
	for status in stat:
		total[status]=0
	total["resub"]=0

	for ce in counters.keys():
		if ( ce <> "fail" and ce <> "resubmitted" ):
			tot=0
			for status in stat:
				if ( not counters[ce].has_key(status) ):
					num = 0
					print " %6d " % (num),
				else:
					num = counters[ce][status]
					print " %s%6d%s " % (bold, num, reset),
				total[status]+=num
				tot+=num
			print " %6d " % (counters[ce]["resubmitted"]),
			total["resub"]+=counters[ce]["resubmitted"]
			print "%s%6d%s   %s" % (red, tot, reset, ce)

	print ""
  # print totals
	tot=0
	for status in stat:
		print " %s%6d%s " % (green, total[status], reset),
		tot+=total[status]
	print " %s%6d%s " % (green, total["resub"], reset),
	print "%s%6d%s   %s" % (green, tot, reset, "Total")
	print ""
	if ( tot ):
		print "Number of job resubmitted: %d (%.2f%%)" % (counters["resubmitted"], counters["resubmitted"]*100.00/tot)
	print ""
	# failure reasons
	if ( counters["fail"]["Total"] ):
		print "Most common failures' logged reasons:"
		print ""
		tfl=0
		pfl=100.00/counters["fail"]["Total"]
		for item in failreason:
			if (counters["fail"][item] != 0 ):
				print "%5d (%.2f%%)\t%s%s%s" % (counters["fail"][item], counters["fail"][item]*pfl, bold, item, reset)
				tfl+=counters["fail"][item]
		if (counters["fail"]["Total"] > tfl):
			print "%5d (%.2f%%)\t%s%s%s" % (counters["fail"]["Total"]-tfl, (counters["fail"]["Total"]-tfl)*pfl, bold, "... others reasons", reset)
		print "------------"
		print "%s%5d (100%%)\t%s%s" % (red, counters["fail"]["Total"], "Total", reset)

def usage():
	print 'Use python parse.py [-h] -f filename'
	print ' '
	print '   -h            this help'
	print '   -f filename   the file to parse'
	print '\n Note the file should contains information obtain with the command glite-wms-job-status -v 2'

def main():

	filename=""

	try:
		opts, args = getopt.getopt(sys.argv[1:], "hf:")
	except getopt.GetoptError, err:
		# print help information and exit:
		print str(err) # will print something like "option -a not recognized"
		usage()
		sys.exit(2)
	for o, a in opts:
		if o == "-f":
			filename = a.split(".")[0]
			try:
				file = open(a)
			except IOError:
				print "File doesn't exists"
				usage()
				sys.exit(2)
		elif o in ("-h"):
			usage()
			sys.exit()
		else:
			assert False, "unhandled option"

	if not filename:
		print "A valid filename is required"
		usage()
		sys.exit(2)

	print("\n\n Parsing file " + filename + " ....\n")

	# initialize counters
	counters = {}
	counters["fail"] = {}
	counters["resubmitted"] = 0
	for item in failreason:
		counters["fail"][item] = 0
	counters["fail"]["Total"] = 0

	# read file
	jobs=re.split(sep, file.read())
	file.close()

	# analyze single jobs
	for job in jobs:
		analyze(job, filename, counters)
	
	print_result(counters)

	return 0


if __name__ == "__main__":
    main()




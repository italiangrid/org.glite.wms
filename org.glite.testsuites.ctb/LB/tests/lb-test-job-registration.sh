#!/bin/bash
#
# Copyright (c) Members of the EGEE Collaboration. 2004-2010.
# See http://www.eu-egee.org/partners for details on the copyright holders.
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
#

# show help and usage
progname=`basename $0`
showHelp()
{
cat << EndHelpHeader
Script for testing correct job registration

Prerequisities:
   - LB server
   - environment variables set:

     GLITE_LB_SERVER_PORT - if nondefault port (9000) is used

Tests called:

    job registration

Returned values:
    Exit TEST_OK: Test Passed
    Exit TEST_ERROR: Test Failed
    Exit 2: Wrong Input

EndHelpHeader

	echo "Usage: $progname [OPTIONS]"
	echo "Options:"
	echo " -h | --help            Show this help message."
	echo " -o | --output 'file'   Redirect all output to the 'file' (stdout by default)."
	echo " -t | --text            Format output as plain ASCII text."
	echo " -c | --color           Format output as text with ANSI colours (autodetected by default)."
	echo " -x | --html            Format output as html."
}

# read common definitions and functions
COMMON=lb-common.sh
if [ ! -r ${COMMON} ]; then
	printf "Common definitions '${COMMON}' missing!"
	exit 2
fi
source ${COMMON}

logfile=$$.tmp
flag=0
while test -n "$1"
do
	case "$1" in
		"-h" | "--help") showHelp && exit 2 ;;
		"-o" | "--output") shift ; logfile=$1 flag=1 ;;
		"-t" | "--text")  setOutputASCII ;;
		"-c" | "--color") setOutputColor ;;
		"-x" | "--html")  setOutputHTML ;;
	esac
	shift
done

# redirecting all output to $logfile
touch $logfile
if [ ! -w $logfile ]; then
	echo "Cannot write to output file $logfile"
	exit $TEST_ERROR
fi

DEBUG=2

##
#  Starting the test
#####################

{
test_start


# check_binaries
printf "Testing if all binaries are available"
check_binaries $GRIDPROXYINFO $SYS_GREP $SYS_SED $SYS_AWK
if [ $? -gt 0 ]; then
	test_failed
else
	test_done
fi

printf "Testing credentials"

timeleft=`${GRIDPROXYINFO} | ${SYS_GREP} -E "^timeleft" | ${SYS_SED} "s/timeleft\s*:\s//"`

if [ "$timeleft" = "" ]; then
        test_failed
        print_error "No credentials"
else
        if [ "$timeleft" = "0:00:00" ]; then
                test_failed
                print_error "Credentials expired"
        else
                test_done

		# Register job:
		printf "Registering testing job "
		jobid=`${LBJOBREG} -m ${GLITE_WMS_QUERY_SERVER} -s application | $SYS_GREP "new jobid" | ${SYS_AWK} '{ print $3 }'`

		if [ -z $jobid  ]; then
			test_failed
			print_error "Failed to register job"
		else
			test_done

			# Check result
			jobstate=`${LBJOBSTATUS} ${jobid} | $SYS_GREP "state :" | ${SYS_AWK} '{print $3}'`
			printf "Is the testing job ($jobid) in a correct state? $jobstate"

			if [ "${jobstate}" = "Submitted" ]; then
				test_done
			else
				test_failed
				print_error "Job has not been submitted"
			fi

			printf "Trying to re-register job with the same jobid..."
			${LBJOBREG} -m ${GLITE_WMS_QUERY_SERVER} -s application -j $jobid > /dev/null

			noofevents=`${LBHISTORY} $jobid | $SYS_NL | $SYS_TAIL -n 1 | ${SYS_AWK} '{print $1}'`

			printf "(Event No. $noofevents)..."		
	
			if [ "${noofevents}" = "2" ]; then
				test_done
			else
				test_failed
				print_error "Second registration did not take place"
			fi


			printf "Trying to re-register job with the same jobid, 'exclusive' flag on..."
			${LBJOBREG} -m ${GLITE_WMS_QUERY_SERVER} -s application -j $jobid -E > /dev/null

			noofevents=`${LBHISTORY} $jobid | $SYS_NL | $SYS_TAIL -n 1 | ${SYS_AWK} '{print $1}'`

			printf "(Event No. $noofevents)..."		
	
			if [ "${noofevents}" = "2" ]; then
				test_done
			else
				test_failed
				print_error "Wrong number of registration events"
			fi


			#Purge test job
			joblist=$$_jobs_to_purge.txt
			echo $jobid > ${joblist}
			${LBPURGE} -j ${joblist} > /dev/null
			$SYS_RM ${joblist}

			jobstate=`${LBJOBSTATUS} ${jobid} | $SYS_GREP "state :" | ${SYS_AWK} '{print $3}'`
                        printf "Test job purged. Testing state... ($jobstate)"

                        if [ "${jobstate}" = "Purged" ]; then
                                test_done


				${LBJOBREG} -h 2>&1 | $SYS_GREP '\-E' > /dev/null

				if [ $? = 0 ]; then
					printf "Trying to re-register. Same JobID, exclusive flag..."
					${LBJOBREG} -m ${GLITE_WMS_QUERY_SERVER} -s application -j $jobid -E > /dev/null
					jobstate=`${LBJOBSTATUS} ${jobid} | $SYS_GREP "state :" | ${SYS_AWK} '{print $3}'`

					if [ "${jobstate}" = "Purged" ]; then
						test_done
					else
						test_failed
						print_error "Job has not remained in purged state"
					fi

				else
					printf "Client does not support the 'exclusive' flag."
					test_skipped
				fi


                        else
                                printf "Job has not been purged, re-registration test will be skipped"

				test_skipped
                        fi


		fi

		
		
	fi
fi

test_end
} &> $logfile

if [ $flag -ne 1 ]; then
 	cat $logfile
 	$SYS_RM $logfile
fi
exit $TEST_OK


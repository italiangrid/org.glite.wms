#!/bin/bash

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
     GLITE_WMS_QUERY_SERVER
     GLITE_WMS_NOTIF_SERVER

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
check_binaries $GRIDPROXYINFO $SYS_GREP $SYS_SED $SYS_AWK $SYS_CURL
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

		# Get path to the proxy cert
		printf "Getting proxy cert path... "

		PROXYCERT=`${GRIDPROXYINFO} | ${SYS_GREP} -E "^path" | ${SYS_SED} "s/path\s*:\s//"`

	        if [ "$PROXYCERT" = "" ]; then
        	        test_failed
                	print_error "Unable to identify the path to your proxy certificate"
	        else
			printf "$PROXYCERT"
        	        test_done
	
			# Register job:
			printf "Registering testing job "
			jobid=`${LBJOBREG} -m ${GLITE_WMS_QUERY_SERVER} -s application | $SYS_GREP "new jobid" | ${SYS_AWK} '{ print $3 }'`

			if [ -z $jobid  ]; then
				test_failed
				print_error "Failed to register job"
			else
				test_done

				# Get list of jobs
				printf "Evaluating job list... "

				$SYS_CURL -3 --silent --key $PROXYCERT --cert $PROXYCERT --capath /etc/grid-security/certificates --output https.$$.tmp https://${GLITE_WMS_QUERY_SERVER}/

				if [ "$?" != "0" ]; then
					test_failed
					print_error "Job list not returned"
				else
					test_done

					printf "Looking up the test job..."

					$SYS_GREP $jobid https.$$.tmp > /dev/null 2> /dev/null

					if [ "$?" != "0" ]; then
						test_failed
						print_error "Test job not found in the list"
					else
						test_done
					fi

					rm https.$$.tmp

				fi

				# Get job status
				printf "Evaluating job status listing... "

				$SYS_CURL -3 --silent --key $PROXYCERT --cert $PROXYCERT --capath /etc/grid-security/certificates --output https.$$.tmp "${jobid}"

				if [ "$?" != "0" ]; then
					test_failed
					print_error "Job status not returned"
				else
					test_done

					printf "Checking for jobid (verifying content)..."

					$SYS_GREP $jobid https.$$.tmp > /dev/null 2> /dev/null

					if [ "$?" != "0" ]; then
						test_failed
						print_error "JobID not found among data returned"
					else
						test_done
					fi

					rm https.$$.tmp

				fi

				#Purge test job
				joblist=$$_jobs_to_purge.txt
				echo $jobid > ${joblist}
				try_purge ${joblist}

			fi

	                # Register notification:
	                printf "Registering notification "

	                notifid=`${LBNOTIFY} new -j ${jobid} | $SYS_GREP "notification ID" | ${SYS_AWK} '{ print $3 }'`
			echo ${LBNOTIFY} new -j ${jobid}
	
        	        if [ -z $notifid ]; then
                	        test_failed 
                        	print_error "Failed to register notification"
	                else
        	                printf "(${notifid}) "
                	        test_done

				# Get notification status
				printf "Evaluating notification status listing... "

				$SYS_CURL -3 --silent --key $PROXYCERT --cert $PROXYCERT --capath /etc/grid-security/certificates --output https.$$.tmp "${notifid}"

				if [ "$?" != "0" ]; then
					test_failed
					print_error "Job status not returned"
				else
					test_done

					printf "Checking for jobid (verifying content)..."

					notifunique=`${SYS_ECHO} ${notifid} | ${SYS_SED} 's/^.*NOTIF://'`

					$SYS_GREP $notifunique https.$$.tmp > /dev/null 2> /dev/null

					if [ "$?" != "0" ]; then
						test_failed
						print_error "Notification ID not found among data returned"
					else
						test_done
					fi

					rm https.$$.tmp

                                fi

				#Drop notification
	                        printf "Dropping the test notification (${notifid})"
        	                dropresult=`${LBNOTIFY} drop ${notifid} 2>&1`
                	        if [ -z $dropresult ]; then
                        	        test_done
	                        else
        	                        test_failed
                	                print_error "Failed to drop notification ${dropresult}"
                        	fi

	                        #Purge test job
         	                joblist=$$_jobs_to_purge.txt
                	        echo $jobid > ${joblist}
                        	try_purge ${joblist}


			fi


		fi
		
		
	fi
fi

test_end
} &> $logfile

if [ $flag -ne 1 ]; then
 	cat $logfile
 	rm $logfile
fi
exit $TEST_OK


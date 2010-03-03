#!/bin/bash

# show help and usage
progname=`basename $0`
showHelp()
{
cat << EndHelpHeader
Script for testing correct reporting of LB server properties over BDII/LDAP.
This should also be thought of as a regression test for bug #55482.

Prerequisities:
   - LB server
   - environment variables set:

     GLITE_WMS_QUERY_SERVER 

Tests called:

    ldap query to the server, checking the output

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
check_binaries $SYS_GREP $SYS_SED $SYS_AWK $SYS_LDAPSEARCH
if [ $? -gt 0 ]; then
	test_failed
else
	test_done

	# Register job:

	server=`${SYS_ECHO} ${GLITE_WMS_QUERY_SERVER} | ${SYS_SED} 's/:.*$//'`

	printf "Checking if BDII operational... "
	$SYS_LDAPSEARCH -x -H ldap://${server}:2170 -b 'o=infosys' > ldap.$$.out
	if [ $? -gt 0 ]; then	
		test_failed
		print_error "No reply"
	else
		test_done
	fi

	printf "Checking Glue 2.0 root entry... "
	$SYS_LDAPSEARCH -x -H ldap://${server}:2170 -b 'o=grid' > ldap.$$.out
	if [ $? -gt 0 ]; then	
		test_failed
		print_error "No reply"
	else
		test_done
	fi

	printf "Checking GlueServiceVersion... "
	glservver=`$SYS_GREP GlueServiceVersion: ldap.$$.out | $SYS_SED 's/^.*GlueServiceVersion:\s*//'`
	if [ "$glservver" == "" ]; then	
		print_error "GlueServiceVersion not specified"
		test_failed
	else
		printf "$glservver"
		test_done
	fi

	rm ldap.$$.out

fi		
		

test_end
} &> $logfile

if [ $flag -ne 1 ]; then
 	cat $logfile
 	$SYS_RM $logfile
fi
exit $TEST_OK


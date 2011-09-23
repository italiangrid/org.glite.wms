#!/bin/sh
##############################################################################
#
# NAME:        nagrun.sh
#
# FACILITY:    SAM (Service Availability Monitoring)
#
# COPYRIGHT:
#         Copyright (c) 2009-2011, Members of the EGEE Collaboration.
#         http://www.eu-egee.org/partners/
#         Licensed under the Apache License, Version 2.0.
#         http://www.apache.org/licenses/LICENSE-2.0
#         This software is provided "as is", without warranties
#         or conditions of any kind, either express or implied.
#
# DESCRIPTION:
#
#         WN script to set up required environment, launch and monitor Nagios
#         instance, send messages (tests results) to Message Bus.
#
# AUTHORS:     Konstantin Skaburskas, CERN
#
# CREATED:     09-Jan-2008
#
##############################################################################

# Set en_US.UTF-8 locale
for l in `locale`;do export ${l//=*/}="en_US.UTF-8";done

VO=
VO_FQAN=
MB_DEST=
MB_URI=
MB_NET=PROD
MB_DISCOVER=1
MB_SORT=1
NAGLIFETIME=600
FW_VERB=1
METRICS_VERB=0
NAGIOS_RUN=1
MTA=1

usage="usage: $(basename $0) -v <vo> -d <dest> [-b <broker_uri>]\n
 [-n <broker_network>] [-t <timeout>] [-w <fw_verb>]\n
 [-z <metric_verb>] [-f <fqan>] [-i <host:port,..>] -B -R -N -h -m\n
-v and -d (if not -m) are mandatory paramters. Defaults:\n
<broker_network> - $MB_NET\n
<timeout> - $NAGLIFETIME sec\n
<metrics_verb> - $METRICS_VERB\n
<fw_verb> - $FW_VERB (2 - messages, 3 - Nagios config/stats/debug)\n
-f <fqan> - VOMS FQAN\n
-B - don't do broker discovery\n
-R - take MB randomly; by default sort by min response time\n
-N - don't run WN tests\n
-m - don't use mta service to transfer messages\n
"

while getopts ":t:v:f:d:b:w:z:n:BSNmh" options; do
  case $options in
    t ) NAGLIFETIME=$OPTARG;;
    v ) VO=$OPTARG;;
    f ) VO_FQAN=$OPTARG;;
    d ) MB_DEST=$OPTARG;;
    b ) MB_URI=${OPTARG//\'/ };;
    n ) MB_NET=$OPTARG;;
    w ) FW_VERB=$OPTARG;;
    z ) METRICS_VERB=$OPTARG;;
    B ) MB_DISCOVER=0;;
    R ) MB_SORT=0;;
    N ) NAGIOS_RUN=0;;
    m ) MTA=0;;
    h ) echo -e $usage
        exit 1;;
    \? ) echo -e $usage
         exit 1;;
    * ) echo -e $usage
         exit 1;;
  esac
done

if [ -z "$VO" ] ; then
  echo -e $usage
  exit 1
fi

if [ $MTA -eq 1 ] && [ -z "$MB_DEST" ] ; then
	echo -e $usage
	exit 1
fi

echo "Launched with parameters: $@"

if [ $FW_VERB -ge 2 ] ; then
	echo "VO=$VO"
	echo "VO_FQAN=$VO_FQAN"
	echo "MB_DEST=$MB_DEST"
	echo "MB_URI=$MB_URI"
	echo "MB_DISCOVER=$MB_DISCOVER"
	echo "MB_SORT=$MB_SORT"
	echo "NAGLIFETIME=$NAGLIFETIME"
	echo "NAGIOS_RUN=$NAGIOS_RUN"
  echo "MTA=$MTA"
fi

# Nagios compliant exit codes
RC_OK=0
RC_WARNING=1
RC_CRITICAL=2
RC_UNKNOWN=3

trap _on_exit EXIT

function _on_exit {
    if [ $MTA -eq 1 ]; then
        _stop_mta
    fi
    rm -rf $NAGROOT $PROBESWD $FW_TMP
}

function _start_mta () {
    if [ ! $MB_URI ] && [ $MB_DISCOVER -eq 1 ] ; then
        MTA_MB_URI=''
        MTA_BDII_URI="--bdii-uri $LCG_GFAL_INFOSYS"
    else
        MTA_MB_URI="--broker-uri $MB_URI"
        MTA_BDII_URI=''
    fi
    if [ $FW_VERB -ge 2 ]; then
    	VERB='debug'
    elif [ $FW_VERB -eq 1 ]; then
        VERB='info'
    elif [ $FW_VERB -eq 0 ]; then
        VERB='warn'
    fi
    cmd="$NAGROOT/bin/$MTABIN --dirq $MSG_OUTGOING \
                            --destination $MB_DEST \
                            --broker-network $MB_NET \
                            --pidfiledir $NAGROOT/var/ \
                            -v $VERB \
                            $MTA_BDII_URI $MTA_MB_URI"
    echo "Launching MTA."
    echo $cmd
    $cmd
    if [ $? != 0 ]; then
        echo "WARNING: Couldn't launch MTA."
        echo "Bailing out."
        exit $RC_WARNING
    fi
}

function _stop_mta () {
    # delay to let MTA to clean message queue
    if ( find ${MSG_OUTGOING}/[0-9]* -type f >/dev/null 2>&1 ); then
        SLEEP=3
        SLEPT=0
        while (( find ${MSG_OUTGOING}/[0-9]* -type f >/dev/null 2>&1 ) && \
            [ $SLEPT -lt $SLEEP ]); do
            sleep 1
            SLEPT=$((SLEPT + 1))
        done
    fi
    if [ -f $MTAPIDFILE ]; then
       kill -15 $(cat $MTAPIDFILE) || \
       kill -s SIGTERM $(cat $MTAPIDFILE)
    else
       echo "NOTE: No MTA pid-file $MTAPIDFILE"
    fi
}

function _package_wnlogs() {
    tar -c $NAGROOT/{var,tmp} $MSG_OUTGOING | gzip > $WNLOGS
}

function _echo_nagstats {
	_totalsvc=$1
	_checkedsvc=$2
	echo ">>>>>>>>>>>>>>>>>> $(date)"
	echo "T |S |c |U |O |W |C |A |P |"
	$NAGSTATS -c $NAGCONF -m -D " |" -d \
		NUMSERVICES,NUMSVCSCHEDULED,NUMSVCCHECKED,NUMSVCUNKN,NUMSVCOK,NUMSVCWARN,NUMSVCCRIT,NUMOACTSVCCHECKS1M,NUMPSVSVCCHECKS1M
	echo "Services Total ${_totalsvc} Checked: ${_checkedsvc}"
}

function _validate_nagios_config() {
   $NAGROOT/bin/nagios -v $NAGCONF || \
      { echo "Nagios conf files contain errors. Bailing out." ; \
        echo "=== $NAGROOT/etc/wn.d/*.cfg" ; \
        for f in `find $NAGROOT/etc/ -name *.cfg`; do \
        	echo -e "\n=>\n=> $f\n=>"; cat $f|nl -ba -s' ' -w3; done ; \
        exit $RC_UNKNOWN; }
}

function _set_nagios_config() {
   echo "Setting Nagios configuration."
   NAGCFG=$(find $NAGROOT/etc/*/ -name *.cfg)
   if [ $FW_VERB -ge 3 ] ; then
      echo "=== $NAGCONF ==="
      egrep "^[a-zA-Z]" $NAGCONF
      echo "=== $NAGRESOURCE ==="
      cat $NAGRESOURCE
      echo "=== *.cfg in $NAGROOT/etc/ ==="
      cat $NAGCFG
   fi

   sed -i -e "s|<nagiosRoot>|${NAGROOT}|g" \
   	-e "s|<nagiosLockFile>|${NAGROOT}/var/nagios.lock|g" \
   	-e "s|<nagiosUser>|"$(id -u)"|g" \
   	-e "s|<nagiosGroup>|"$(id -g)"|g" \
   	-e "s|<serviceCheckTimeout>|600|g" \
   	-e "s|<eventHandlerTimeout>|15|g" \
   	-e "s|<ocspTimeout>|15|g" \
   	-e "s|<statusUpdateInterval>|3|g" \
   	-e "s|<maxConcurrentChecks>|15|g" \
   	-e "s|<siteName>|${SITE_NAME}|g" \
   	-e "s|<nodeIP>|127.0.0.1|g" \
   	-e "s|<nodeName>|localhost.localdomain|g" \
   	-e "s|<ceName>|${CE_NAME}|g" \
   	-e "s|<VO>|${VO}|g" \
   	-e "s|<VOMS>|${VO_FQAN:-$VO}|g" \
   	-e "s|<userHome>|${HOME}|g" \
   	-e "s|<wnjobWorkDir>|${JWD}|g" \
    -e "s|<probesWorkDir>|${PROBESWD}|g" \
    -e "s|<VERBOSITY>|${METRICS_VERB}|g" \
    -e "s|<nagiosCMDFileDIR>|${NAGCMDFILEDIR}|g" \
    -e "s|<MSG_OUTGOING>|${MSG_OUTGOING}|g" \
    -e "s|<nagiosDebugLevel>|${NAGDEBUGLEVEL:-0}|g" \
   	$NAGCONF $NAGRESOURCE $NAGCFG

   _validate_nagios_config

   if [ $FW_VERB -ge 3 ] ; then
      echo "=== $NAGCONF ==="
      egrep "^[a-zA-Z]" $NAGCONF
      echo "=== $NAGRESOURCE ==="
      cat $NAGRESOURCE
      echo "=== *.cfg in $NAGROOT/etc/ ==="
      cat $NAGCFG
   fi
}

function _wait_nagios_statusdat () {
	if [ ! -f ${NAGROOT}/var/status.dat ] ; then
	    SLEEP=10
	    SLEPT=0
	    while ([ ! -f ${NAGROOT}/var/status.dat ] && [ $SLEPT -lt $SLEEP ]); do
	        sleep 2
	        SLEPT=$((SLEPT + 2))
	    done
	    if [ ! -f ${NAGROOT}/var/status.dat ] ; then
	        echo "WARNING: No Nagios status.dat file after $SLEEP sec."
	        echo "WARNING: ${NAGROOT}/var/status.dat"
	        echo "WARNING: Cannot proceed. Check WN. Bailing out."
	        exit $RC_WARNING
	    fi
	    echo "NOTE: Nagios status.dat file appeared after $SLEPT sec."
	fi
}

#
ARCH=$(uname -m)
JWD=$(pwd)
export NAGROOT=$JWD/nagios
NAGCONF=$NAGROOT/etc/nagios.cfg
NAGRESOURCE=$NAGROOT/etc/resource.cfg
PROBESWD=$JWD/gridprobes

NAGDAEMON=''

NAGSTATS=$NAGROOT/bin/nagiostats

MTABIN=mta-simple
MTAPIDFILEDIR=$NAGROOT/var
MTAPIDFILE=$MTAPIDFILEDIR/$MTABIN.pid

FW_TMP=/tmp/sam.$$.$RANDOM
NAGCMDFILEDIR=$FW_TMP
MSG_OUTGOING=$FW_TMP/msg-outgoing
mkdir -p $NAGCMDFILEDIR $MSG_OUTGOING

if [ $? -ne 0 ]; then
    echo "CRITICAL: Failed creating directories needed for Nagios:"
    echo "CRITICAL: $NAGCMDFILEDIR and $MSG_OUTGOING"
    echo "CRITICAL: Cannot proceed. Check WN TMPDIR. Bailing out."
    exit $RC_CRITICAL
fi

# Nagios debug codes/types   | FW debug levels
#    -1 = Everything            -
#     0 = Nothing               0
#     1 = Functions             3
#     2 = Configuration         3
#     4 = Process information   2,3
#     8 = Scheduled events      2,3
#    16 = Host/service checks   1,2,3
#    32 = Notifications         -
#    64 = Event broker          3
#   128 = External commands     1,2,3
#   256 = Commands (handlers)   2,3
#   512 = Scheduled downtime    -
#  1024 = Comments              -
#  2048 = Macros                1,2,3
if [ $FW_VERB -ge 3 ] ; then
    NAGDEBUGLEVEL=$(( 4095 ^ 32 ^ 512 ^ 1024 ))
elif [ $FW_VERB -eq 2 ]; then
    NAGDEBUGLEVEL=$(( 4 | 8 | 16 | 128 | 256 | 2048 ))
elif [ $FW_VERB -eq 1 ]; then
    NAGDEBUGLEVEL=$(( 16 | 128 | 2048 ))
else
    NAGDEBUGLEVEL=0
fi

# tarball with log files to go into OutputSandbox
WNLOGS=$JWD/wnlogs.tgz
touch $WNLOGS

# should be available on WNs
if [ -z $SITE_NAME ]; then
	SITE_NAME=FAKE-SITE
fi
if [ -n "$GLOBUS_CE" ]; then
	CE_NAME=$GLOBUS_CE
elif [ -n "$GLITE_CE" ]; then
	CE_NAME=$GLITE_CE
elif [ -n "$OSG_HOSTNAME" ]; then
    CE_NAME=$OSG_HOSTNAME
else
	CE_NAME=$(glite-brokerinfo getCE)
fi

rm -rf $NAGROOT
mkdir -p $NAGROOT && chmod -R 700 $NAGROOT

gunzip -f gridjob.tgz && tar xf gridjob.tar -C $NAGROOT

# gridmon and gridmonsam Python packages will become available here
export PYTHONPATH=$NAGROOT/probes:$PYTHONPATH

# 64bit Python loading 32bit libs
if [ "x${ARCH}" == "xx86_64" ] ; then
   if ( ! /usr/bin/env python -c "import _lcg_util" > /dev/null 2>&1 ) ; then
      # Try to change to 32bit Python from SW area
      PYTHON=PYTHON-2.4.6-i386
      PYPATH=$VO_OPS_SW_DIR/$PYTHON
      [ -f $PYPATH/env.sh ] && . $PYPATH/env.sh
   fi
fi
export PYTHONPATH=$PYTHONPATH:$NAGROOT/lib/python2.3/site-packages

export PERL5LIB=$PERL5LIB:$NAGROOT/usr/lib/perl5/vendor_perl/5.8.5/:$NAGROOT/usr/lib/perl5/vendor_perl/5.8.5/i386-linux-thread-multi:$NAGROOT/usr/lib64/perl5/vendor_perl/5.8.5/x86_64-linux-thread-multi

# Ensure that we'll have this info.
exec 3>&1 1>&2
echo "=== [$(date)] ==="
echo "=== Running on ==="
echo "=== Site: $SITE_NAME"
echo "=== CE: $CE_NAME"
echo "=== WN: $(hostname)"
echo "=== WN arch: ${ARCH}."
exec 1>&- 1>&3 3>&-
echo "=== [$(date)] ==="
echo "=== Running on ==="
echo "=== Site: $SITE_NAME"
echo "=== CE: $CE_NAME"
echo "=== WN: $(hostname)"
echo "=== WN arch: ${ARCH}"

if [ $MTA -eq 1 ] ; then
  if [ ! $MB_URI ] && [ "$MB_DISCOVER" == "0" ] ; then
	  echo "ATTENTION: No MB URI given and MB discovery is disabled."
	  echo "ATTENTION: Test results cannot be published from WN."
	  echo "Exiting."
	  exit $RC_UNKNOWN
  fi
fi

if [ $FW_VERB -ge 1 ] ; then
    echo "Check Python version:"
    which python
    python -V

    echo -e "Can we import Python LDAP ... "
    /usr/bin/env python -c "import ldap" && echo "YES." || echo "NO."
fi

if [ $MTA -eq 1 ]; then
    # Launch Message Transfer Agent
    _start_mta
fi

if [ $NAGIOS_RUN -eq 0 ]; then
	echo "ATTENTION: Asked not to run WN tests."
	echo "Bailing out."
	exit $RC_UNKNOWN
fi

# Set Nagios configuration
_set_nagios_config

# Launch Nagios
echo "Launching Nagios: $(date)"
if [ -z "${NAGDAEMON}" ] ; then
	# background process
	echo "Launch Nagios as background process."
    FNAGPID=`pwd`/getnag.pid
    ulimit -s 10240
    $NAGROOT/bin/nagios $NAGCONF > $FNAGPID 2>&1 &
    if [ $? -ne 0 ]; then
    	cat $FNAGPID
        echo "ERROR: Couldn't launch Nagios! (background)"
        exit $RC_UNKNOWN
    fi
    sleep 1
    _nagpid=$(grep PID $FNAGPID|sed -e 's|^.*=||; s|[^0-9]*$||')
    cat $FNAGPID
    rm -f $FNAGPID
else
    # daemon mode
	echo "Launch Nagios in daemon mode."
    ulimit -s 10240
    $NAGROOT/bin/nagios $NAGDAEMON $NAGCONF
    if [ $? -ne 0 ]; then
        echo "ERROR: Couldn't launch Nagios! (daemon)"
        exit $RC_UNKNOWN
    fi
    sleep 1
fi
# wait for status.dat to appear
_wait_nagios_statusdat

NAGTIMESTOP=$(($(date +%s) + $NAGLIFETIME))

nagpid=$($NAGSTATS -c $NAGCONF -m -d NAGIOSPID)
if [ "${nagpid}" == "0" ]; then
    nagpid=${_nagpid}
fi
echo "Nagios pid: $nagpid"

# Follow Nagios process
totalsvc=$($NAGSTATS -c $NAGCONF -m -d NUMSERVICES)
while ( `ps -p $nagpid > /dev/null` ); do
 	checkedsvc=$($NAGSTATS -c $NAGCONF -m -d NUMSVCCHECKED)
 	if [ $(date +%s) -ge $NAGTIMESTOP ] ; then
 		_echo_nagstats $totalsvc $checkedsvc
		echo "High level timeout $NAGLIFETIME sec reached. [$(( $NAGTIMESTOP - $(date +%s) ))] Killing Nagios."
		kill -9 $nagpid && killall nagios && \
			{ echo "Nagios was successfully killed."; break; }
	fi
	if [ $FW_VERB -ge 3 ] ; then
		_echo_nagstats $totalsvc $checkedsvc
	fi
	if [ $checkedsvc -ge $totalsvc ] ; then
		_echo_nagstats $totalsvc $checkedsvc
		echo "All services were checked. Killing Nagios."
		kill -15 $nagpid && \
			echo "Nagios was successfully killed."
	fi
	sleep 2
done
_package_wnlogs
date
echo "================== $(date)"

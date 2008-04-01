#!/bin/sh
#
# Common functions for the LCG data management tests.
#
# The startup procedure defined here is common for all the UI-data-lcg- tests
# and defines a number of options common for most of the test scripts
#
# Usage examples:
# DM-lcg-cr.sh lxb1921.cern.ch
# DM-lcg-cp.sh --vo cms lxb1921.cern.ch
# DM-lcg-rep.sh -v lxb1921.cern.ch lxb6130.cern.ch
#
# Author: Dmitry Zaborov <Dmitry.Zaborov@cern.ch>
# Version: $Id$

# ... Command line options and parameters

function usage() {

  echo ""
  echo "Tests of lcg_util"
  echo ""
  echo "Command-line parameters: "
  echo '$1                                 first SE hostname (mandatory unless VO_<VO>_DEFAULT_SE is defined)'
  echo '$2                                 second SE hostname (for lcg-rep only)'
  echo ""
  echo "Command-line options: "
  echo "-v                                 verbose mode (-v will be passes to lcg- commands)"
  echo "-t <timeout>                       in seconds"
  echo "-n <nbstreams>                     number of streams (for lcg-cr,-cp,-rep)"
  echo "--vo <vo>                          VO to be fed to lcg- commands. If not given, and LCG_GFAL_VO not defined an attempt is made to determine VO name using voms-proxy-info"
  echo "--usefullsurl                      use full SURLs whenever possible"
  echo "--nobdii                           use --nobdii whenever possible (implies --usefullsurl)"
  echo "--defaultsetype <default_se_type>  [srmv1|srmv2|se|none]"
  echo "--setype <se_type>                 [srmv1|srmv2|se|none] - for lcg-cr/rf/del/ls/gt"
  echo "--srcsetype <source_se_type>       [srmv1|srmv2|se|none] - for lcg-cp, lcg-rep"
  echo "--dstsetype <destination_se_type>  [srmv1|srmv2|se|none] - for lcg-cp, lcg-rep"
  echo "--st <space_token>                 for lcg-cr, lcg-gt, lcg-getturls, lcg-sd"
  echo "--sst <source_space_token>         for lcg-cp, lcg-rep"
  echo "--dst <destination_space_token>    for lcg-cp, lcg-rep"
  echo ""
  echo "Certain tests may ignore certain options"
  echo ""
  exit $1
}

# ... prefixed echo

function myecho() {
  echo " ===> $@"
}

# ... Run command succussfuly or exit with cleanup

function run_command() {

  echo ""
  echo " $" $@

  OUTPUT=$(eval $@ 2>&1)
  if [ $? -gt 0 ]; then
    echo "${OUTPUT}"
    myecho "$1 failed"
    echo ""
    myexit 1 $1
  fi

  echo "${OUTPUT}"
  
  return 0
}

# ... Common startup procedure

function lcg_test_startup() {

  VO_OPTIONS=""
  SE_HOST=""
  GUID=""
  VERBOSE=""
  OPT_DEFAULTSETYPE=""
  OPT_SRCSETYPE=""
  OPT_DSTSETYPE=""
  OPT_SETYPE=""
  OPT_TOKEN=""
  OPT_DSTTOKEN=""
  OPT_SRCTOKEN=""
  OPT_TIMEOUT=""
  OPT_NBSTREAMS=""
  OPT_NO_BDII=""
  USE_FULL_SURL=""
  
  # ... Parse options
  #     Quotation is disabled to allow constructions like "--vo $2" to work correctly
  #     But beware you can not pass a vo name containing spaces or other special characters because of that

  OPTS=`getopt --unquoted --longoptions "help::,vo:,defaultsetype:,srcsetype:,dstsetype:,setype:,st:,sst:,dst:,nobdii,usefullsurl" --options "vt:n:" -- "$@"` || usage 1

  set -- $OPTS
  
  while [ ! -z "$1" ]
  do
    case "$1" in

      --help) usage 0
          ;;
      -v) myecho "verbose flag set in command line"
          VERBOSE="-v"
          shift
          ;;
      --vo) myecho "VO given in command line: $2"
            VO_OPTIONS="--vo $2"
            myecho "Will use VO Options:" $VO_OPTIONS
            shift
	    shift 
            ;;
      -t) myecho "timeout time set to $2 seconds"
          OPT_TIMEOUT="-t $2"
          shift
	  shift
          ;;
      -n) myecho "number of streams in file transfers set to $2"
          OPT_NBSTREAMS="-n $2"
          shift
	  shift
          ;;
      --usefullsurl) myecho "Full SURLs will be used whenever possible"
	  USE_FULL_SURL="yes"
          shift
          ;;
      --nobdii) myecho "--nobdii option will be passed to commands sensitive to se type"
          myecho "Full SURLs will be used whenever possible"
          OPT_NO_BDII="--nobdii"
	  USE_FULL_SURL="yes"
          shift
          ;;
      --defaultsetype | -D) myecho "default se type set in command line: $2"
          OPT_DEFAULTSETYPE="--defaultsetype $2"
          shift
          shift
          ;;
      --srcsetype) myecho "se type to use with lcg-cp, lcg-rep for source file: $2"
          OPT_SRCSETYPE="--srcsetype $2"
          shift
          shift
          ;;
      --dstsetype) myecho "se type to use with lcg-cp, lcg-rep for destination file: $2"
          OPT_DSTSETYPE="--dstsetype $2"
          shift
          shift
          ;;
      --setype) myecho "se type to use with lcg-cr, lcg-del, lcg-gt, lcg-getturls, lcg-ls, lcg-rf: $2"
          OPT_SETYPE="--setype $2"
          shift
          shift
          ;;
      --sst) myecho "space token to use with source file (lcg-cp, lcg-rep)       : $2"
          OPT_SRCTOKEN="--sst $2"
          shift
          shift
          ;;
      --dst) myecho "space token to use with destination file (lcg-cp, lcg-rep)  : $2"
          OPT_DSTTOKEN="--dst $2"
          shift
          shift
          ;;
      --st) myecho "space token to use with lcg-cr, lcg-gt, lcg-getturls, lcg-sd: $2"
          OPT_TOKEN="--st $2"
          shift
          shift
          ;;
      --) shift ; break ;;
      *) echo "Error: Invalid Argument $1" 1>&2; usage; exit 1 ;;
    esac
  done


  # ... define SE host

  if [ -n "$1" ]; then
    SE_HOST=$1
    myecho "SE host      : $SE_HOST"
  else
    DEFAULT_SE=`printenv VO_${VO}_DEFAULT_SE`
    if [ -n "$DEFAULT_SE" ]; then
      myecho "SE defined in VO_${VO}_DEFAULT_SE: $DEFAULT_SE"
    else
      myecho "Please supply SE hostname in command line"
      usage 1
    fi
    usage 1
  fi

  # ... define second SE host

  if [ -n "$2" ]; then
    SE2=$2
    myecho "second storage element (for lcg-rep): $SE2"
  fi
  
  if [ "$SE_HOST" == "$SE2" ]; then
    myecho "Please provide _different_ names for the two storage elements !"
    usage 1
  fi
  
  # ... print important environment variables

  myecho "[for information]: LCG_GFAL_INFOSYS=$LCG_GFAL_INFOSYS"
  myecho "[for information]: LCG_GFAL_VO=$LCG_GFAL_VO"
  myecho "[for information]: LFC_HOST=$LFC_HOST"
  
  # .. special stuff for --nobdii & --usefullsurls

  if [ -n "$USE_FULL_SURL" ]; then
    define_endpoint SE_ENDPOINT $SE_HOST
    define_endpoint SE2_ENDPOINT $SE2
  fi

  OPT_NO_BDII_SE=$OPT_NO_BDII
  OPT_NO_BDII_LCG_CP=$OPT_NO_BDII
  OPT_NO_BDII_LCG_REP=$OPT_NO_BDII

  if [ -n "$OPT_NO_BDII" ]; then
    define_setypes
  fi

  # ... define options for every lcg- command

  LCG_CR_OPTIONS_BASIC="$VERBOSE $VO_OPTIONS $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE $OPT_TOKEN $OPT_NBSTREAMS $OPT_NO_BDII_SE"
  LCG_CR_OPTIONS_BDII="$VERBOSE $VO_OPTIONS $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE $OPT_TOKEN $OPT_NBSTREAMS -d $SE_HOST"
  LCG_CP_OPTIONS_BDII="$VERBOSE $VO_OPTIONS $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SRCSETYPE $OPT_DSTSETYPE $OPT_SRCTOKEN $OPT_DSTTOKEN $OPT_NBSTREAMS"
  LCG_REP_OPTIONS_BDII="$VERBOSE $VO_OPTIONS $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SRCSETYPE $OPT_DSTSETYPE $OPT_SRCTOKEN $OPT_DSTTOKEN $OPT_NBSTREAMS"
  LCG_DEL_OPTIONS_BDII="$VERBOSE $VO_OPTIONS $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE"

  LCG_LS_OPTIONS="$VERBOSE $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE $OPT_NO_BDII_SE"
  LCG_RF_OPTIONS="$VERBOSE $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE $OPT_NO_BDII_SE $VO_OPTIONS"
  LCG_GT_OPTIONS="$VERBOSE $OPT_TIMEOUT $OPT_DEFAULTSETYPE $OPT_SETYPE $OPT_NO_BDII_SE $OPT_TOKEN"
  LCG_SD_OPTIONS="$VERBOSE $OPT_TIMEOUT $OPT_TOKEN"
  LCG_UF_OPTIONS="$VERBOSE $VO_OPTIONS"

  LCG_CR_OPTIONS="$LCG_CR_OPTIONS_BDII $OPT_NO_BDII_SE"
  LCG_CP_OPTIONS="$LCG_CP_OPTIONS_BDII $OPT_NO_BDII_LCG_CP"
  LCG_REP_OPTIONS="$LCG_REP_OPTIONS_BDII $OPT_NO_BDII_LCG_REP"
  LCG_DEL_OPTIONS="$LCG_DEL_OPTIONS_BDII $OPT_NO_BDII_SE"

  LCG_GETTURLS_OPTIONS="$LCG_GT_OPTIONS"

#  myecho "lcg-cr options: $LCG_CR_OPTIONS"
#  myecho "lcg-cp options: $LCG_CP_OPTIONS"
#  myecho "lcg-rep options: $LCG_REP_OPTIONS"

  myecho "start time: " `date`

  # ... Create temporary file
    
  LOCAL_FILE=`mktemp -p /tmp ui.test.tmp.file.XXXXXXXXXXX`
  LOCAL_FILE_NAME=`basename $LOCAL_FILE`
  LOCAL_FILE_URI=file:$LOCAL_FILE

  echo "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" > $LOCAL_FILE || myexit 1
  echo "H test file created by `id -un` on `date`                                    " >> $LOCAL_FILE || myexit 1
  echo "H this is a temporary file created for `basename $0` test                            " >> $LOCAL_FILE || myexit 1
  echo "H Please remove this file if it is more than a few hours old but still resides on an SE " >> $LOCAL_FILE || myexit 1
  echo "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" >> $LOCAL_FILE || myexit 1
  echo ""
  echo " ---    $LOCAL_FILE     --- "
  cat $LOCAL_FILE || myexit 1
  LOCAL_FILE_CREATED="yes"
  echo " --- end of $LOCAL_FILE --- "
}

# ... Find first text line containing guid: , select last word
function extract_guid() {
  export $1=`grep -m 1 -E "^ *guid:" <<<"$2" | awk -F ' ' '/guid:/ {print $NF}'` # (<start_of_newline><any_number_of_spaces>guid:)
  myecho "$1 :" `printenv $1`
}

# ... Find first text line containing lfn: , select last word
function extract_lfn() {
  export $1=`awk -F ' ' '/lfn:/ {print $NF; exit}' <<<"$2"`
  myecho "$1  :" `printenv $1`
}

# ... Find first text line containing srm: or sfn: , select last word
function extract_surl() {
  export $1=`awk -F ' ' '/srm:/ {print $NF; exit} /sfn:/ {print $NF; exit}' <<<"$2"`
  myecho "$1 :" `printenv $1`
}

# ... Find first text line containing gsiftp: , select last word
function extract_gsiftp_turl() {

  TMPTURL=`grep -m 1 -E "^ *gsiftp:" <<<"$2" | awk -F ' ' '/gsiftp:/ {print $NF}'`

  if [ -z "$TMPTURL" ]; then
    myecho "Could not determine valid gsiftp TURL !"
    myexit 1
  fi

  export $1=$TMPTURL
  myecho "$1 :" `printenv $1`
}

# ... Find first text line containing gsiftp: , select NEXT line
function extract_reqid() {

  TMPREQID=`awk -F ' ' '/gsiftp:/ {getline; print $(NF)}' <<<"$2"`
  if [ -z "$TMPREQID" ]; then
    myecho "Could not determine ReqId/Token !"
    myexit 1
  fi
#  myecho "request id / token is $TMPREQID"
  export $1=$TMPREQID
  myecho "$1 :" `printenv $1`

}

# .. define endpoints for the SE hosts (to be used with --nobdii of --usefullsurls)

function define_endpoint() {

    SEx_HOST=$2
    if [ -z "$SEx_HOST" ]; then
      return 1
    fi
    myecho "Trying to determine endpoint for $SEx_HOST"
    # ldapsearch is not case-sensitive?
    for SERVICE_TYPE in srm srm_v2 srm_v1 ; do
      OUTPUT=`ldapsearch -z 2 -x -H ldap://$LCG_GFAL_INFOSYS -b "mds-vo-name=local, o=grid" "(&(GlueServiceURI=*//${SEx_HOST}*)(GlueServiceType=${SERVICE_TYPE}))" GlueServiceEndpoint`
      ENDPOINT=`awk -F ' ' '/GlueServiceEndpoint:/ {print $NF}' <<<"$OUTPUT"`
      if [ -n "$ENDPOINT" ]; then
        break
      fi
    done
    myecho "SE endpoint : $ENDPOINT"

    SEx_ENDPOINT=`sed s/httpg/srm/ <<<$ENDPOINT`
    myecho "SRM endpoint:   ${SEx_ENDPOINT}"

    if [ -z "${SEx_ENDPOINT}" ]; then
       myecho "Could not determine srm endpoint for ${SEx_HOST}"
       exit 1
    fi

   export $1=${SEx_ENDPOINT}
}

# ...  Define SE types if still undefined (to be used with --nobdii)

function define_setypes() {

  APPARENT_SE_TYPE=se
  if grep managerv2 <<<${SE_ENDPOINT} >/dev/null; then
     APPARENT_SE_TYPE=srmv2
  elif grep managerv1 <<<${SE_ENDPOINT} >/dev/null; then
     APPARENT_SE_TYPE=srmv1
  fi
    
  if [ -z "$OPT_SETYPE" ]; then
     OPT_NO_BDII_SE="$OPT_NO_BDII --setype ${APPARENT_SE_TYPE}"
     myecho "setype not given for ${SE_HOST} - will use ${APPARENT_SE_TYPE}"
  fi
  if [ -z "$OPT_SRCSETYPE" ]; then
     OPT_NO_BDII_LCG_CP="$OPT_NO_BDII --srcsetype ${APPARENT_SE_TYPE} --dstsetype ${APPARENT_SE_TYPE}"
     OPT_NO_BDII_LCG_REP="$OPT_NO_BDII --srcsetype ${APPARENT_SE_TYPE}"
     #myecho "setype not given for ${SE_HOST} - will use ${APPARENT_SE_TYPE}"
  fi

  if [ -n "${SE2_ENDPOINT}" ]; then
    APPARENT_SE_TYPE=se
    if grep managerv2 <<<${SE2_ENDPOINT} >/dev/null; then
       APPARENT_SE_TYPE=srmv2
    elif grep managerv1 <<<${SE2_ENDPOINT} >/dev/null; then
       APPARENT_SE_TYPE=srmv1
    fi

    if [ -z "$OPT_DSTSETYPE" ] && [ -n "$SE2" ]; then
       OPT_NO_BDII_LCG_REP="$OPT_NO_BDII_LCG_REP --dstsetype ${APPARENT_SE_TYPE}"
       myecho "setype not given for $SE2 - will use ${APPARENT_SE_TYPE}"
    fi
  fi
}

# ... Replace short SURL by a full one if USE_FULL_SURL flag is set

function convert_to_full_surl() {

  if [ -n "$USE_FULL_SURL" ]; then

    myecho "Converting to full SURL ..."
    VARNAME=$1
    export $VARNAME        # export is needed for printenv to see this variable
    #myecho "converting $VARNAME"
    TMPSURL=`printenv $VARNAME`
    TMPHOST=""
    if grep ${SE_HOST} <<<"${TMPSURL}" >/dev/null; then
      TMPHOST=${SE_HOST}
      SURL_PATH=`sed "s/srm:\/\/${SE_HOST}//" <<<"${TMPSURL}"`
      TMPSURL="${SE_ENDPOINT}?SFN=${SURL_PATH}"
    elif grep "${SE2}" <<<"${TMPSURL}" >/dev/null; then
      TMPHOST=$SE2
      SURL_PATH=`sed "s/srm:\/\/${SE2}//" <<<"${TMPSURL}"`
      TMPSURL="${SE2_ENDPOINT}?SFN=${SURL_PATH}"
    else
      myecho "Dont know this host"
      return 1
    fi

    export $1=$TMPSURL
    myecho "$1 :" `printenv $1`

  fi
}

# ... Cleanup. Remove the GRID file(s) if needed (GUID must point to the right file!), remove local temporary file.

function cleanup() {

  echo ""
  myecho "cleaning up ... "
  
  RETCODE=0
  
  if [ "$2" == "lcg-uf" ]; then
    myecho "Trying to recover the file for cleanup purpose"
    echo ""; echo " $ lcg-rf $LCG_RF_OPTIONS -g $GUID $SURL"
    lcg-rf $LCG_RF_OPTIONS -g $GUID $SURL
  fi

  if [ -n "$GUID" ] ; then

    echo ""; echo " $ lcg-del $VERBOSE $VO_OPTIONS -a $GUID"
    lcg-del $VERBOSE $VO_OPTIONS -a $GUID

    if [ $? -ne 0 ]; then
      myecho "lcg-del failed"
      RETCODE=1
    fi
  fi

  if [ -n "$SURL2" ] ; then

    echo ""; echo " $ lcg-del $VERBOSE $VO_OPTIONS $SURL2"
    lcg-del $VERBOSE $VO_OPTIONS $SURL2

    if [ $? -ne 0 ]; then
      myecho "lcg-del failed"
      RETCODE=1
    fi
  fi

  if [ -f "$LOCAL_FILE" ]; then
    #myecho "Removing $LOCAL_FILE"
    rm -f $LOCAL_FILE
  fi

  return $RETCODE
}

# ... Exit with cleanup

function myexit() {

  cleanup $@

  echo ""
  myecho "end time: " `date`
  echo ""

  if [ $? -ne 0 ] || [ "$1" != "0" ]; then

    #echo " *** Make sure you have a valid [voms] proxy and correct SE is given *** "
    echo " *** test NOT passed *** "
    if [ "$1" != "0" ]; then
      echo " *** failed command: $2 *** "
    fi
    exit 1
  fi
 
  echo "    === test PASSED === "
  exit 0
}

# library of shell functions used by the sensor

# print summary and exit with given code
exit_with_summary() {
    local ret=$1
    shift
    local summary="$*"

    echo "summary: $summary"
    exit $ret
}

# prints the grid jobid
jobid() {
    local jobid

    if [ "$GLITE_WMS_JOBID" ]; then
        jobid=$GLITE_WMS_JOBID
    elif [ "$EDG_WL_JOBID" ]; then
        jobid=$EDG_WL_JOBID
    elif [ "$GLOBUS_GRAM_JOB_CONTACT" ]; then
        jobid=$GLOBUS_GRAM_JOB_CONTACT
    fi

    echo $jobid
}

# prints the current VO
vo() {
    local VO

    VO=`voms-proxy-info -vo 2>/dev/null`
    test "$VO" || VO=`id -n -g`

    # use only the first VO (multiple VOMS fqan are possible in cert)
    echo "$VO" | head -1
}

# print the CE name from the environment
ce() {
    local CE

    if [ "$GLITE_CE" ]; then
        CE=$GLITE_CE
    elif [ "$GLOBUS_CE" ]; then
        CE=$GLOBUS_CE
    fi

    echo $CE
}

# prints the free space in MB for given directory
free_space() {
    local dir=$1
    local blocks_free=`stat -f $dir -c %a`
    local block_size=`stat -f $dir -c %s`

    echo $[$blocks_free*$block_size/1024/1024]
}

# prints the current UTC time in seconds since epoch
now() {
    date -u +%s
}

# convert same return code to name
same_id2name() {
    local id=$1
    local name=$id # return back the id if not recognized
    case "$id" in
        $SAME_OK)       name=OK ;;
        $SAME_INFO)     name=INFO ;;
        $SAME_NOTICE)   name=NOTICE ;;
        $SAME_WARNING)  name=WARNING ;;
        $SAME_ERROR)    name=ERROR ;;
        $SAME_CRITICAL) name=CRITICAL ;;
        $SAME_MAINTENANCE)  name=MAINTENANCE ;;
    esac
    echo $name
}

# convert status name to code
same_name2id() {
    local name=$1
    local id=$name # default
    case "$name" in
        OK)     id=$SAME_OK ;;
        INFO)   id=$SAME_INFO ;;
        NOTICE) id=$SAME_NOTICE ;;
        WARNING)id=$SAME_WARNING ;;
        ERROR)    id=$SAME_ERROR ;;
        CRITICAL) id=$SAME_CRITICAL ;;
        MAINTENANCE) id=$SAME_MAINTENANCE ;;
    esac
    echo $id
}

# get default se for given vo
get_default_se() {
    local VO DEFAULT_SE

    VO=`echo $1 | tr '[a-z]' '[A-Z]'`
    DEFAULT_SE=`eval echo \\\$VO_${VO}_DEFAULT_SE`

    if [ "$DEFAULT_SE" ]; then
        echo $DEFAULT_SE
        return 0
    else
        return 1
    fi
}

# digitally sign a file
sign() {
    if [ $# -ne 2 ]; then
        echo usage: sign infile outfile
        return 1
    fi

    local IN=$1
    local OUT=$2

    local CERT=${X509_USER_CERT:-~/.globus/usercert.pem}
    local KEY=${X509_USER_KEY:-~/.globus/userkey.pem}

    if [ ! -r $IN ]; then
        echo $IN not readable
        return 1
    fi

    if [ ! -r $CERT ]; then
        echo certificate not readable
        return 1
    fi

    if [ ! -r $KEY ]; then
        echo private key not readable
        return 1
    fi

    # base64 encode for smime
    local B64=`mktemp`
    openssl base64 -in $1 -out $B64

    # smime sign
    openssl smime -sign -signer $CERT -inkey $KEY -in $B64 -out $OUT

    rm $B64
}

# verify and extract a digitally signed file
verify() {
    local usage="usage: verify -a auth_ids -i input -o output [-d CAdir]"

    # parse options
    local TEMP
    if ! TEMP=`getopt -o "a:i:o:d:" -- "$@"`; then
        echo $usage
        return 1
    fi

    local AUTZ_FILE IN OUT
    local CERT_DIR=${X509_CERT_DIR:-/etc/grid-security/certificates}
    eval set -- "$TEMP"
    while true; do
        case "$1" in
            -a) AUTZ_FILE=$2; shift 2;;
            -d) CERT_DIR=$2; shift 2;;
            -i) IN=$2; shift 2;;
            -o) OUT=$2; shift 2;;
            --) shift; break;;
            *) echo unknown option "'$1'"; usage; return 1
        esac
    done

    # check values
    if [ ! "$AUTZ_FILE" ]; then
        echo authorization file not defined
        echo $usage
        return 1
    fi

    if [ ! "$IN" ]; then
        echo input file not defined
        echo $usage
        return 1
    fi

    if [ ! "$OUT" ]; then
        echo output file not defined
        echo $usage
        return 1
    fi

    if [ ! -r $IN ]; then
        echo input file not readable
        return 1
    fi

    if [ ! -d "$CERT_DIR" ]; then
        echo ca directory not accessible
        return 1
    fi

    # check signature and cert
    local CERT=`mktemp verify_cert.XXXXXX`
    local OUT_B64=`mktemp verify_b64.XXXXXX`
    if ! openssl smime -verify -CApath $CERT_DIR -signer $CERT \
         -in $IN -out $OUT_B64 &>/dev/null; then
        echo signature verification failed
        return 1
    fi

    # check authorization
    local SUBJ=`openssl x509 -subject -noout -in $CERT | sed 's/^subject= *//'`
    local AUTHORIZED="" DN
    while read DN; do
        if [ "$SUBJ" == "$DN" ]; then
            AUTHORIZED="yes"
            break
        fi
    done < $AUTZ_FILE

    if [ ! "$AUTHORIZED" ]; then
        echo authorization failed for $SUBJ
        return 1
    fi

    # decode file
    openssl base64 -d -in $OUT_B64 -out $OUT

    rm $CERT $OUT_B64
}


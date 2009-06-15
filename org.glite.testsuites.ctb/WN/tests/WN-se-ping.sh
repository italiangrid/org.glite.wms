#! /bin/sh

. lib.sh

SAME_OK=0
SAME_ERROR=1

# test configuration
if [ ! "$LCG_GFAL_INFOSYS" ]; then
    echo "LCG_GFAL_INFOSYS not set"
    exit 1
fi

# define search target
VO=`vo`
SE=`get_default_se $VO` || SE=`get_default_se ops`
[ "$SE" ] || exit_with_summary $SAME_ERROR "could not find good SE"

# do the search
SRM_URI=`ldapsearch -H ldap://$LCG_GFAL_INFOSYS -x -b o=grid -LLL \
            "(&(objectclass=GlueService)(GlueServiceType=srm_*)
               (GlueServiceURI=*/$SE:*))" GlueServiceURI | \
         sed -n '/^GlueServiceURI: */s///p'`

# check results
if [ "$SRM_URI" ]; then # use srm
    SRM_HOST=${SRM_URI#*://}
    SRM_HOST=${SRM_HOST%%/*}
    SRM_HOST=${SRM_HOST%%:*}
    if type glite-srm-ping &>/dev/null; then
        summary="$SRM_HOST using srm ping"
        if glite-srm-ping -s $SRM_URI; then
            exit_with_summary $SAME_OK $summary
        else
            exit_with_summary $SAME_ERROR $summary
        fi
    else
        # TODO: try srmcp
        exit_with_summary $SAME_WARNING \
            "could not test srm: no clients available"
    fi
else # use gsiftp
    if type glite-gridftp-exists &>/dev/null; then
        summary="$SE using gsiftp ping (glite flavor)"
        if glite-gridftp-exists gsiftp://$SE/; then
            exit_with_summary $SAME_OK $summary
        else
            exit_with_summary $SAME_ERROR $summary
        fi
    elif type edg-gridftp-exists &>/dev/null; then
        summary="$SE using gsiftp ping (edg flavor)"
        if edg-gridftp-exists gsiftp://$SE/; then
            exit_with_summary $SAME_OK $summary
        else
            exit_with_summary $SAME_ERROR $summary
        fi
    else
        exit_with_summary $SAME_WARNING \
            "could not test gsiftp: no clients available"
    fi
fi


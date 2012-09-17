#!/bin/sh

SAME_OK=0
SAME_ERROR=1

. lib.sh

STAGE2=$VO_OPS_SW_DIR/same_stage2.tgz.smime
STAGE2_DIR=$1

# check for possible problems
if [ -z "$VO_OPS_SW_DIR" ]; then
    exit_with_summary $SAME_ERROR "VO_OPS_SW_DIR not set"
elif [ ! -d $VO_OPS_SW_DIR ]; then
    exit_with_summary $SAME_ERROR "$VO_OPS_SW_DIR not a directory"
elif [ ! -r $VO_OPS_SW_DIR -a -x $VO_OPS_SW_DIR ]; then
    exit_with_summary $SAME_ERROR "$VO_OPS_SW_DIR not accessible"
elif [ ! -e $STAGE2 ]; then
    exit_with_summary $SAME_WARNING "stage2 not found"
elif [ ! -r $STAGE2 ]; then
    exit_with_summary $SAME_WARNING "stage2 not accessible"
fi

# verify
STAGE2_TGZ=`mktemp stage2.XXXXXX`
if ! verify -a $SAME_SENSOR_HOME/authorized_dns -i $STAGE2 -o $STAGE2_TGZ; then
    exit_with_summary $SAME_WARNING "verification failed"
fi

# extract
if ! (mkdir -p $STAGE2_DIR && tar xzf $STAGE2_TGZ -C $STAGE2_DIR); then
    exit_with_summary $SAME_WARNING "extraction failed"
fi

# clean
rm -f $STAGE2_TGZ

# all good
exit_with_summary $SAME_OK "extracted ok"


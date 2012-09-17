#!/bin/bash

NUM_OF_ITERATIONS=${1:-2};
NUM_OF_JOBS=${2:-10};		#Change NUM_OF_JOBS to make the stress tests 
SLEEP_TIME=${3:-604800};
USER="cesga025";
QUEUE="cesga"
FILE_OUT=`mktemp $HOME/qsub_stress_out_XXXXXXXX`
FILE_ERR=`mktemp $HOME/qsub_stress_err_XXXXXXXX`


exec 1<>$FILE_OUT
exec 2<>$FILE_ERR

submit_job () {
   BS_JOB_ID="";
   #BS_JOB_ID=`su - $USER -c "echo sleep $SLEEP_TIME | qsub -q $QUEUE -o $TMPDIR/tmpout -e $TMPDIR/tmperr"`
   BS_JOB_ID=`echo sleep $SLEEP_TIME | qsub -q $QUEUE -o $HOME/stdout -e $HOME/stderr | cut -d" " -f3`
}

jobs_iteration () {
        for JOB_NUM in `seq 1 $NUM_OF_JOBS`;do
                echo -e "0\t$INTER_NUM\t$JOB_NUM"
                submit_job && JOBS_ARRAY[JOB_NUM]=$BS_JOB_ID;
        done

        for JOB_NUM in `seq 1 $NUM_OF_JOBS`;do
                echo -e "1\t$INTER_NUM\t$JOB_NUM\t${JOBS_ARRAY[JOB_NUM]}"
                qdel ${JOBS_ARRAY[JOB_NUM]}
        done
}

for ITER_NUM in `seq 1 $NUM_OF_ITERATIONS`;do
        jobs_iteration &
        # this wait a hour and it returns to send the jobs + 1000
        sleep 3600
        NUM_OF_JOBS=`echo $NUM_OF_JOBS + 1000 | bc`
done

wait

mv $FILE_OUT qsub_stress_`date -Iminutes`_"$NUM_OF_ITERATIONS"_"$NUM_OF_JOBS".out
mv $FILE_ERR qsub_stress_`date -Iminutes`_"$NUM_OF_ITERATIONS"_"$NUM_OF_JOBS".err

exit 0;

#!/bin/sh

# --> All jobs run in high priority to execute before others, since they are
# simple but important. High priority is represented by high value!

USER="esfreire"
GROUP="cesga"
QUEUE="cesga"
HOWMANY="$1"
TOTAL=`echo $HOWMANY - 1 | bc` # Because if we execute the script with 200 the qsub only execute 199
JOB='cd $HOME;date;hostname;whoami;pwd';

if [ -z $HOWMANY ];then
        exit 2
fi

trap "exit 2" 1 2 3 15
# Create a unique temporary directory, else exit with error
TMPDIR=`mktemp -d $HOME/qsub_test-XXXXXXXX` || {
     echo Could not create temporary directory in /home/esfreire/ ; exit 1; }
# Clean up the TMPDIR always before leaving the script
trap "rm -rf $TMPDIR" 0



JOBIDS="$TMPDIR/jobids"
rm -f $JOBIDS

echo "START TIME: `date -Iminutes`"

# Then check for succesful execution of HOWMANY jobs, meaning all of them
echo Submitting $HOWMANY simple jobs...
for JOBNO in `seq 1 $HOWMANY`
do
        JOBDIR="$TMPDIR/$JOBNO"
        mkdir $JOBDIR
        cd $JOBDIR 
        echo $JOB | qsub -p 1024 >> $JOBIDS
done

# Wait for all jobs to complete, or be removed somehow from queue. 
# WARNING: this could wait for too much so this script should be supervised,
# either by the user or by another script
echo Waiting all jobs to end...
COMPLETED=`qstat -s z -u $USER | awk 'BEGIN { cont=0 }{ cont++; if ( cont > 3 ) print $1}' | wc -l`
while [ $COMPLETED != $TOTAL ]
do
        sleep 30
        COMPLETED=`qstat -s z -u $USER | awk 'BEGIN { cont=0 }{ cont++; if ( cont > 3 ) print $1}' | wc -l`
        echo "$COMPLETED jobs completed..."
done
echo "DONE!"
# Now that we know the jobs are done, check stdout and stderr of all jobs
echo Checking successful execution...
echo A simple dot means job success, an exclamation mark is failure!
FAIL=no
for JOBNO in `seq 1 $HOWMANY`
do
        JOBOUT="$TMPDIR/$JOBNO/STDIN.o*"
        JOBERR="$TMPDIR/$JOBNO/STDIN.e*"
        if [ `cat $JOBERR 2>/dev/null | wc -l` != 0 -o `cat $JOBOUT 2>/dev/null | wc -l` == 0 ]
        then
                echo -n !
                FAIL=yes
        else
                echo -n .
        fi
done
echo

echo "END TIME: `date -Iminutes`"

[ $FAIL == yes ] && exit 1

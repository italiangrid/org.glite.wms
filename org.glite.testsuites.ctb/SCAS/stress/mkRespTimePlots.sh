#!/bin/sh

function usage() {
echo "Usage ./mkRespTimePlots  [<log_location>] [<datafile>]"
echo "   <log_location> a directory were *_data_norm files are stored"
echo "   <datafile> a file with time,resptime data"
}

function createPlot() {

absdatafile=$1

#check the file
if [ ! -f $absdatafile ]; then
  echo "$absdatafile is not a valid file"
  break
fi
#cp -f $absdatafile ./
datafile=`basename $absdatafile`
datafile_noext=`echo $datafile | cut -d'.' -f1`
gnuplotfile=$datafile_noext.plt
echo "Creating $gnuplotfile file"
cat <<EOF > $gnuplotfile

set title "Response Time"
set xlabel "Time (sec from Unix epoch)"
set ylabel "Response time (sec)"
set autoscale
set datafile separator ","
set terminal postscript enhanced color
set output "$datafile_noext.ps"
plot "$datafile" using 1:2 title "$datafile"
EOF

gnuplot $gnuplotfile
if [ $? -ne 0 ]; then
  echo "Error in gnuplot call"
 break
else
 echo "$datafile_noext.ps ready"
  fi
}

rm -f ./*_data


if [ $# -eq 0 ]; then
  echo "You have to provide at least one datafile or one directory"
  usage
  exit 1
fi

for argument in "$@"
do

  if [ -d $argument ]; then
    echo "working on $argument directory"
    logdir=$argument
    pushd $logdir
    for file in `ls *_data_norm`
    do
      createPlot $file
    done
  elif [ -f $argument ];then
    file=$argument
    createPlot $file
  else
    echo "$argument is not a valid directory nor a valid file"
  fi
done
        
exit 0

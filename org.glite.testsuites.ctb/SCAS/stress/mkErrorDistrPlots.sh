#!/bin/sh

function usage() {
echo "Usage $0  [<log_location>] [<datafile>]"
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
datafile=`basename $absdatafile`
datafile_noext=`echo $datafile | cut -d'.' -f1`
gnuplotfile=${datafile_noext}_error.plt
echo "Creating $gnuplotfile file"
cat <<EOF > $gnuplotfile

set title "Error distribution"
set xlabel "Time (sec)"
set xtics autofreq
set ytics ("no error" 0, "error" 1)
set datafile separator ","
set terminal postscript landscape enhanced color
set output "${datafile_noext}_error.ps"
plot "$datafile" using 1:2 title "$datafile" with points 1
EOF

gnuplot $gnuplotfile
if [ $? -ne 0 ]; then
  echo "Error in gnuplot call"
 break
else
 echo "${datafile_noext}_error.ps ready"
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
    for file in `ls *_error_norm`
    do
      createPlot $file
    done
    popd
  elif [ -f $argument ];then
    file=$argument
    createPlot $file
  else
    echo "$argument is not a valid directory nor a valid file"
  fi
done
        
exit 0

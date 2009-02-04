#!/bin/sh

function usage() {
echo "Usage ./mkRespTimePlots <datafile> [...<datafile>]"
echo "   <datafile> a file with time,resptime data"
}

if [ $# -eq 0 ]; then
  echo "You have to provide at least one datafile"
  usage
  exit 1
fi

for absdatafile in "$@"
do

#check the file
  if [ -f absdatafile ]; then
    echo "$absdatafile is not a valid file"
    break
  fi
  cp -f $absdatafile ./
  datafile=`basename $absdatafile`
  datafile_noext=`echo $datafile | cut -d'.' -f1`
  gnuplotfile=$datafile_noext.plt
  echo "Creating $gnuplotfile file"
  cat <<EOF > $gnuplotfile

set title "Response Time"
set xlabel "Time (sec from Unix epoch)"
set ylabel "Response time (sec)"
#set key title "$datafile"
set autoscale
set datafile separator ","
set terminal postscript enhanced color
set output "$datafile_noext.ps"
plot "$datafile" using 1:2
EOF
  gnuplot $gnuplotfile
  if [ $? -ne 0 ]; then
    echo "Error in gnuplot call"
   break
  else
    echo "$datafile_noext.ps ready"
  fi
done

rm -f ./*_data

exit 0

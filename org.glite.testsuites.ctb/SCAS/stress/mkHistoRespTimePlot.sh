#!/bin/sh

function usage() {
echo "Usage ./mkRespTimePlots  <datafile>"
echo "   <datafile> a file with (interval end,frequency) data"
}

function createPlot() {

absdatafile=$1

#check the file
if [ ! -f $absdatafile ]; then
  echo "$absdatafile is not a valid file"
  break
fi
cp -f $absdatafile ./
datafile=`basename $absdatafile`
datafile_noext=`echo $datafile | cut -d'.' -f1`
gnuplotfile=$datafile_noext.plt
echo "Creating $gnuplotfile file"
cat <<EOF > $gnuplotfile

set title "Response Time Frequency Histogram"
set xlabel "Time (sec)"
set ylabel "Response time (sec)"
set autoscale
set style data boxes
set style fill solid
set xtics 1
set terminal png
set output "histo_${datafile_noext}.png"
unset key
plot "$datafile"
EOF

gnuplot $gnuplotfile
if [ $? -ne 0 ]; then
  echo "Error in gnuplot call"
 break
else
 echo "histo_$datafile_noext.png ready"
  fi
}

rm -f ./*_data


if [ $# -eq 0 ]; then
  echo "You have to provide at least one datafile or one directory"
  usage
  exit 1
fi

if [ -f $1 ];then
  file=$1
  createPlot $file
else
  echo "$argument is not a valid directory nor a valid file"
fi

exit 0

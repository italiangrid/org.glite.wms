#!/bin/sh

function usage() {
echo "Usage ./mkMemConsPlots <datafile>"
echo "   <datafile> the scas monitoring log file"
}

if [ $# -eq 0 ]; then
  echo "You have to provide one datafile"
  usage
  exit 1
else
  absdatafile=$1
fi

#check the file
if [ ! -f $absdatafile ]; then
  echo "$absdatafile is not a valid file"
fi
cp -f $absdatafile ./
datafile=`basename $absdatafile`
datafile_noext=`echo $datafile | cut -d'.' -f1`
gnuplotfile=$datafile_noext.plt
echo "Creating $gnuplotfile file"

cat <<EOF > $gnuplotfile
set title "SCAS server memory consumption"
set xlabel "Time (sec)"
set ylabel "Memory consumption (KB)"
set autoscale
set datafile separator ","
set terminal postscript landscape enhanced color
set output "$datafile_noext.ps"
plot "$datafile" using 1:6 title ""
EOF
gnuplot $gnuplotfile
if [ $? -ne 0 ]; then
  echo "Error in gnuplot call"
else
  echo "$datafile_noext.ps ready"
fi

rm -f ./*_data

exit 0

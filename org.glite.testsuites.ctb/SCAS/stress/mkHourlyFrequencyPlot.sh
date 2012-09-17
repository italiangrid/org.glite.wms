#!/bin/sh

function usage() {
echo "Usage $0 <datafile>"
echo "   <datafile> the frequency data file"
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
set title "Hourly Frequency"
set xlabel "Time (hour)"
set ylabel "Frequency Hz"
set autoscale
set datafile separator ","
set terminal postscript landscape enhanced color
set output "$datafile_noext.ps"
plot "$datafile" using 1:2 title "" with lines
EOF
gnuplot $gnuplotfile
if [ $? -ne 0 ]; then
  echo "Error in gnuplot call"
else
  echo "$datafile_noext.ps ready"
fi

rm -f ./*_data

exit 0

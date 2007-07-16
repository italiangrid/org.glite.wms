# loads.sh outCE outWN outCE graph.png

INPUT1=$1
INPUT2=$2
INPUT3=$3
OUTPUT=$4
TICS=$5
( 
cat<<EOF
set grid
set key left nobox
set xtics border nomirror norotate $TICS
set ytics border nomirror norotate 
set style data lines

set xdata time
set timefmt "%Y-%m-%dT%H:%M:%S+0200"
set format x "%H:%M"
set xlabel "Time"

set ylabel "1 min load average"

set title "Average Loads"

set terminal png size 500,300
EOF

echo "set output '$OUTPUT'"
echo "plot '$INPUT1' using 1:2 title 'lcgCE' with lines lw 3, \
     '$INPUT2' using 1:2 title 'WN_SGE' with lines lw 3, \
     '$INPUT3' using 1:8 title 'SGE_server' with lines lw 3"
)|gnuplot

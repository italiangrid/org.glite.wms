# gip-bdii.sh outBDII graph.png
INPUT=$1
OUTPUT=$2
TICS=$3
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

set ylabel "Jobs"

set title "LRMS Queue Length"

set terminal png size 500,300
EOF

echo "set output '$OUTPUT'"
echo "plot '$INPUT' using 1:6 title 'Waiting (GIP)' with lines lw 3, \
     '$INPUT' using 1:9 title 'Waiting (BDII)' with lines lw 3"
)|gnuplot

# sgeschedd-mem.sh outCE graph.png
INPUT=$1
OUTPUT=$2
TICS=$3
( 
cat<<EOF
set mouse
set grid
set key left nobox
set xtics border nomirror norotate $TICS
set ytics border nomirror norotate
set y2tics border nomirror norotate
set style data lines

set xdata time
set timefmt "%Y-%m-%dT%H:%M:%S+0200"
set format x "%H:%M"
set xlabel "Time"

set ylabel "Size (Meg)"
set y2label "Queued Jobs"

set title "SGE Schedd Process Size"

set terminal png size 600,300
EOF


echo "set output '$OUTPUT'"
echo "plot '$INPUT' using 1:11 axes x1y2 title 'Jobs' with lines lw 3, \
           '$INPUT' using 1:7 title 'SGE Schedd Footprint' with lines lw 3"
            
)|gnuplot

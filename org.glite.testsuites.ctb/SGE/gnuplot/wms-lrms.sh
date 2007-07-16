
# wms-lrms.sh outwms outCE_wms graph.png
WMS_INPUT=$1
LRMS_INPUT=$2
OUTPUT=$3
TICS=$4

( 
cat<<EOF
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

set ylabel "Jobs"

set title "WMS vs LRMS status report"

set terminal png size 600,300
EOF

[ -n "$OUTPUT" ] && echo "set output '$OUTPUT'"
echo "plot '$WMS_INPUT' using 1:9 title 'WMS Ready' with lines lw 3, \
           '$WMS_INPUT' using 1:3 title 'WMS Done' with lines lw 3, \
           '$WMS_INPUT' using 1:7 title 'WMS Running' with lines lw 3, \
           '$WMS_INPUT' using 1:5 title 'WMS Scheduled' with lines lw 3, \
           '$LRMS_INPUT' using 1:12 title 'LRMS Done' with lines lw 3"

)|gnuplot

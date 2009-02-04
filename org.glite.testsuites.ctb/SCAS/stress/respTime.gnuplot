set title "Response Time"
set xlabel "Time (sec from Unix epoch)
set ylabel "Response time (sec)
set autoscale
set datafile separator ","
plot "vtb-generic-111.cern.ch_data" using 1:2
set terminal postscript enhanced color
set output "plot.ps"


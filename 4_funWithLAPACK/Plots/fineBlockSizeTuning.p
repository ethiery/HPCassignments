set terminal png
set output 'Plots/fineBlockSizeTuning.png'

set autoscale
unset logscale
set xtics auto
set ytic auto
set grid mxtics mytics
set title "Compute time of the LU factorization of a 4000x4000 matrix, with 20 cores of one node, depending on fine block size"
set ylabel "Compute time (s)"
set xlabel "Fine block size"
plot "Output/fineBlockSizeTuning.dat" using 1:2 title '' with linespoints
set terminal png
set output 'Plots/coarseBlockSizeTuning.png'

set autoscale
unset logscale
set xtics auto
set ytic auto
set grid mxtics mytics
set title "Compute time of the LU factorization of a 4000x4000 matrix, with 20 cores of one node, fine blocks of size 84, depending on coarse block size"
set ylabel "Compute time (s)"
set xlabel "Coarse block size"
plot "Output/coarseBlockSizeTuning.dat" using 1:2 title '' with linespoints
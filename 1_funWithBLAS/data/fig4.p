set terminal png
set output 'myLib_vs_mkl_ddot_vectorial_parallel.png'

set autoscale
set logscale
set xtic auto
set ytic auto
set key top right
set grid mxtics mytics
set title "ddot - MKL multi threads VS myLib implémentation vectorielle multi threads"
set ylabel "Flops/s"
set xlabel "Taille des opérandes"
plot "myLib_ddot_vectorial_parallel_withoutFlush.dat" using 1:3 title 'MyLib' with points, \
     "mkl_ddot_multiThread_withoutFlush.dat" using 1:3 title 'MKL' with points

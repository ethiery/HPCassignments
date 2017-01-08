set terminal png
set output 'myLib_vs_mkl_ddot_vectorial_sequential.png'

set autoscale
unset logscale ; set logscale x
set xtic auto
set ytic auto
set key top right
set grid mxtics mytics
set title "ddot - MKL single thread VS myLib implémentation vectorielle séquentielle"
set ylabel "Flops/s"
set xlabel "Taille des opérandes"
plot "myLib_ddot_vectorial_sequential_withoutFlush.dat" using 1:3 title 'MyLib' with points, \
     "mkl_ddot_singleThread_withoutFlush.dat" using 1:3 title 'MKL' with points

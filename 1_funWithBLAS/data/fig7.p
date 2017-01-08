set terminal png
set output 'myLib_dgemm_bloc_vs_ijk.png'

set autoscale
set logscale
set xtic auto
set ytic auto
set key top right
set grid mxtics mytics
set title "dgemm version bloquée VS version scalaire"
set ylabel "flops/s"
set xlabel "Taille commune à toutes les dimensions des opérandes"
plot "myLib_dgemm_scalar_ijk_withoutFlush.dat" using 1:3 title 'Version scalaire' with points, \
     "myLib_dgemm_bloc_withoutFlush.dat" using 1:3 title 'Version bloquée parallélisée avec OPENMP' with points, \

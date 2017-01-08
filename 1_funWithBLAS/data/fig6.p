set terminal png
set output 'myLib_dgemm_scalar_versions_flops.png'

set autoscale
set logscale
set xtic auto
set ytic auto
set key top right
set grid mxtics mytics
set title "dgemm versions scalaires - performance en flops/s"
set ylabel "flops/s"
set xlabel "Taille commune à toutes les dimensions des opérandes"
plot "myLib_dgemm_scalar_ijk_withoutFlush.dat" using 1:3 title 'Triple boucle ordre (i, j, k)' with points, \
     "myLib_dgemm_scalar_jik_withoutFlush.dat" using 1:3 title 'Triple boucle ordre (j, i, k)' with points, \
     "myLib_dgemm_scalar_kij_withoutFlush.dat" using 1:3 title 'Triple boucle ordre (k, i, j)' with points

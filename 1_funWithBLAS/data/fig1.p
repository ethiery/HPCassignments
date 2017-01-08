set terminal png
set output 'myLib_ddot_scalar_sequential_cacheEffects.png'

set autoscale
set logscale
set xtic auto
set ytic auto
set grid mxtics mytics
set title "Performances ddot - implémentation séquentiel et scalaire"
set ylabel "Flops/s"
set xlabel "Taille des opérandes"
plot "myLib_ddot_scalar_sequential_withFlush.dat" using 1:3 title 'Avec cache flush' with points, \
     "myLib_ddot_scalar_sequential_withoutFlush.dat" using 1:3 title 'Sans cache flush' with points

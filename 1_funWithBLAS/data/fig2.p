set terminal png size 1000,480
set output 'myLib_ddot_scalar_sequential_cacheMissRatios.png'

set autoscale
set yrange [0:0.05]
unset logscale ; set logscale x
set xtic auto
set ytic auto
set grid mxtics mytics
set title "Ratios de Cache Miss L1 - ddot - implémentation séquentiel et scalaire"
set ylabel "Ratio Cache Miss / Nombre d'instruction"
set xlabel "Taille des opérandes"
set key outside bottom
plot "myLib_ddot_scalar_sequential_withFlush.dat" using 1:7 title 'Cache de données L1, avec cache flush' with points, \
     "myLib_ddot_scalar_sequential_withoutFlush.dat" using 1:7 title 'Cache de données L1, sans cache flush' with points
     #"myLib_ddot_scalar_sequential_withFlush.dat" using 1:8 title 'Cache de données LL, avec cache flush' with points, \
     #"myLib_ddot_scalar_sequential_withoutFlush.dat" using 1:8 title 'Cache de données L1, sans cache flush' with points

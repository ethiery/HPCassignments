set terminal png
set output 'speedup1024.png'

set autoscale
unset logscale
set xtic auto
set ytic auto
set grid mxtics mytics
set title "Speedup sur la simulation de 10000 pas de temps sur 1024 particules"
set ylabel "Speedup"
set xlabel "Nombre de processus distribué sur 4 machines de 16 coeurs"
plot "time1024.dat" using 1:($2/$3) title '' with linespoints,\
     x title '' with lines

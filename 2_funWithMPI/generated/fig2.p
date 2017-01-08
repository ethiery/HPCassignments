set terminal png
set output 'speedup512.png'

set autoscale
unset logscale
set xtic auto
set ytic auto
set grid mxtics mytics
set title "Speedup sur la simulation de 10000 pas de temps sur 512 particules"
set ylabel "Speedup"
set xlabel "Nombre de processus sur une même machine"
plot "time512.dat" using 1:($2/$3) title '' with linespoints,\
     x title '' with lines

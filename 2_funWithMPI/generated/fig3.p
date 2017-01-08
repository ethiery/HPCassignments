set terminal png
set output 'time512bis.png'

set autoscale
unset logscale
set xtic auto
set ytic auto
set grid mxtics mytics
set title "Temps de simulation de 10000 pas de temps sur 512 particules"
set ylabel "Temps d'exécution (s)"
set xlabel "Nombre de processus distribué sur 4 machines de 16 coeurs"
plot "time512bis.dat" using 1:3 title '' with linespoints

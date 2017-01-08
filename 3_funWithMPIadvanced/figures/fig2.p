set terminal png
set output 'outputs/speedup1nodeWithScatterGather.png'

set autoscale
unset logscale
set xtics (1,4,9,16)
set ytic auto
set grid mxtics mytics
set title "Speed up sur un noeud unique, en comptant les temps de scatter/gather"
set ylabel "Speed up"
set xlabel "Nombre de coeurs utilisés"
plot "outputs/measures1200.dat" using 1:($2/($3+$4+$5)) title '1200x1200' with linespoints,\
	 "outputs/measures6000.dat" using 1:($2/($3+$4+$5)) title '6000x6000' with linespoints,\
	 "outputs/measures12000.dat" using 1:($2/($3+$4+$5)) title '12000x12000' with linespoints,\
	 "outputs/measures24000.dat" using 1:($2/($3+$4+$5)) title '24000x24000' with linespoints,\
	 x title 'y=x' with lines

set terminal png
set output 'outputs/speedupNnodesWithScatterGather.png'

set autoscale
unset logscale
set xtics (1,4,9,16)
set ytic auto
set grid mxtics mytics
set title "Speed up sur plusieurs noeuds, en comptant les temps de scatter/gather"
set ylabel "Speed up"
set xlabel "Nombre de noeuds (1 coeur par noeud)"
plot "outputs/measuresInter1200.dat" using 1:($2/($3+$4+$5)) title '1200x1200' with linespoints,\
	 "outputs/measuresInter6000.dat" using 1:($2/($3+$4+$5)) title '6000x6000' with linespoints,\
	 "outputs/measuresInter12000.dat" using 1:($2/($3+$4+$5)) title '12000x12000' with linespoints,\
	 "outputs/measuresInter24000.dat" using 1:($2/($3+$4+$5)) title '24000x24000' with linespoints,\
	 x title 'y=x' with lines

set terminal png
set output 'speedupMPI_SEQ_ASYNC.png'

unset logscale
set key top left

set xtics (1, 2, 4, 8, 16)
set ytic auto
set grid mxtics mytics
set ylabel "Speed-up par rapport au séquentiel"
set xlabel "Nombre de threads utilisés"
plot "data/benchALL.dat" using 1:($2/$18) title 'Damier 4096x4096' with linespoints,\
	 "data/benchALL.dat" using 1:($3/$19) title 'Damier 8192x8192' with linespoints,\
	 "data/benchALL.dat" using 1:($4/$20) title 'Damier 16384x16384' with linespoints,\
	 "data/benchALL.dat" using 1:($5/$21) title 'Damier 32768x32768' with linespoints,\
	 x notitle with line
set terminal png
set output 'speedupMPI_OMP_SYNC.png'

unset logscale
set key top left

set xtics (1, 2, 4, 8, 16, 32, 64)
set ytic auto
set grid mxtics mytics
set ylabel "Speed-up par rapport au séquentiel"
set xlabel "Nombre de threads utilisés (8 par noeud)"
plot "data/benchMPI_OMP_SYNC.dat" using 1:($2/$5) title 'Damier 8192x8192' with linespoints,\
	 "data/benchMPI_OMP_SYNC.dat" using 1:($3/$6) title 'Damier 16384x16384' with linespoints,\
	 "data/benchMPI_OMP_SYNC.dat" using 1:($4/$7) title 'Damier 32768x32768' with linespoints,\
	 x notitle with line
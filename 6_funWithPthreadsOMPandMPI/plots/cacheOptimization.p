set terminal png
set output './cacheOptimization.png'

unset logscale
set logscale x
set key bottom right

set xtics (256, 512, 1024, 2048, 4096, 8192, 16384, 32768)
set ytic auto
set grid mxtics mytics
set ylabel "Speed-up par rapport à une version non bloquée"
set xlabel "Taille de bloc"
plot "data/cacheOptimization.dat" using 1:($7/$6) title 'Damier 2048x2048' with linespoints,\
	 "data/cacheOptimization.dat" using 1:($11/$10) title 'Damier 8192x8192' with linespoints,\
	 "data/cacheOptimization.dat" using 1:($13/$12) title 'Damier 16384x16384' with linespoints,\
	 "data/cacheOptimization.dat" using 1:($15/$14) title 'Damier 32768x32768' with linespoints,\
	 1 notitle with line
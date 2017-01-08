set terminal png
set output 'Plots/pdgetrfScaling.png'

tSeq(Col) = system("sed '2!d' Output/pdgetrf.dat | awk '{print $" . Col . "}'")

set autoscale
unset logscale
set xtics auto
set ytic auto
set grid mxtics mytics
set title "Speed up on the LU factorization of a random matrix"
set ylabel "Speed-up compare to one node"
set xlabel "Number of nodes used (20 cores per node)"
plot "Output/pdgetrf.dat" using 1:(tSeq(2)/$2) title '10584x10584' with linespoints,\
	 "Output/pdgetrf.dat" using 1:(tSeq(3)/$3) title '21168x21168' with linespoints,\
	 "Output/pdgetrf.dat" using 1:(tSeq(4)/$4) title '30240x30240' with linespoints,\
	 "Output/pdgetrf.dat" using 1:(tSeq(5)/$5) title '40824x40824' with linespoints,\
	 x title 'y=x' with lines
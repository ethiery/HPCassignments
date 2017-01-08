set terminal png
set output 'Plots/tuning.png'
set bars 4.0
set style fill empty

set logscale y
unset logscale x
set ylabel "Relative errors on forces applied on particles"
set xlabel "Theta"
# Data columns: l/d min 1stQuartile Median 3rdQuartile Max
plot "Output/tuning.dat" using 1:3:2:6:5 with candlesticks title "relativeError (min, max and quartiles)", \
	 "" using 1:4:4:4:4 with candlesticks lt -1 notitle


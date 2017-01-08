sizes=`sed '1d' Output/pdgetrf1.dat | awk '{print $1}' | tr "\n" " "`
echo "# nbNodes ${sizes}" > Output/pdgetrf.dat

for nbNodes in 1 3 6 9
do
	vals=`sed '1d' Output/pdgetrf${nbNodes}.dat | awk '{print $2}' | tr "\n" " "`
	echo "${nbNodes} ${vals}" >> Output/pdgetrf.dat
done

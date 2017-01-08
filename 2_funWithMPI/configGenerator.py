import sys
import math

f = open('inputs/{}.dat'.format(sys.argv[1]), 'w')

n = int(sys.argv[1])
f.write('{}\n'.format(n))

for i in range(n):
    f.write('{:.5E} 0 {:.5E} {:.5E} 0\n'.format(
        (i+1) * 1e23,
        (i+1) * 1e9,
        (i%10) * 1e3))

f.close()

# funWithMPI

## Compilation

make

## Execution

To compute N steps, starting from the initial state described by file input.dat,
using sequential code, saving each step k in a file k.dat in the directory
outputDir (which must not exist beforehand),
and allowing particles to travel at most R (in ]0,1[) of the distance to the
closest particle at each step :

./simulator -i input.dat -n N -s -o outputDir -r R

To benchmark, disable the output :
./simulator -i input.dat -n N -s -r R

To use the parallel version using MPI, omit the -s:
./simulator -i input.dat -n N -r R

We recommend playing with R and displaying the result to see which R is needed

To save the result in a video file video.mp4 of L ms. ffmpeg need to be installed
for that. The generation can take some time (<1sec per sec of video generated)

python3 videoGenerator.py input.dat outputDir L video.mp4

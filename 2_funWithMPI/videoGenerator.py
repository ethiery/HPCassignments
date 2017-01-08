import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def readFile(file):
    x, y = [], []
    with open(file, 'r') as f:
        f.readline() #skip first line
        for line in f:
            values = line.split(' ')
            x.append(float(values[1]))
            y.append(float(values[2]))
    return x, y


if len(sys.argv) != 5:
    print('Usage: python3 videoGenerator inputFile outputFilesDir videoLengthInMs videoFileName')
    exit()

inputFile = sys.argv[1]
if not os.path.isfile(inputFile):
    print('Invalid path to input file')
    exit()

xi, yi = readFile(inputFile)
xmin, xmax = min(xi), max(xi)
ymin, ymax = min(yi), max(yi)


d = sys.argv[2]
if not os.path.isdir(d):
    print('Invalid path to output files directory')
    exit()


videoLengthMs = int(sys.argv[3])
videoFileName = sys.argv[4]
# prepare frames, fusing the ones that won't fit into 20 fps anyway

frames = []
files = [f for f in os.listdir(d)]
files.sort(key=lambda s: int(s.replace('.dat', '')))

msPerFile = videoLengthMs / len(files)
msPerFrame = 50
nextFrameAfter = 0
currentTime = 0
frameXs, frameYs = [], []
for f in files:
    x, y = readFile(os.path.join(d, f))
    xmin, xmax = min(min(x), xmin), max(max(x), xmax)
    ymin, ymax = min(min(y), ymin), max(max(y), ymax)
    frameXs.extend(x)
    frameYs.extend(y)
    if currentTime >= nextFrameAfter:
        frames.append((frameXs,frameYs))
        frameXs, frameYs = [], []
        while nextFrameAfter <= currentTime:
            nextFrameAfter += msPerFrame
    currentTime += msPerFile

fig = plt.figure()
plt.xlim(xmin, xmax)
plt.ylim(ymin, ymax)
scat = plt.scatter(xi, yi)

def update_plot(frame):
    # print(coords)
    # scat.set_offsets(coords)
    fig.clear()
    plt.xlim(xmin, xmax)
    plt.ylim(ymin, ymax)
    plt.scatter(*frame)

anim = animation.FuncAnimation(fig, update_plot, frames=frames, interval=videoLengthMs/len(frames), repeat=False)
anim.save(videoFileName, writer=animation.FFMpegWriter(fps=1000/msPerFrame))
# plt.show()

CC=mpicc
CFLAGS= -std=c11 -Wall -O3
LDFLAGS = -std=c11 -Wall -lm

SRC=Particle.c sequential.c util.c parallel.c
OBJ=$(SRC:.c=.o)

TESTS=tests
EXEC=simulator

all: $(TESTS) $(EXEC)

simulator: $(OBJ) driver.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

tests: $(OBJ) tests.o
	rm -f testIO.dat ;\
	$(CC) -o $@ $^ $(LDFLAGS)

earthMoonSeq.mp4: $(EXEC)
	./simulator -i inputs/earthMoon.dat -s -o earthMoonSeqOut -r 0.01 -n 1000 -p 25
	python3 videoGenerator.py inputs/earthMoon.dat earthMoonSeqOut/ 2000 generated/earthLuneSeq.mp4
	rm -r earthMoonSeqOut

solarSystemSeq.mp4: $(EXEC)
	./simulator -i inputs/solarSystem.dat -s -o solarSystemSeqOut -r 0.001 -n 2000000 -p 10000
	python3 videoGenerator.py inputs/solarSystem.dat solarSystemSeqOut/ 10000 generated/solarSystemSeq.mp4
	rm -r solarSystemSeqOut

collisionSeq.mp4: $(EXEC)
	./simulator -i inputs/collision.dat -s -o collisionSeqOut -r 0.001 -n 2000000 -p 10000
	python3 videoGenerator.py inputs/collision.dat collisionSeqOut/ 10000 generated/collisionSeq.mp4
	rm -r collisionSeqOut

compEarthMoon: $(EXEC)
	./simulator -i inputs/earthMoon.dat -s -o earthMoonSeqOut -r 0.01 -n 1000 -p 25
	mpiexec -n 2 ./simulator -i inputs/earthMoon.dat -o earthMoonParOut -r 0.01 -n 1000 -p 25
	diff earthMoonSeqOut earthMoonParOut
	rm -r earthMoonSeqOut earthMoonParOut

compSolarSystem: $(EXEC)
	./simulator -i inputs/solarSystem.dat -s -o solarSystemSeqOut -r 0.001 -n 20000 -p 100
	mpiexec -n 8 ./simulator -i inputs/solarSystem.dat -o solarSystemParOut -r 0.001 -n 20000 -p 100
	diff solarSystemSeqOut solarSystemParOut
	rm -r solarSystemSeqOut solarSystemParOut


compCollision: $(EXEC)
	./simulator -i inputs/collision.dat -s -o collisionSeqOut -r 0.001 -n 20000 -p 100
	mpiexec -n 8 ./simulator -i inputs/collision.dat -o collisionParOut -r 0.001 -n 20000 -p 100
	diff collisionSeqOut collisionParOut
	rm -r collisionSeqOut collisionParOut


.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(TESTS) $(EXEC) *.mp4 *Out *.dat

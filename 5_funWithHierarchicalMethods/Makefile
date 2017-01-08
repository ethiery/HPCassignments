CC=mpicc
CFLAGS= -std=c11 -Wall -O3 -DMKL_ILP64 -fopenmp -I${MKLROOT}/include -march=native
LDFLAGS= -std=c11 -Wall -fopenmp -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_gnu_thread -lpthread -ldl -lm
EXEC=tests tuning benchLocal benchNaive benchDistributed
SRC=Multipole.c Quadtree.c Morton.c Cell.c WorkingVecs.c utils.c
OBJ=$(SRC:.c=.o)

all: $(EXEC)

tests: tests.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

tuning: tuning.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

benchNaive: benchNaive.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

benchLocal: benchLocal.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

benchDistributed: benchDistributed.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

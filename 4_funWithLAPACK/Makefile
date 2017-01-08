CC=mpicc
CFLAGS= -std=c11 -Wall -O3 -DMKL_ILP64 -fopenmp -I${MKLROOT}/include
LDFLAGS = -std=c11 -Wall -fopenmp -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_gnu_thread -lpthread -ldl -lm
EXEC=correctionAndStability performance
SRC=LU.c utils.c distributedData.c genericWrapper.c
OBJ=$(SRC:.c=.o)

all: $(EXEC)

correctionAndStability: correctionAndStability.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

performance: performance.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

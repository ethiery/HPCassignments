CC=mpicc
CFLAGS= -std=c11 -Wall -O3 -DMKL_ILP64 -fopenmp -I${MKLROOT}/include
LDFLAGS = -std=c11 -Wall -fopenmp -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_gnu_thread -lpthread -ldl -lm
EXEC= localGemm distributedGemm randomMatrixGenerator


all: $(EXEC)

localGemm: localGemm.o util.o
	$(CC) -o $@ $^ $(LDFLAGS)

distributedGemm: distributedGemm.o util.o
	$(CC) -o $@ $^ $(LDFLAGS)

randomMatrixGenerator: randomMatrixGenerator.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC) inputs/* outputs/*

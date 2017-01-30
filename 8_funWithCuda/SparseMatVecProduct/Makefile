CC=nvcc
CFLAGS=-lcublas


test: test.o
	$(CC) -o $@ $^

sparseMatVec: sparseMatVec.o
	$(CC) -o $@ $^ $(CFLAGS)


%.o: %.cu
	$(CC) -o $@ -c $<

.PHONY: clean mrproper

clean:
	rm -rf *.o *.out *.err

mrproper: clean
	rm -rf test sparseMatVec

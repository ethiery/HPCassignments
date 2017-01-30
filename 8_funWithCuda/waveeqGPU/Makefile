CC=nvcc

SRC=src
INC=inc
LIB=lib
BIN=bin
SOL=sol

TARGET=libutils.so
COMPUTE=libcompute.so

all:
	$(CC) --shared -cudart shared --compiler-options '-fPIC -g' -o $(TARGET) $(SRC)/utils.cu
	mv $(TARGET) $(LIB)/.
	$(CC) --shared -cudart shared --compiler-options '-fPIC -g' -o $(COMPUTE) $(SRC)/compute.cu
	mv $(COMPUTE) $(LIB)/.
	$(CC) -cudart shared -I./$(INC) -L./$(LIB) $(LIB)/libutils.so $(LIB)/libcompute.so $(SRC)/wave.cu -o $(BIN)/wave


clean:
	rm -f *.o $(LIB)/libutils.so $(LIB)/libcompute.so $(BIN)/wave $(SOL)/*


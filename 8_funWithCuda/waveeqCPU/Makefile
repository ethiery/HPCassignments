CC=gcc
CFLAGS=-lm -W -Wall -ansi -pedantic -std=c99 -m64
LFLAGS=-fPIC -shared

SRC=src
INC=inc
LIB=lib
BIN=bin

TARGET=libutils.so
COMPUTE=libcompute.so

all:
	$(CC) -fPIC -shared -c $(SRC)/utils.c -o $(TARGET)
	mv $(TARGET) $(LIB)/.
	$(CC) -fPIC -shared -c $(SRC)/compute.c -o $(COMPUTE)
	mv $(COMPUTE) $(LIB)/.
	$(CC) -I./$(INC) -L./$(LIB) $(LIB)/libutils.so $(LIB)/libcompute.so $(SRC)/wave.c $(CFLAGS) -o $(BIN)/wave


clean:
	rm -f *.o $(LIB)/libutils.so $(LIB)/libcompute.so $(BIN)/wave


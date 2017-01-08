SRCDIR=src
TESTDIR=tst
VPATH=$(SRCDIR):$(TESTDIR)

CC=gcc
CFLAGS = -std=c11 -Wall -lm -g -O0 -I$(SRCDIR) -I$(TESTDIR) -DMKL_ILP64 -fopenmp -I${MKLROOT}/include -march=native
LDFLAGS = -std=c11 -Wall -lm -L$(SRCDIR) -L$(TESTDIR) -fopenmp
MKLFLAGS = -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_gnu_thread -lpthread -lm -ldl

CUSTOMSRC = ddot.c dgemm.c
CUSTOMOBJ = $(CUSTOMSRC:.c=.o)

SHAREDSRC = util.c tests.c benchtools.c dgemm_versions.c
SHAREDOBJ = $(SHAREDSRC:.c=.o)

EXEC = myLib mkl

all: $(EXEC) clean

myLib: $(CUSTOMOBJ) $(SHAREDOBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(CUSTOMFLAGS)

mkl: $(SHAREDOBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(MKLFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(CUSTOMFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o *.gch

mrproper: clean
	rm -rf $(EXEC) *.csv

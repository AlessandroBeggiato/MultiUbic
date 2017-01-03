IDIR =include
CC=gcc
CFLAGS=-I $(IDIR) -O3 -Wall

ODIR=src/obj
BINDIR=bin

_OBJ_partitioner =  partitioner.o msd.o
OBJ_partitioner = $(patsubst %, $(ODIR)/%, $(_OBJ_partitioner))

_OBJ_MultiUbic = MultiUbic.o debug.o ec.o exm.o global.o glue.o h.o marking.o msd.o netconv.o nodelist.o output.o pe.o readlib.o readpep.o stack.o unfold.o al.o dls.o ls.o lst.o
OBJ_MultiUbic = $(patsubst %, $(ODIR)/%, $(_OBJ_MultiUbic))

all: MultiUbic tools

tools: partitioner testNetGen

partitioner: $(OBJ_partitioner)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS)

MultiUbic: $(OBJ_MultiUbic)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS)

testNetGen: src/testNetGen.cpp
	g++ -o $(BINDIR)/testNetGen $^ $(CFLAGS)

$(ODIR)%.o: src%.c
	$(CC) -c -o $@ $^ $(CFLAGS)

$(ODIR)/al.o: src/al/al.c
	$(CC) -c -o $@ src/al/al.c $(CFLAGS)

$(ODIR)/dls.o: src/dls/dls.c
	$(CC) -c -o $@ src/dls/dls.c $(CFLAGS)

$(ODIR)/ls.o: src/ls/ls.c
	$(CC) -c -o $@ src/ls/ls.c $(CFLAGS)

$(ODIR)/lst.o: src/lst/lst.c
	$(CC) -c -o $@ src/lst/lst.c $(CFLAGS)

.PHONY: clean all tools

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
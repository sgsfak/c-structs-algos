CC=clang
CFLAGS=-O2 -Wall -std=c99
all: hashes hashes2 search

hashes: hashes.o lch_hmap.o hfn.o
hashes2: hashes.o lch_hmap2.o hfn.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	\rm -rf *.o hashes hashes2

CC=clang
CFLAGS=-O2 -Wall -std=c99

CFLAGS += -MMD -MP
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

all: hashes hashes2 search

hashes: hashes.o lch_hmap.o hfn.o vec.o
hashes2: hashes.o lch_hmap2.o hfn.o vec.o
	$(CC) -o $@ $^ $(CFLAGS)

-include $(SRC:%.c=%.d)

clean:
	\rm -rf $(OBJ) hashes hashes2

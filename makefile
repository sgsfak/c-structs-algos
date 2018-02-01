CC=clang
CFLAGS=-O3 -Wall -Werror -std=c99
all: hashes search

hashes: lch_hmap.o hfn.o


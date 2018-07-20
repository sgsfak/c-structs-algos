CC=cc
CXX=c++
CFLAGS=-O2 -Wall -std=c99
CXXFLAGS=-O2 -Wall -std=c++11

CFLAGS += -MMD -MP
CXXFLAGS += -MMD -MP
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

all: hashes hashes2 cpphashes search vec_test

hashes: hashes.o lch_hmap.o hfn.o vec.o
	$(CC) -o $@ $^ $(CFLAGS)

hashes2: hashes.o lch_hmap2.o hfn.o vec.o
	$(CC) -o $@ $^ $(CFLAGS)


cpphashes: cpphashes.cpp vec.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

vec_test: vec.o

-include $(SRC:%.c=%.d)

clean:
	\rm -rf $(OBJ) hashes hashes2 cpphashes *.d

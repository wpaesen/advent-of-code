CFLAGS+=-g -O3
CXXFLAGS+=-g -O3

all: aocpp aoc

aocpp: aoc.cpp
	$(CXX) $(CXXFLAGS) -o aocpp aoc.cpp

aoc: aoc.c
	$(CC) $(CFLAGS) -o aoc aoc.c


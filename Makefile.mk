CC = g++
CFLAGS = -g -Wall -O2 -std=c++0x
sources = $(wildcard *.cc)
objects = $(sources:.cc=.o)

force_check:;

.cc.o: 
	$(CC) $(CFLAGS) $(include_dirs) -c $< -o $@


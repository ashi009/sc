CC = g++
CPPFLAGS = -Wall -O2 -std=c++0x
sources = $(wildcard *.cc)
objects = $(sources:.cc=.o)

TARGET ?= linux

ifeq ($(TARGET),linux)
other_flags += 
else
other_flags += -DGLFWTHREAD
endif

.cc.o: 
	$(CC) $(CPPFLAGS) $(other_flags) $(include_dirs) -c $< -o $@

%.pbo.o: %.cc
	$(CC) $(CPPFLAGS) $(other_flags) -DPBO $(include_dirs) -c $< -o $@

%.cpu.o: %.cc
	$(CC) $(CPPFLAGS) $(other_flags) -DCPU $(include_dirs) -c $< -o $@


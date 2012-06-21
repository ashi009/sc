CC = g++
CPPFLAGS = -g -Wall -O2 -std=c++0x
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


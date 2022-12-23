ROOT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

INCLUDE_DIR ?= $(ROOT_DIR)/common
LIB_DIR ?= $(ROOT_DIR)/common

CC = gcc
CXX = g++
CPPFLAGS ?=
CFLAGS ?= -O0

CPPFLAGS:=$(CPPFLAGS) -I$(INCLUDE_DIR) 
LDLIBS:= $(LDLIBS) -lpmem -Wl,-rpath=/usr/local/lib64

LIBSRC:=$(wildcard $(LIB_DIR)/*.c )
HEADERS:=$(wildcard $(INCLUDE_DIR)/*.h )

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): %:%.c $(LIBSRC) $(HEADERS)
	$(CC) $(filter %.c %.o, $^) $(LDLIBS) $(CPPFLAGS) $(CFLAGS) -o $@

clean:
	rm -rf $(TARGETS) *.o

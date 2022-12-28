TARGETS:= sender receiver 

$(TARGETS): config.h

config.h: gen_config_h.py
	python3 $^

include ../Makefile.inc
CFLAGS += -lpthread

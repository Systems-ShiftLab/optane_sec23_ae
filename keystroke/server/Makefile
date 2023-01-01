# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2015-2018, Intel Corporation

WS=../deps/wsServer

CFLAGS += -I$(WS)/include -DDISABLE_VERBOSE

LDFLAGS += -L.
LDFLAGS += -lws
LDFLAGS += -lpmemkv 
LDFLAGS += -lpthread 

all: ws_server

libws.a:
	make -C $(WS)
	rm -f libws.a
	cp $(WS)/libws.a .

ws_server: libws.a ws_server.c 
	gcc ws_server.c -o ws_server $(LDFLAGS) $(CFLAGS)

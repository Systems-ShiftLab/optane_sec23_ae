pmemkv_server: pmemkv_server.c latency_log.c
	gcc $^ -luv -lpmemkv -o $@

clean: 
	rm -rf errlog log pmemkv_server

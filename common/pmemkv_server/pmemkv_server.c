/*
 * pmemkv_server.c
 * This requires libuv and libpmemkv.
 * To compile: gcc pmemkv_server.c -luv -lpmemkv -o pmemkv_server
 *
 *
 * Once the server is running, the following messages will work
 * ------> put-my_key_string-my_value
 * ------> get-my_key_string
 * ------> ping
 */

#include <uv.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <x86intrin.h>
#include <libpmemkv.h>

#include "latency_log.h"

//#define LOG(msg) printf(stdout,msg)
#define LOG(msg) fprintf(stderr,"%s\n",msg)

#define PMEMKV_ASSERT(expr) \
	do { \
		if (!(expr)) \
			puts(pmemkv_errormsg()); \
		assert(expr); \
	} while (0)



static uv_tcp_t server;
static uv_loop_t *loop;
static uv_buf_t msg_buf = {0};
static pmemkv_db *db = NULL;



static void client_close_cb(uv_handle_t *handle)
{
	free(handle);
}


static void write_done_cb(uv_write_t *req, int status)
{
	free(req);
	if (status == -1) {
		LOG("response failed");
	}
}

#define RESPONSE_WRITE_HARDCODED(c,x) response_write(c,x"\n",sizeof(x))
static void response_write(uv_stream_t *client, char *res,size_t len)
{
    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t)); assert (req != NULL);


	uv_buf_t wrbuf = uv_buf_init(res, len);
	uv_write(req, client, &wrbuf, 1, write_done_cb);
}


static void pmemkv_put_handler(uv_stream_t *client,const char* key, const char* value) 
{
    size_t key_len = strlen(key)+1;
    size_t value_len = strlen(value)+1;
	int s = pmemkv_put(db, key, key_len, value, value_len);
	PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);
    char* read_buff = malloc(value_len);
	s = pmemkv_get_copy(db, key, key_len, read_buff, value_len, NULL);
	PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);
	assert(!strcmp(value, read_buff));
    free(read_buff);
    RESPONSE_WRITE_HARDCODED(client,"success");
}

static void pmemkv_get_handler(uv_stream_t *client,const char* key)
{
    size_t key_len = strlen(key)+1;
    size_t value_len = 0;
	int s = pmemkv_get_copy(db, key, key_len, NULL, 0,&value_len);
    if (s==PMEMKV_STATUS_NOT_FOUND) 
    {
        RESPONSE_WRITE_HARDCODED(client,"<null>");
        return;
    }

    char* value = malloc(value_len);

	s = pmemkv_get_copy(db, key, key_len, value, value_len,NULL);
    value[value_len-1] = '\n';
	PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);

    response_write(client,value,value_len);
    free(value);
}

static void get_read_buf_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
	buf->base = msg_buf.base;
	buf->len = msg_buf.len;
}

//Message format: "put-This is my key-This is my value<\n>"
//Message format: "get-This is my key<\n>"
#define MAX_ARGS 3
#define MAX_DELIM (MAX_ARGS-1)
#define INVL_PKT(x)  do { RESPONSE_WRITE_HARDCODED(client,x); return; } while (0)
#define STR_CMP(x,y) (strncmp(x,y,sizeof(x)-1) == 0)
#define TIME(fn_call,lat_val_ptr) do { \
    unsigned int dummy; \
    uint64_t start = __rdtscp(&dummy); \
    fn_call; \
    uint64_t end = __rdtscp(&dummy); \
    *lat_val_ptr = end-start; \
} while (0)
static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	if (nread <= 0) {
		LOG("client connection closed");
		uv_close((uv_handle_t *)client, client_close_cb);
		return;
	}

    assert(nread < buf->len);

    if (buf->base[nread-1] != '\n') INVL_PKT("invalid packet - does not terminate with newline<\\n>");
    buf->base[nread-1] = 0;

    size_t delim_pos[MAX_DELIM] = {nread-1};
    size_t delim_cnt = 0;

    for (int i=0;i<nread-1;i++)
    {
        char c = buf->base[i];
        if (c!='-') continue;
        delim_pos[delim_cnt] = i;
        delim_cnt ++;
        if (delim_cnt == MAX_DELIM) break;
    }

    size_t arg_cnt = delim_cnt +1;

    char* args[MAX_ARGS] = {NULL};
    args[0] = buf->base;
    for (int i=0;i<delim_cnt;i++)
    {
        buf->base[delim_pos[i]] = 0;
        args[i+1] = &buf->base[delim_pos[i]+1];
    }

    uint64_t latency;
    if (STR_CMP("put",args[0])) {
        if (arg_cnt != 3) INVL_PKT("'put' request requires 3 arguments");
        TIME(pmemkv_put_handler(client,args[1], args[2]),&latency);
        update_records(latency,'p');
    }
    else if (STR_CMP("get",args[0])) {
        if (arg_cnt != 2) INVL_PKT("'get' request requires 2 arguments");
        TIME(pmemkv_get_handler(client,args[1]),&latency);
        update_records(latency,'g');
    }
    else if (STR_CMP("ping",args[0])) {
        if (arg_cnt != 1) INVL_PKT("'ping' request requires 1 argument");
        RESPONSE_WRITE_HARDCODED(client,"pong");
    }
    else if (STR_CMP("logdump",args[0])) {
        if (arg_cnt != 2) INVL_PKT("'logdump' request requires 2 arguments");
        if(dump_records(args[1]) == 0) {RESPONSE_WRITE_HARDCODED(client,"success");}
        else {RESPONSE_WRITE_HARDCODED(client,"failure");}
    }
    else if (STR_CMP("logreset",args[0])) {
        if (arg_cnt != 1) INVL_PKT("'logreset' request requires 1 arguments");
        reset_records();
        RESPONSE_WRITE_HARDCODED(client,"success");
    }
    else {
        INVL_PKT("invalid packet - unrecognizable command");
    }

}

static void connection_cb(uv_stream_t *_server, int status)
{
	if (status != 0) {
		LOG("client connect error");
		return;
	}
	LOG("new client");

	uv_tcp_t *client = malloc(sizeof(uv_tcp_t)); assert(client != NULL);

	uv_tcp_init(loop, client);

	if (uv_accept(_server, (uv_stream_t *)client) == 0) {
		uv_read_start((uv_stream_t *)client, get_read_buf_cb, read_cb);
	} else {
		uv_close((uv_handle_t *)client, client_close_cb);
	}
}


#define POOL_SIZE 256 * 1024UL * 1024UL
#define MAX_READ_LEN (64 * 1024UL) /* 64 kilobytes */

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: %s file-name port\n", argv[0]);
		return 1;
	}

	const char *path = argv[1];
	int port = atoi(argv[2]);

	void *read_buf = malloc(MAX_READ_LEN); assert(read_buf != NULL);

	msg_buf = uv_buf_init(read_buf, MAX_READ_LEN);

	LOG("Creating config");
	pmemkv_config *cfg = pmemkv_config_new(); PMEMKV_ASSERT(cfg != NULL);
	int s = pmemkv_config_put_path(cfg, argv[1]); PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);
	s = pmemkv_config_put_size(cfg, POOL_SIZE); PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);
	s = pmemkv_config_put_create_if_missing(cfg, true); PMEMKV_ASSERT(s == PMEMKV_STATUS_OK);
    
	LOG("Opening pmemkv database with 'cmap' engine");
	s = pmemkv_open("cmap", cfg, &db);
	PMEMKV_ASSERT(s == PMEMKV_STATUS_OK && db != NULL); 

    LOG("Initializing the server-side latency log");
    init_records();

	loop = uv_default_loop();

	/* tcp server initialization */
	uv_tcp_init(loop, &server);
	struct sockaddr_in bind_addr;
	uv_ip4_addr("0.0.0.0", port, &bind_addr);
	int ret = uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0); assert(ret == 0);


	ret = uv_listen((uv_stream_t *)&server, SOMAXCONN, connection_cb);
	assert(ret == 0);

	ret = uv_run(loop, UV_RUN_DEFAULT);
	assert(ret == 0);

	uv_loop_delete(loop);
	free(read_buf);

	return 0;
}

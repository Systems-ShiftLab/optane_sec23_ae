// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2015-2018, Intel Corporation */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <ws.h>
#include <libpmemkv.h>



#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))


static pmemkv_db *db = NULL;



#define POOL_SIZE	(1024/2*1024*1024l) //Think carefully before changing this

/*** Websocket server  ***/ 
/**
 * @dir example/
 * @brief wsServer examples folder
 *
 * @file send_receive.c
 * @brief Simple send/receiver example.
 */

/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
void onopen(int fd);
void onopen(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
#ifndef DISABLE_VERBOSE
	printf("Connection opened, client: %d | addr: %s\n", fd, cli);
#endif
	free(cli);
}

/**
 * @brief Called when a client disconnects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
#ifndef DISABLE_VERBOSE
	printf("Connection closed, client: %d | addr: %s\n", fd, cli);
#endif
	free(cli);
}

/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and
 * retrieve informations about the client.
 *
 * @param msg Received message, this message can be a text
 * or binary message.
 *
 * @param size Message size (in bytes).
 *
 * @param type Message type.
 */

void insert_into_map(const char *key, const char *value,size_t value_len) {

    size_t key_len = strlen(key)+1;
	int s = pmemkv_put(db, key, key_len, value, value_len);
	assert(s == PMEMKV_STATUS_OK);
}
int cnt=0;
void onmessage(int fd, const unsigned char *msg, uint64_t size, int type)
{
	char *cli;
	cli = ws_getaddress(fd);
#ifndef DISABLE_VERBOSE
	printf("I receive a message: %s (size: %" PRId64 ", type: %d), from: %s/%d\n",
		msg, size, type, cli, fd);
#endif
	printf("%d\n",cnt++);

	insert_into_map((char*)"1",(const char*)msg,2000);


	free(cli);

	/**
	 * Mimicks the same frame type received and re-send it again
	 *
	 * Please note that we could just use a ws_sendframe_txt()
	 * or ws_sendframe_bin() here, but we're just being safe
	 * and re-sending the very same frame type and content
	 * again.
	 */
	ws_sendframe(fd, (char *)msg, size, true, type);
}

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: %s file-name port \n", argv[0]);
		return 1;
	}

	const char *path = argv[1];
	int port = atoi(argv[2]);

	pmemkv_config *cfg = pmemkv_config_new(); assert(cfg != NULL);
	int s = pmemkv_config_put_path(cfg, path); assert(s == PMEMKV_STATUS_OK);
	s = pmemkv_config_put_size(cfg, POOL_SIZE); assert(s == PMEMKV_STATUS_OK);
	s = pmemkv_config_put_create_if_missing(cfg, true); assert(s == PMEMKV_STATUS_OK);
	s = pmemkv_open("cmap", cfg, &db); assert(s == PMEMKV_STATUS_OK && db != NULL); 

	char buffer[1000] = {0};

	insert_into_map((char*)"1", buffer, 2000);

	struct ws_events evs;
	evs.onopen    = &onopen;
	evs.onclose   = &onclose;
	evs.onmessage = &onmessage;
	ws_socket(&evs, port, 0); /* Never returns. */

	return 0;
}

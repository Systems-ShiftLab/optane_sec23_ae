#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <sys/time.h>
#include <x86intrin.h>

#include "util.h"

static int sock;

static char* in_buffer;
static char* out_buffer;
static size_t out_buffer_datalen;

void init_connection(const char* ip, int port)
{
    struct sockaddr_in server;
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		fprintf(stderr, "Could not create socket\n");
        exit(1);
	}
    // Set ip
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
		fprintf(stderr, "Connection error\n");
        exit(1);
	}

    in_buffer = malloc(4096);
    out_buffer = malloc(4096*2);
}


#define SEND_CONST_STR(skt,str) send(skt, str"\n",sizeof(str),0)
void send_ping_request()
{

    if (SEND_CONST_STR(sock,"ping") < 0){
        fprintf(stderr, "Fail to send\n");
        exit(1);
    }
    
    if (recv(sock , in_buffer , 1024, 0) < 0) {
        fprintf(stderr, "Fail to receive\n");
        exit(1);
    }
}

void send_logreset_request()
{
    if (SEND_CONST_STR(sock,"logreset") < 0){
        fprintf(stderr, "Fail to send\n");
        exit(1);
    }
    
    if (recv(sock , in_buffer , 1024, 0) < 0) {
        fprintf(stderr, "Fail to receive\n");
        exit(1);
    }

}

void send_logdump_request()
{
    if (SEND_CONST_STR(sock,"logdump-noteboard") < 0){
        fprintf(stderr, "Fail to send\n");
        exit(1);
    }
    
    if (recv(sock , in_buffer , 1024, 0) < 0) {
        fprintf(stderr, "Fail to receive\n");
        exit(1);
    }

}
uint64_t send_update_request(int key)
{
    static int prev_key = -1;
    if (prev_key != key) {
        sprintf(out_buffer, "put-%d-", key);

        size_t temp_end = strlen(out_buffer);
        char* aux_buffer = &out_buffer[temp_end];
        memset(aux_buffer,'v',512);
        aux_buffer[511] = '\n';
        aux_buffer[512] = 0;
        out_buffer_datalen = strlen(out_buffer);
        prev_key = key;
    }

    unsigned int dummy;
    uint64_t start,end;
    start = get_usec();
    if (send(sock , out_buffer , out_buffer_datalen, 0) < 0) {
        fprintf(stderr, "Fail to send\n");
        return 1;
    }

    if (recv(sock , in_buffer , 1024, 0) < 0) {
        fprintf(stderr, "Fail to receive\n");
        return 1;
    }
    end = get_usec();
    return end-start;
}

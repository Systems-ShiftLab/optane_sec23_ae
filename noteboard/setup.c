#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <sys/time.h>


int sock;
struct sockaddr_in server;

void gen_insert_cmd(int key, char* val,char* message){
    sprintf(message, "put-%d-%s\n", key, val);
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server ip> <port> <num locs>\n",argv[0]);
        return 1;
    }


    size_t num_locs = atoi(argv[3]);

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
		fprintf(stderr, "Could not create socket\n");
        return 1;
	}
    // Set ip
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));

	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
		fprintf(stderr, "Connection error\n");
		return 1;
	}


    char* out_buffer;
    char* aux_buffer;
    char* in_buffer;

    in_buffer = malloc(4096);
    out_buffer = malloc(4096*2);
    aux_buffer = malloc(4096);



    for (int i=0;i<3000;i++) {

        if (send(sock , "ping\n" , strlen("ping\n"), 0) < 0) {
            fprintf(stderr, "Fail to send\n");
            return 1;
        }
        
        if (recv(sock , in_buffer , 1024, 0) < 0) {
            fprintf(stderr, "Fail to receive\n");
            return 1;
        }
    }


    for (int i = 0; i < num_locs; ++i) {
        memset(aux_buffer,'f',4095);
        aux_buffer[4095] = 0;
        gen_insert_cmd(i,aux_buffer,out_buffer);
        if (send(sock , out_buffer , strlen(out_buffer), 0) < 0) {
            fprintf(stderr, "Fail to send\n");
            return 1;
        }
        
        if (recv(sock , in_buffer , 1024, 0) < 0) {
            fprintf(stderr, "Fail to receive\n");
            return 1;
        }
    }

    for (int i = 0; i < num_locs; ++i) {
        memset(aux_buffer,'v',512);
        aux_buffer[511] = 0;
        gen_insert_cmd(i,aux_buffer,out_buffer);

        if (send(sock , out_buffer , strlen(out_buffer), 0) < 0) {
            fprintf(stderr, "Fail to send\n");
            return 1;
        }
        
        if (recv(sock , in_buffer , 1024, 0) < 0) {
            fprintf(stderr, "Fail to receive\n");
            return 1;
        }

    }
}

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <sys/time.h>

#include "util.h"
#include "config.h"

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server ip> <port>\n",argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server;
 	
    char in_buffer[1024] = {0};

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
    printf("Connected\n");


    uint64_t time_quantum=get_usec()/MSG_BIT_INTERVAL;
    while(1)
    {
        //printf("\r%ld",time_quantum);
        //fflush(stdout);
        uint64_t new_time_quantum=get_usec()/MSG_BIT_INTERVAL;
        if (new_time_quantum != time_quantum)
        {
            assert(new_time_quantum == time_quantum+1);
            time_quantum++;
            break;
        }
    }

    int num_bits = COVERT_MSG_SIZE;
    for (int i = 0; i < num_bits; ++i) {
        int num_wl = 0;
        while(1)
        {
            char* msg = sender2server_msg;
            int msg_len = sizeof(sender2server_msg)-1;

            uint64_t start,end;
            start = get_usec();
            if (send(sock , msg , msg_len, 0) < 0) {
                fprintf(stderr, "Fail to send\n");
                return 1;
            }
            
            char in_buffer[1024] = {0};
            if (recv(sock , in_buffer , 1024, 0) < 0) {
                fprintf(stderr, "Fail to receive\n");
                return 1;
            }
            end=get_usec();
            uint64_t lat = end-start;

            if (lat > WEAR_LVL_LAT_LO && lat < WEAR_LVL_LAT_HI) num_wl ++;

            uint64_t new_time_quantum=end/MSG_BIT_INTERVAL;
            if (new_time_quantum != time_quantum)
            {
                assert(new_time_quantum == time_quantum+1);
                printf("%ld - %d\n",time_quantum,num_wl);
                time_quantum++;
                break;
            }

        }
    }
}

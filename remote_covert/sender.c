#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>

#include "util.h"
#include "config.h"

_Atomic int global_msg_bit_no = 0;
_Atomic int total_packets = 0;

void sender_main_thread(void* args){
    int sock = *(int *)args;
    char* msg = sender2server_msg;
    int msg_len = sizeof(sender2server_msg)-1;
    while(1){
        int current_char=global_msg[global_msg_bit_no];
        if (current_char=='1')
        {
            total_packets++;
            if (send(sock , msg , msg_len, 0) < 0) {
                fprintf(stderr, "Fail to send\n");
                exit(1);
            }//TODO: I think snd and receive can have different threads as well
            
            char in_buffer[1024] = {0};
            if (recv(sock , in_buffer , 1024, 0) < 0) {
                fprintf(stderr, "Fail to receive\n");
                exit(1);
            }
            //printf("%s\n",in_buffer);
        }
    }

}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server ip> <port>\n",argv[0]);
        return 1;
    }

    int socks[SNDR_THREAD_CNT];
    struct sockaddr_in servers[SNDR_THREAD_CNT];
 	
    for(int i=0;i<SNDR_THREAD_CNT;i++)
    {
        socks[i] = socket(AF_INET , SOCK_STREAM , 0);
        if (socks[i] == -1) {
            fprintf(stderr, "Could not create socket\n");
            return 1;
        }
        // Set ip
        servers[i].sin_addr.s_addr = inet_addr(argv[1]);
        servers[i].sin_family = AF_INET;
        servers[i].sin_port = htons(atoi(argv[2]));

        if (connect(socks[i] , (struct sockaddr *)&servers[i] , sizeof(servers[i])) < 0) {
            fprintf(stderr, "Connection error\n");
            return 1;
        }
    }
    printf("Finished all connections\n");

    pthread_t threads[SNDR_THREAD_CNT];

    for (int i = 0; i < SNDR_THREAD_CNT; ++i) {
        pthread_create(&threads[i], NULL, (void*)&sender_main_thread, (void*)&socks[i]);
    }
    
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

    while(global_msg[global_msg_bit_no])
    {
        uint64_t new_time_quantum=get_usec()/MSG_BIT_INTERVAL;
        if (new_time_quantum != time_quantum)
        {
            printf("%ld - %c\n",time_quantum,global_msg[global_msg_bit_no]);
            assert(new_time_quantum == time_quantum+1);
            time_quantum++;
            global_msg_bit_no++;
        }
    }
    printf("%d",total_packets);

}

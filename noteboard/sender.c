#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <sys/time.h>

#include "noteboard_util.h"
#include "config.h"

int main(int argc, char const *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server ip> <port> <binary message> <loc per bit>\n",argv[0]);
        return 1;
    }

    const char* message = argv[3];
    int locs_per_bit = atoi(argv[4]);

    init_connection(argv[1],atoi(argv[2]));

    for ( int bit_idx = 0; message[bit_idx];bit_idx++)
    {
        char bit = message[bit_idx];
        printf("%d-%c\n",bit_idx,bit);
        if (bit == '0') continue;
        int start_key = bit_idx * locs_per_bit;
        int end_key = start_key + locs_per_bit - 1;
        for(int curr_key = start_key; curr_key<=end_key; curr_key++)
        {
            for(int j=0;j<SENDER_WRITE_COUNT;j++)
            {
                send_update_request(curr_key);
            }
        }
    }
}

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
        fprintf(stderr, "Usage: %s <server ip> <port> <message len> <loc per bit>\n",argv[0]);
        return 1;
    }

    size_t message_len = atoi(argv[3]);
    int locs_per_bit = atoi(argv[4]);
 	
    init_connection(argv[1],atoi(argv[2]));

    for (int i=0;i<3000;i++)
        send_ping_request();

    for ( int bit_idx = 0;bit_idx < message_len;bit_idx++)
    {
        int start_key = bit_idx * locs_per_bit;
        int end_key = start_key + locs_per_bit - 1;
        int wl_lo = 0;
        int wl_mid = 0;
        int wl_hi = 0;
        for(int curr_key = start_key; curr_key<=end_key; curr_key++)
        {
            for(int j=0;j<RECEIVER_THRES_LO;j++)
                if (send_update_request(curr_key) > WEAR_LVL_LAT) 
                {
                    wl_lo +=1;
                }
            for(int j=RECEIVER_THRES_LO;j<RECEIVER_THRES_MID;j++)
                if (send_update_request(curr_key) > WEAR_LVL_LAT) 
                {
                    wl_mid +=1;
                }
            for(int j=RECEIVER_THRES_MID;j<RECEIVER_THRES_HI;j++)
                if (send_update_request(curr_key) > WEAR_LVL_LAT) 
                {
                    wl_hi += 1;
                }
        }
        printf("%d-%d-%d-%d\n",bit_idx,wl_lo,wl_mid,wl_hi);
    }
}

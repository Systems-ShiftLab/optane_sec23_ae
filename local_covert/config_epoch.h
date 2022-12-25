#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SENDER_FILE "/mnt/ramdisk/sender"
#define RECEIVER_FILE "/mnt/ramdisk/receiver"

#define REGION_SIZE ((1024*1024*1024UL))
#define MAX_MSG_LEN ((4096UL))
//#define MAX_MSG_LEN ((1100UL))

#define AIT_INDEX_BITS 0x1aUL
// #define AIT_SET_NUM 1

// Timing parameters
// Contention channel
#define CONT_INTERVAL_CYCLE 900000 // 0.83 ms
#define CONT_MEMSET_SIZE 4096UL // Sender
#define CONT_WAIT 10000UL // Receiver
#define CONT_THRE_CYCLE 1400UL // Receiver
#define CONT_LOAD_COUNT 350 // Receiver

// AIT channel
#define AIT_INTERVAL_CYCLE 200000 // 0.12 ms
#define AIT_PRIME_CYCLE 8000UL // Receiver
#define AIT_ACCESS_CYCLE 140000UL // Sender
#define AIT_ACCESS_COUNT 64 // Sender
#define AIT_ACCESS_REP 4 // Sender
#define AIT_THRE_CYCLE 790UL // Receiver
#define AIT_LOAD_COUNT 3 // Receiver

// RMW channel
#define RMW_INTERVAL_CYCLE 185000 // 0.13 ms
#define RMW_PRIME_CYCLE 9000UL // Receiver
#define RMW_ACCESS_CYCLE 170000UL // Sender
#define RMW_ACCESS_COUNT 150 // Sender
#define RMW_THRE_CYCLE 500 // Receiver
#define RMW_LOAD_COUNT 8 // Receiver

#define EPOCH_MASK 0x3FFFFFFFFUL
#endif

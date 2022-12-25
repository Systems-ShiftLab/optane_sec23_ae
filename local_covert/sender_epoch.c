#include "util.h"
#include "config_epoch.h"

void *pmem_area;
uint64_t global_epoch;

// Write contention channel
void send_bit_cont(uint8_t bit) {
    uint64_t next_epoch = global_epoch + CONT_INTERVAL_CYCLE;
    int offset = 0;
    do {
        if (bit==1)
        {
        // Access PM
            pmem_memset_persist(pmem_area + offset, 0, CONT_MEMSET_SIZE);
            offset += CONT_MEMSET_SIZE;
        }
        else if (bit == 0) {}
        else {
            fprintf(stderr, "Bit value can be either 0 or 1\n");
            exit(1);
        }
    } while(get_cycle()  < next_epoch);
    global_epoch = next_epoch;
}

// AIT set contention channel
void send_bit_ait(uint8_t bit) {
    uint64_t rand_val, start_time, end_time;
    uint64_t next_epoch = global_epoch + AIT_INTERVAL_CYCLE;
    if (bit == 1) {
        // Probe set
        spin_wait_cycle(AIT_PRIME_CYCLE);

        start_time = get_cycle();
        for (int i = 0; i < AIT_ACCESS_COUNT; ++i) {
            uint64_t rep = AIT_ACCESS_REP;
            get_rand(&rand_val, REGION_SIZE);

            void* addr = pmem_area + (((rand_val & (~AIT_MASK)) | (AIT_INDEX_BITS << 12)) & (~0xfffUL));
            asm volatile(
                "xor %%r10, %%r10\n"
                "xor %%r11, %%r11\n"
                "AIT_LOOP:\n"
                "vmovntdqa (%[addr], %%r10), %%zmm0\n"
                "mfence\n"
                "add $256, %%r10\n"  // increment by 1 RMW line
                "inc %%r11\n"
                "cmp %[count], %%r11\n"
                "jl AIT_LOOP\n"
            :
            : [addr]"r"(addr), [count]"r"(rep)
            : "r10", "r11", "zmm0");
        }
        end_time = get_cycle();
        //spin_wait_cycle((AIT_INTERVAL_CYCLE - AIT_PRIME_CYCLE) - (end_time - start_time));
        // printf("ait access_time %lu\n", end_time - start_time);
    } else if (bit == 0) {
        // Wait
        //spin_wait_cycle(AIT_INTERVAL_CYCLE);
    } else {
        fprintf(stderr, "Bit value can be either 0 or 1\n");
        exit(1);
    }
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;

}

#if 0
// AIT set contention channel across NUMA
void send_bit_ait_numa(uint8_t bit) {
    uint64_t rand_val, start_time, end_time;
    if (bit == 1) {
        // Probe set
        spin_wait_cycle(AIT_PRIME_CYCLE);

        start_time = get_cycle();
        for (int i = 0; i < AIT_ACCESS_COUNT; ++i) {
            uint64_t rep = AIT_ACCESS_REP;
            get_rand(&rand_val, REGION_SIZE);
            for (int j = 0; j < AIT_SET_NUM; ++j) {
                void* addr = pmem_area + (((rand_val & (~AIT_MASK)) | ((AIT_INDEX_BITS + j) << 12)) & (~0xfffUL));
                asm volatile(
                    "xor %%r10, %%r10\n"
                    "xor %%r11, %%r11\n"
                    "AIT_LOOP_NUMA:\n"
                    "vmovntdqa (%[addr], %%r10), %%zmm0\n"
                    "mfence\n"
                    "add $256, %%r10\n"  // increment by 1 RMW line
                    "inc %%r11\n"
                    "cmp %[count], %%r11\n"
                    "jl AIT_LOOP_NUMA\n"
                :
                : [addr]"r"(addr), [count]"r"(rep)
                : "r10", "r11", "zmm0");
            }
        }
        end_time = get_cycle();
        spin_wait_cycle((AIT_INTERVAL_CYCLE - AIT_PRIME_CYCLE) - (end_time - start_time));
        // printf("ait-numa access_time %lu\n", end_time - start_time);
    } else if (bit == 0) {
        // Wait
        spin_wait_cycle(AIT_INTERVAL_CYCLE);
    } else {
        fprintf(stderr, "Bit value can be either 0 or 1\n");
        exit(1);
    }
}
#endif

void send_bit_rmw(uint8_t bit) {
    uint64_t rand_val, start_time, end_time;
    uint64_t next_epoch = global_epoch + RMW_INTERVAL_CYCLE;
    if (bit == 1) {
        spin_wait_cycle(RMW_PRIME_CYCLE);

        start_time = get_cycle();
        for (int i = 0; i < RMW_ACCESS_COUNT; ++i) {
            uint64_t rep = 4;
            get_rand(&rand_val, REGION_SIZE);
            void* addr = pmem_area + (rand_val & (~RMW_MASK));
            // *(uint64_t*)addr = i;
            // cache_flush(addr, 8);
            asm volatile(
                "vmovntdqa (%[addr]), %%zmm0\n"
                "mfence\n"
            :
            : [addr]"r"(addr)
            : "zmm0");
        }
        end_time = get_cycle();
        //spin_wait_cycle(RMW_INTERVAL_CYCLE - (end_time - start_time) - RMW_PRIME_CYCLE);
        // printf("rmw access_time %lu\n", end_time - start_time);
    } else if (bit == 0) {
        // Wait
        //spin_wait_cycle(RMW_INTERVAL_CYCLE);
    } else {
        fprintf(stderr, "Bit value can be either 0 or 1\n");
        exit(1);
    }
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;
}

void (*send_bit)(uint8_t bit);

// Geneate a bitstrema message
void gen_message(uint8_t* message, int len) {
    for (int i = 0; i < len; ++i) {
        message[i] = rand() % 2;
    }
    printf("Message:\n");
    for (int i = 0; i < len; ++i) {
        printf("%d", message[i]);
    }
    printf("\n");
}

void send_message(uint8_t* message, int len) {
    assert(message);
    printf("Message Begin\n");
    int msg_index = 0;
    do {
       //printf("%d\n",msg_index);
        send_bit(message[msg_index]);
        msg_index++;
    } while(msg_index < len);
    printf("Message End\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./sender <type> <msg len>\n");
        exit(1);
    }

    pmem_area = create_pmem(SENDER_FILE, REGION_SIZE);

    int message_len = atoi(argv[2]);
    
    // Set channel type
    if (!strcmp(argv[1], "ait")) {
        send_bit = send_bit_ait;
    } else if (!strcmp(argv[1], "cont")) {
        send_bit = send_bit_cont;
    } else if (!strcmp(argv[1], "rmw")) {
        send_bit = send_bit_rmw;
    } else {
        fprintf(stderr, "Unknown type\n");
        exit(1);
    }

    uint8_t* message = (uint8_t*)malloc(message_len * sizeof(uint8_t));
    gen_message(message, message_len);


    uint64_t present_epoch = (get_cycle()|EPOCH_MASK)^EPOCH_MASK;
    uint64_t next_epoch = present_epoch + EPOCH_MASK+1;
    printf("%lx -- Target epoch\n",next_epoch);
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;
    
    uint64_t msg_start_time = get_cycle();
    send_message(message, message_len);
    uint64_t msg_end_time = get_cycle();
    printf("%ld\n",msg_end_time - msg_start_time);

    free(message);
    pmem_unmap(pmem_area, REGION_SIZE);
}

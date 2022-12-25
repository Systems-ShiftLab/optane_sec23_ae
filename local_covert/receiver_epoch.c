#include "util.h"
#include "config_epoch.h"

void* pmem_area;
uint64_t global_epoch;
uint64_t curr_offset, next_offset;
// uint64_t curr_offset_vec[AIT_SET_NUM];

// Write contention channel
uint8_t get_bit_cont() {
    uint64_t chase_start, chase_end, chase_cycle;
    uint64_t next_epoch = global_epoch + CONT_INTERVAL_CYCLE;

    uint8_t bit_val;

    spin_wait_cycle(CONT_WAIT);

    chase_start = get_cycle();
    curr_offset = load_list_count(pmem_area, curr_offset, CONT_LOAD_COUNT);
    chase_end = get_cycle();
    chase_cycle = chase_end - chase_start;
    
     while(get_cycle()  < next_epoch);
    global_epoch = next_epoch;

    bit_val = (chase_cycle > CONT_LOAD_COUNT * CONT_THRE_CYCLE);

    // printf("chase_cycle = %ld, bit %d\n", chase_cycle/CONT_LOAD_COUNT, bit_val);
    return bit_val;
}


// AIT set contention channel
uint8_t get_bit_ait() {
    uint64_t start, end;
    uint64_t prime_cycle, probe_cycle, flush_cycle; 
    uint64_t next_epoch = global_epoch + AIT_INTERVAL_CYCLE;

    uint8_t bit_val;
    // Prime
    start = get_cycle();
    load_list_count(pmem_area, curr_offset, AIT_LOAD_COUNT);
    // end = get_cycle();

    prime_cycle = get_cycle() - start;
    // printf("prime_cycle %lu\n", prime_cycle);

    spin_wait_cycle(AIT_ACCESS_CYCLE);

    // Probe
    start = get_cycle();
    load_list_count(pmem_area + 256, curr_offset, AIT_LOAD_COUNT);
    // end = get_cycle();

    probe_cycle = get_cycle() - start;

    // Flush lists from CPU cache
    start = get_cycle();
    load_list_count_flush(pmem_area + 256, curr_offset, AIT_LOAD_COUNT);
    curr_offset = load_list_count_flush(pmem_area, curr_offset, AIT_LOAD_COUNT);
    // _mm_mfence();
    // end = get_cycle();

    flush_cycle = get_cycle() - start;
    // printf("flush_cycle %lu\n", flush_cycle);

    //spin_wait_cycle((AIT_INTERVAL_CYCLE - AIT_ACCESS_CYCLE) - probe_cycle - prime_cycle - flush_cycle);
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;

    bit_val = (probe_cycle > AIT_LOAD_COUNT * AIT_THRE_CYCLE);
    // printf("probe_cycle %lu, bit %d\n", probe_cycle / AIT_LOAD_COUNT, bit_val);

    return bit_val;
}

#if 0
// AIT set contention channel across NUMA
uint8_t get_bit_ait_numa() {
    uint64_t start, end;
    uint64_t prime_cycle, probe_cycle, flush_cycle; 

    uint8_t bit_val;

    // Prime
    start = get_cycle();
    for (int i = 0; i < AIT_SET_NUM; ++i) {
        load_list_count(pmem_area, curr_offset_vec[i], AIT_LOAD_COUNT);
    }
    end = get_cycle();

    prime_cycle = end - start;
    // printf("prime_cycle %lu\n", prime_cycle);

    spin_wait_cycle(AIT_ACCESS_CYCLE);

    // Probe
    start = get_cycle();
    for (int i = 0; i < AIT_SET_NUM; ++i) {
        load_list_count(pmem_area, curr_offset_vec[i] + 256, AIT_LOAD_COUNT);
    }
    end = get_cycle();

    probe_cycle = end - start;

    start = get_cycle();
    for (int i = 0; i < AIT_SET_NUM; ++i) {
         curr_offset_vec[i] = load_list_count_flush(pmem_area, curr_offset_vec[i], AIT_LOAD_COUNT);
    }
    _mm_mfence();
    end = get_cycle();

    flush_cycle = end - start;
    printf("flush_cycle %lu\n", flush_cycle);

    spin_wait_cycle((AIT_INTERVAL_CYCLE - AIT_ACCESS_CYCLE) - probe_cycle - prime_cycle - flush_cycle);

    bit_val = (probe_cycle > AIT_LOAD_COUNT * AIT_THRE_CYCLE * AIT_SET_NUM);
    // printf("probe_cycle %lu, bit %d\n", probe_cycle / AIT_LOAD_COUNT / AIT_SET_NUM, bit_val);

    return bit_val;
}
#endif

uint8_t get_bit_rmw() {
    uint64_t start, end;
    uint64_t prime_cycle, probe_cycle;
    uint8_t bit_val;
    uint64_t next_epoch = global_epoch + RMW_INTERVAL_CYCLE;

    // prime
    start = get_cycle();
    load_list_count(pmem_area, curr_offset, RMW_LOAD_COUNT);
    end = get_cycle();
    prime_cycle = end - start;
    // printf("prime_cycle %lu, bit %d\n", prime_cycle, bit_val);

    spin_wait_cycle(RMW_ACCESS_CYCLE);

    // Probe
    start = get_cycle();
    // Shift by 64B and run pointer-chasing
    curr_offset = load_list_count(pmem_area + 64, curr_offset, RMW_LOAD_COUNT);
    end = get_cycle();
    probe_cycle = end - start;

    //spin_wait_cycle((RMW_INTERVAL_CYCLE - RMW_ACCESS_CYCLE) - probe_cycle - prime_cycle);
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;

    bit_val = (probe_cycle > RMW_LOAD_COUNT * RMW_THRE_CYCLE);
    // printf("probe_cycle %lu, bit %d\n", probe_cycle / RMW_LOAD_COUNT, bit_val);

    return bit_val;
}

uint8_t (*get_bit)();

void receive_message(uint8_t* message, int message_length) {
    int msg_index = 0;

    do {
        message[msg_index] = get_bit();
        msg_index++;
    } while (msg_index <= message_length);
}

int main(int argc, char **argv){

    uint64_t init_start, init_end;
    //init_start = get_cycle();

    if (argc != 3) {
        fprintf(stderr, "Usage: ./receiver <type> <expected length>\n");
        exit(1);
    }
    
    pmem_area = create_pmem(RECEIVER_FILE, REGION_SIZE);
    
    int message_len = atoi(argv[2]);

    int chasing_count;

    // Set channel type
    if (!strcmp(argv[1], "ait")) {
        get_bit = get_bit_ait;
        chasing_count = REGION_SIZE / AITL_SIZE / 256;
        gen_ait_chasing_list(pmem_area, chasing_count, RMWL_SIZE, AIT_INDEX_BITS);
        curr_offset = AIT_INDEX_BITS << 12;
    } else if (!strcmp(argv[1], "cont")) {
        get_bit = get_bit_cont;
        chasing_count = REGION_SIZE / RMWL_SIZE;
        gen_chasing_list(pmem_area, chasing_count, RMWL_SIZE);
        curr_offset = 0;
    } else if (!strcmp(argv[1], "rmw")) {
        get_bit = get_bit_rmw;
        chasing_count = REGION_SIZE / RMWL_SIZE;
        gen_chasing_list(pmem_area, chasing_count, RMWL_SIZE);
        curr_offset = 0;
    } else {
        fprintf(stderr, "Unknown type\n");
        exit(1);
    }

    cache_flush(pmem_area, REGION_SIZE);

    uint8_t* message = (uint8_t*)malloc(MAX_MSG_LEN * sizeof(uint8_t));
    memset(message, 0, MAX_MSG_LEN * sizeof(uint8_t));

    //printf("Receiver init complete\n");

    assert(message_len<=MAX_MSG_LEN);

    //init_end = get_cycle();
    //printf("init_time - %ld\n",init_end-init_start);

    uint64_t present_epoch = (get_cycle()|EPOCH_MASK)^EPOCH_MASK;
    uint64_t next_epoch = present_epoch + EPOCH_MASK+1;
    printf("%lx -- Target epoch\n",next_epoch);
    while(get_cycle() < next_epoch);
    global_epoch = next_epoch;

    receive_message(message, message_len);

    // Only print out the expected length
    int corr_count = 0;
    srand(0);
    for (int i = 0; i < message_len; ++i) {
        uint8_t correct = rand() % 2;
        if (message[i] == correct)
        {
            printf(GREEN"%u"RESET, message[i]);
            corr_count++;
        }
        else 
            printf(RED"%u"RESET, message[i]);
    }
    printf("\n");
    printf("%d\n",corr_count);

    free(message);
    pmem_unmap(pmem_area, REGION_SIZE);
}

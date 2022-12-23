#ifndef __UTIL_H__
#define __UTIL_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>

#include <libpmem.h>
#include <x86intrin.h>

#define CL_SIZE 64UL
#define RMW_SIZE 16384UL
#define RMWL_SIZE 256UL
#define AIT_SIZE (4096*4096UL)
#define AITL_SIZE 4096UL
#define PAGE_SIZE 4096UL

#define MICRO_PER_SEC 1000000

#define RMW_MASK 0xffUL
#define AIT_MASK 0xff000UL

// Color
#define RESET "\e[0m"
#define RED   "\e[0;31m"
#define GREEN "\e[0;32m"

// struct chasing_node_t {
//     struct chasing_node_t *next;
// };


void cache_flush(void* addr, uint64_t size);

void* create_pmem(char* file_name, uint64_t size);

int get_rand(uint64_t *rd, uint64_t range);

uint64_t get_cycle();
uint64_t get_usec();

uint64_t sync_cycle(uint64_t sync_mask,uint64_t sync_jitter);
uint64_t sync_usec(uint64_t sync_mask,uint64_t sync_jitter);

void spin_wait_cycle(uint64_t time_cycle);
void spin_wait_usec(uint64_t time_usec);

void store_list(char *start_addr, uint64_t node_count, uint64_t *cindex, uint64_t node_size);
uint64_t load_list_count(void *list_addr, uint64_t start_offset, uint64_t node_count);
uint64_t load_list_count_flush(void *list_addr, uint64_t start_offset, uint64_t node_count);
uint64_t load_list_time(void *list_addr, uint64_t start_offset, uint64_t cycle);                                                                        

int gen_perm(uint64_t* perm_array,uint64_t count);
void gen_chasing_index(uint64_t* perm_array,uint64_t count);
void gen_chasing_list(void* pmem_ptr, int count, int node_size);
void gen_ait_chasing_list(void* pmem_ptr, int count, int node_size, uint64_t ait_index_bits);

void warmup_cpu();

#endif

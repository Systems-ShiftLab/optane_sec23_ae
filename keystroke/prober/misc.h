#pragma once
#include <stdint.h>

void gen_fwd_rev_chasing_list(void* pmem_ptr, int count);
uint64_t load_fwd_list1(void *list_addr, uint64_t start_offset, uint64_t node_count);
uint64_t load_fwd_list2(void *list_addr, uint64_t start_offset, uint64_t node_count);
uint64_t load_bkwd_list1(void *list_addr, uint64_t start_offset, uint64_t node_count);
uint64_t load_bkwd_list2(void *list_addr, uint64_t start_offset, uint64_t node_count);

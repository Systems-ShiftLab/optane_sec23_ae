#include <stdint.h>
#include <stdbool.h>

#include "util.h"

static void store_node(void *addr,uint64_t node_val,bool is_fwd)
{
    assert (((uint64_t)addr % RMWL_SIZE) == 0);
    uint64_t *nd_addr0, *nd_addr1;
    if (is_fwd) {
         nd_addr0 = addr;
         nd_addr1 = addr+CL_SIZE;
    }
    else {
         nd_addr0 = addr+(CL_SIZE*2);
         nd_addr1 = addr+(CL_SIZE*3);
    }
    *nd_addr0 = node_val;
    *nd_addr1 = node_val;
}

#define store_bkwd_node(x,y) store_node(x,y,0)
#define store_fwd_node(x,y) store_node(x,y,1)
// Offset 0, 64 --> Forward list
// Offset 128, 192 --> Backward list
static void store_list_fwd_rev(void* pmem_loc, uint64_t node_count, uint64_t *cindex) 
{
    for (uint64_t i=0; i < node_count; i++) {
        uint64_t curr = i * RMWL_SIZE;
        uint64_t next = cindex[i] * RMWL_SIZE;
        store_fwd_node(pmem_loc+curr,next);
        store_bkwd_node(pmem_loc+next,curr);
    }
}

void gen_fwd_rev_chasing_list(void* pmem_ptr, int count) 
{
    uint64_t* cindex = (uint64_t*)malloc(sizeof(uint64_t) * count);
	gen_chasing_index(cindex, count);
    store_list_fwd_rev(pmem_ptr, count, cindex);
    free(cindex);
}


uint64_t load_fwd_list1(void *list_addr, uint64_t start_offset, uint64_t node_count)
{
    assert (((uint64_t)list_addr % RMWL_SIZE) == 0);
    assert ((start_offset % RMWL_SIZE) == 0);
    return load_list_count(list_addr+0,start_offset,node_count);
}
uint64_t load_fwd_list2(void *list_addr, uint64_t start_offset, uint64_t node_count)
{
    assert (((uint64_t)list_addr % RMWL_SIZE) == 0);
    assert ((start_offset % RMWL_SIZE) == 0);
    return load_list_count(list_addr+(1*CL_SIZE),start_offset,node_count);
}

uint64_t load_bkwd_list1(void *list_addr, uint64_t start_offset, uint64_t node_count)
{
    assert (((uint64_t)list_addr % RMWL_SIZE) == 0);
    assert ((start_offset % RMWL_SIZE) == 0);
    return load_list_count(list_addr+(2*CL_SIZE),start_offset,node_count);
}
uint64_t load_bkwd_list2(void *list_addr, uint64_t start_offset, uint64_t node_count)
{
    assert (((uint64_t)list_addr % RMWL_SIZE) == 0);
    assert ((start_offset % RMWL_SIZE) == 0);
    return load_list_count(list_addr+(3*CL_SIZE),start_offset,node_count);
}

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<libpmem.h>
#include<x86intrin.h>

#include "util.h"
#include "experiment.h"
#include "timing.h"
#include "mem.h"


int main(int argc, char* argv[])
{

	if(argc != 2 )
	{
		fprintf(stderr,"Usage: %s <rep_cnt>",argv[0]);
		exit(-1);
	}

	pmem_exp_init("/mnt/ramdisk/pmem_pool",REGION_SIZE,true);

    int rep_cnt = atoi(argv[1]);


	warmup_cpu();
	uint64_t offset = 0;
	uint64_t tsp_start, tsp_end;
    for(int i=0;i<rep_cnt;i++)
	{
		char* addr = &mapped_area[offset];	
		load_time(addr,&tsp_start,&tsp_end);
		asm volatile (
		"clflush (%[addr])\n"
		"clflush 64(%[addr])\n"
		"clflush 128(%[addr])\n"
		"clflush 192(%[addr])\n"
		"mfence\n"
		::[addr]"r"(addr));
		load_time(addr,&tsp_start,&tsp_end);
		uint64_t latency = tsp_end - tsp_start;
		printf("%lu\n",latency);
        offset += RMWL_SIZE;
	}
	pmem_unmap(mapped_area,REGION_SIZE);
	return 0;
}

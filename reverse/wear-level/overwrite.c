
#include<time.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#include "timing.h"
#include "experiment.h"
#include "mem.h"
#include "util.h"

#define REGION_SIZE 0x40000000  //A 1Gb pmem region

extern inline __attribute__((always_inline))
void write_nt256_time(char *addr,uint64_t *tsp_start, uint64_t *tsp_end)
{
	asm volatile (

	TSP_START
	"vmovntdq  %%zmm0, 0x0(%[addr]) \n" 
    "clflush (%[addr])\n"
	"vmovntdq  %%zmm0, 0x40(%[addr]) \n"
    "clflush 64(%[addr])\n"
	"vmovntdq  %%zmm0, 0x80(%[addr]) \n"
    "clflush 128(%[addr])\n"
	"vmovntdq  %%zmm0, 0xC0(%[addr]) \n"
    "clflush 192(%[addr])\n"
    "mfence \n"
	TSP_END

	: [tsp_start]"=&r"(*tsp_start),[tsp_end]"=&r"(*tsp_end)
	: [addr]"r"(addr)
	:  "%rdx", "%rax","rcx", "memory");
}

int main(int argc, char* argv[])
{

	if(argc != 2)
	{
		fprintf(stderr,"Usage: %s <write_cnt - e.g. 500000>",argv[0]);
		exit(0);
	}


	pmem_exp_init("/mnt/ramdisk/pmem_pool",REGION_SIZE,true);

	int total_count = atoi(argv[1]);
	uint64_t* buffer = malloc(total_count * sizeof(uint64_t));


	for(int i=0;i<total_count;i++)
	{
		uint64_t offset = 0;
		uint64_t tsp_end,tsp_start;
		write_nt256_time(mapped_area+offset, &tsp_start, &tsp_end);
		buffer[i] = tsp_end - tsp_start;
	}

    printf("Iteration,Latency\n");
	for(int i=0;i<total_count;i++)
	{
		uint64_t latency = buffer[i];
        if ( latency < 90000  && (i % 1000) ) continue;
		printf("%d,%lu\n",i,latency);
	}

	free(buffer);
	pmem_exp_end();
	return 0;
}

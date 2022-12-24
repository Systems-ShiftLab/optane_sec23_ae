// store-load-load

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpmem.h>

#include "aux.h"
#include "pc_timing.h"

#define REGION_SIZE 0x1000000000  //A 64Gb pmem region
#define LATENCY_OPS_COUNT 1048576L


int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr,"Usage: %s number_of_accessed_elements bitmask \n",argv[0]);
		fprintf(stderr,"A bitmask of 0 still implies a stride of 64\n");
		fprintf(stderr,"The output is the total latency of all loads\n");
		exit(1);
	}

	uint64_t input_access_count = atol (argv[1]);
	uint64_t input_bitmask = atol (argv[2]);
	input_bitmask <<=3;
	input_bitmask |=7; //Converting the bitmask to 64B stride

	uint64_t region_size = input_access_count * (1<<count_bits(input_bitmask)) * sizeof(uint64_t);
	


	//pmem region initialization
	int is_pmem;
	size_t len;
	char* mapped_area = pmem_map_file("/mnt/ramdisk/pmem_region",REGION_SIZE,PMEM_FILE_CREATE,0666,&len,&is_pmem);

	if(!is_pmem)
	{
		fprintf(stderr,"Not pmem region");
		exit(1);
	}
	if(len != REGION_SIZE)
	{
		fprintf(stderr,"Region size small");
		exit(1);
	}

	//chasing index initialization
	uint64_t region_count = REGION_SIZE / region_size;

	if(region_count == 0)
	{
		fprintf(stderr,"Region exceeds pool size");
		exit(1);
	}

	if (region_count > LATENCY_OPS_COUNT)
		region_count = LATENCY_OPS_COUNT;

	if (region_size <= 512)
		region_count *= 8;

	if (region_size >= 16384)
		region_count /= 2;

	if (region_size >= 32768)
		region_count /= 2;

	if (region_size >= 64 * 1024)
		region_count /= 2;

	if (region_size >= 128 * 1024)
		region_count /= 2;

	if (region_size >= 512 * 1024)
		region_count /= 2;
	if(region_count == 0)
        region_count = 1;


	fprintf(stderr,"Using region_size: %ld\n",region_size);
	fprintf(stderr,"Using region_count: %ld\n",region_count);

	for(int i=0;i<region_count;i++)
		gen_bitmask_perm((uint64_t*)(mapped_area+(i*region_size)),input_access_count,input_bitmask);


	PC_VARS

	PC_BEFORE_READ

	asm volatile(
		"xor    %%r8, %%r8 \n"
		"xor    %%r11, %%r11 \n"

	"LOOP_OUTER%=: \n"
		"lea    (%[start_addr], %%r8), %%r9 \n"
		"xor    %%r10, %%r10 \n"
		"xor	%%r12, %%r12 \n"

	"LOOP_INNER%=: \n"
		"vmovntdqa	(%%r9, %%r12,8), %%zmm0\n"
		"inc %%r10\n"
		"movq	%%xmm0, %%r12\n"

		"cmp    %[accesscount], %%r10 \n"
		"jl     LOOP_INNER%= \n"
	//	CHASING_LD_FENCE
		"mfence \n"
		"add    %[skip], %%r8 \n"

		"inc    %%r11 \n"
		"cmp    %[count], %%r11 \n"
		"jl     LOOP_OUTER%= \n"

		:                         
		: [start_addr] "r"(mapped_area), 
		  [accesscount] "r"(input_access_count), [count] "r"(region_count),
		  [skip] "r"(region_size)
		: "%r12", "%r11", "%r10", "%r9", "%r8" );
	
	PC_AFTER_READ

	PC_PRINT_MEASUREMENT

	pmem_unmap(mapped_area,len);
	return 0;
}

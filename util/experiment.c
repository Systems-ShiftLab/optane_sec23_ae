#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <libpmem.h>
#include "experiment.h"

char* mapped_area;
void pmem_exp_init(char* file_name,size_t region_size, bool is_memset)
{
	pagemap_fd = open("/proc/self/pagemap", O_RDONLY); 
	assert(pagemap_fd >= 0); 
	int is_pmem;
	size_t len;
	mapped_area = pmem_map_file(file_name,region_size,PMEM_FILE_CREATE,0666,&len,&is_pmem);
	if(!is_pmem)
	{
		fprintf(stderr,"Not pmem region");
		exit(1);
	}
	if(len != region_size)
	{
		fprintf(stderr,"Region size small");
		exit(1);
	}

	if (is_memset)
		pmem_memset_persist(mapped_area, 0, region_size);
//	for(int i=0;i<region_size;i+=64) 
//			asm volatile ("clflush %[target]"::[target]"m"(mapped_area[i]));
//	asm volatile ("mfence\n");
	setvbuf(stdout, NULL, _IONBF, 0);
}

//TODO: This is pretty bad
void pmem_exp_end()
{
	pmem_unmap(mapped_area,REGION_SIZE);
	close(pagemap_fd);
}

int pagemap_fd;
//uint64_t get_physical_addr(void* virtual) {
//	uintptr_t virtual_addr = (uintptr_t)virtual;
//	uint64_t value;
//	off_t offset = (virtual_addr /PAGE_SIZE) * sizeof(value);
//	int got = pread(pagemap_fd, &value, sizeof(value), offset);
//	assert(got == 8);
//
//	// Check the "page present" flag.
//	assert(value & (1ULL << 63));
//
//	uint64_t frame_num =  value & ((1ULL << 54) - 1);
//	return (frame_num * PAGE_SIZE) | (virtual_addr & (PAGE_SIZE - 1));
//}


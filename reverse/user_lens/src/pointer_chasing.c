// store-load-load

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpmem.h>
#include "chasing.h"

#define REGION_SIZE 0x1000000000  //A 64Gb pmem region
#define LATENCY_OPS_COUNT 1048576L

#define PC_VARS                                                                \
	unsigned int c_store_start_hi, c_store_start_lo;                       \
	unsigned int c_load_start_hi, c_load_start_lo;                         \
	unsigned int c_load_end_hi, c_load_end_lo;                             \
	unsigned long c_store_start;                                           \
	unsigned long c_load_start, c_load_end;

#define PC_BEFORE_WRITE                                                        \
	asm volatile("rdtscp \n\t"                                             \
		     "lfence \n\t"                                             \
		     "mov %%edx, %[hi]\n\t"                                    \
		     "mov %%eax, %[lo]\n\t"                                    \
		     :                                                         \
		     [hi] "=r"(c_store_start_hi), [lo] "=r"(c_store_start_lo)  \
		     :                                                         \
		     : "rdx", "rax", "rcx");

#define PC_BEFORE_READ                                                         \
	asm volatile("rdtscp \n\t"                                             \
		     "lfence \n\t"                                             \
		     "mov %%edx, %[hi]\n\t"                                    \
		     "mov %%eax, %[lo]\n\t"                                    \
		     : [hi] "=r"(c_load_start_hi), [lo] "=r"(c_load_start_lo)  \
		     :                                                         \
		     : "rdx", "rax", "rcx");

#define PC_AFTER_READ                                                          \
	asm volatile("rdtscp \n\t"                                             \
		     "lfence \n\t"                                             \
		     "mov %%edx, %[hi]\n\t"                                    \
		     "mov %%eax, %[lo]\n\t"                                    \
		     : [hi] "=r"(c_load_end_hi), [lo] "=r"(c_load_end_lo)      \
		     :                                                         \
		     : "rdx", "rax", "rcx");                                   \

#define PC_PRINT_MEASUREMENT(job_name)                             \
	c_store_start =                                                  \
		(((unsigned long)c_store_start_hi) << 32) | c_store_start_lo;  \
	c_load_start =                                                   \
		(((unsigned long)c_load_start_hi) << 32) | c_load_start_lo;    \
	c_load_end = (((unsigned long)c_load_end_hi) << 32) | c_load_end_lo;   \
	printf("[%s] region_size %ld, block_size %ld, count %d, "            \
		"cycle %ld - %ld - %ld\n",                                      \
		job_name, region_size, block_size, count,                      \
		c_store_start, c_load_start, c_load_end);                       \

int main(int argc, char* argv[])
{
  if(argc < 4)
  {
    fprintf(stderr,"Usage: %s region_size block_size [operation]",argv[0]);
    exit(1);
  }

  uint64_t region_size = atol (argv[1]);
  uint64_t block_size = atol (argv[2]);
  uint8_t operation = atoi (argv[3]);
  region_size = (region_size/block_size) * block_size;

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
  int count = REGION_SIZE / region_size;

  if (count > LATENCY_OPS_COUNT)
    count = LATENCY_OPS_COUNT;

  if (region_size <= 512)
    count *= 8;

  if (region_size >= 16384)
    count /= 2;

  if (region_size >= 32768)
    count /= 2;

  if (region_size >= 64 * 1024)
    count /= 2;

  if (region_size >= 128 * 1024)
    count /= 2;

  if (region_size >= 512 * 1024)
    count /= 2;

  unsigned int chasing_func_index = chasing_find_func(block_size);
  if (chasing_func_index == -1) 
  {
    fprintf(stderr,"Pointer chasing benchamrk with block size %ld byte not found\n",block_size);
    return 2;
  }
  uint64_t csize = region_size /
       (chasing_func_list[chasing_func_index].block_size);

  uint64_t* cindex = malloc(sizeof(uint64_t) * csize);
  init_chasing_index(cindex, csize);

  PC_VARS;

  if (operation == 0)
  {
    PC_BEFORE_WRITE

    chasing_func_list[chasing_func_index].st_func(mapped_area, region_size, region_size, count, cindex);
    asm volatile("mfence \n" :::);

    PC_BEFORE_READ
    PC_AFTER_READ

    PC_PRINT_MEASUREMENT(chasing_func_list[chasing_func_index].name);
  }
  else if (operation == 1)
  {
    PC_BEFORE_WRITE
      chasing_func_list[chasing_func_index].raw_func (mapped_area, region_size, region_size, count, cindex);
      asm volatile("mfence \n" :::);
    PC_BEFORE_READ
    PC_AFTER_READ

    PC_PRINT_MEASUREMENT(chasing_func_list[chasing_func_index].name);
  }
  else
  {
    PC_BEFORE_WRITE
      chasing_func_list[chasing_func_index].st_func (mapped_area, region_size, region_size, count, cindex);
      asm volatile("mfence \n" :::);
    PC_BEFORE_READ
      chasing_func_list[chasing_func_index].ld_func (mapped_area, region_size, region_size, count, cindex);
      asm volatile("mfence \n" :::);
    PC_AFTER_READ

    PC_PRINT_MEASUREMENT(chasing_func_list[chasing_func_index].name);
  }
      

//Frequency measurement
  free(cindex);
  pmem_unmap(mapped_area,len);
  return 0;
}

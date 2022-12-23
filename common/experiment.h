#include <stdbool.h>
#include "util.h"

#define REGION_SIZE 0x40000000

extern int pagemap_fd;
extern char* mapped_area;

void pmem_exp_init(char* file_name,size_t region_size, bool is_memset);
void pmem_exp_end();
uint64_t get_physical_addr(void* virtual_addr);
char cookie_data(int offset);
#define PMEM_EXP_INIT_NAME(x) pmem_exp_init(#x,REGION_SIZE,1);

#define PMEM_EXP_INIT PMEM_EXP_INIT_NAME(/mnt/ramdisk/pmem_pool)

#define PMEM_EXP_END  pmem_exp_end();

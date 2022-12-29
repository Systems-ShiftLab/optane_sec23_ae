#include<time.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libpmem.h>
#include<x86intrin.h>

#define PANIC(x) do { \
    fprintf(stderr,x"\n"); \
    exit(1); \
    } while(0) 



int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr,"Usage: %s <pmem_cmap_file>\n",argv[0]);
		exit(0);
	}

    size_t mapped_len;
    int is_pmem;
    char* mapped_area = pmem_map_file(argv[1],0,0,0,&mapped_len,&is_pmem);
    if(!mapped_area || !is_pmem) {
        PANIC("Mapping error");
    }

    size_t curr_off = 0;

    int wears_done = 0;
    char buff[256];
    while(curr_off < mapped_len)
    {
        char* v = &mapped_area[curr_off];
        if(!(v[0] == 'v' && v[1] == 'v' && v[2] == 'v')) {curr_off += 256; continue; }
        wears_done++;
        printf("Cleaning offset: 0x%zx\n",curr_off);
        memcpy(buff,v,256);

        int write_count = 0;
        while(1) {
            write_count++;
            unsigned int dummy;
            uint64_t start = __rdtscp(&dummy);
            pmem_memcpy_persist(v,buff,256);
            memcpy(buff,v,256);
            uint64_t end = __rdtscp(&dummy);
            if (end-start > 100000)
                break;
        }
        printf("Write count: %d\n",write_count);
        curr_off += 256;
    }
    printf("Wears done: %d\n",wears_done);

	pmem_unmap(mapped_area,mapped_len);

	return 0;
}

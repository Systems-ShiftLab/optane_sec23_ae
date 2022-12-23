#include "util.h"
#include "experiment.h"
#include <sys/time.h>

int main(int argc, char **argv){

	if(argc != 3)
	{
		fprintf(stderr,"Usage: %s <util_frac> <pool_path>",argv[0]);
		exit(0);
	}

    double util_frac = atof(argv[1]);
    void* pmem_area = create_pmem(argv[2], REGION_SIZE);

    double wait_frac = (1/util_frac) - 1;
#define WRITE_SIZE 4096
    while(1){
        int offset = rand() % (REGION_SIZE/WRITE_SIZE);
        offset*=WRITE_SIZE;

        uint64_t start = get_cycle();
        pmem_memset_persist(pmem_area+offset, 0, WRITE_SIZE);
        pmem_memset_persist(pmem_area+offset, 0, WRITE_SIZE);
        pmem_memset_persist(pmem_area+offset, 0, WRITE_SIZE);
        uint64_t end = get_cycle();
        uint64_t wait_cyc = (end-start) * wait_frac;
        spin_wait_cycle(wait_cyc);
    }

    pmem_unmap(pmem_area, REGION_SIZE);
}
